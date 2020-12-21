//! Virtqueues are the virtio mechanism for transmitting data from and to an I/O device.

use alloc::boxed::Box;
use core::cell::Cell;
use core::mem;
use intrusive_collections::LinkedListLink;
use kernel::memory;

// =============================================================================
// Virtqueue memory layout
// =============================================================================

/// Virtqueue descriptor.
#[repr(C)]
#[derive(Debug)]
pub struct VirtqDesc {
    pub addr: u64,
    pub len: u32,
    pub flags: u16,
    pub next: u16,
}

pub const VIRTQ_DESC_F_NEXT: u16 = 1;
pub const VIRTQ_DESC_F_WRITE: u16 = 2;
pub const VIRTQ_DESC_F_INDIRECT: u16 = 4;

/// Virtqueue available ring.
#[repr(C)]
#[derive(Debug)]
pub struct VirtqAvail {
    pub flags: u16,
    pub idx: u16,
    pub ring: [u16],
}

pub const VIRTQ_AVAIL_F_NO_INTERRUPT: u16 = 0x01;

/// Virtqueue used ring.
#[repr(C)]
#[derive(Debug)]
pub struct VirtqUsed {
    pub flags: u16,
    pub idx: u16,
    pub ring: [VirtqUsedElem],
}

#[repr(C)]
#[derive(Debug)]
pub struct VirtqUsedElem {
    pub id: u32,
    pub len: u32,
}

pub const VIRTQ_USED_F_NO_NOTIFY: u16 = 0x01;

// =============================================================================
// Virtqueue API
// =============================================================================

/// Virtqueue.
#[derive(Debug)]
pub struct Virtqueue {
    /// Queue index.
    pub queue_idx: u16,
    /// Number of elements in the virtqueue.
    pub queue_size: usize,
    /// Queue notification offset.
    pub notify_off: u16,
    /// Last seen index in the used ring.
    pub last_seen_used: Cell<u16>,
    /// Raw pointer to the descriptor table.
    pub raw_descriptor_table_ptr: usize,
    /// Raw pointer to the available ring.
    pub raw_available_ring_ptr: usize,
    /// Raw pointer to the used ring.
    pub raw_used_ring_ptr: usize,
    /// A linked list link.
    pub link: LinkedListLink,
}

intrusive_adapter!(pub VirtqueueAdapter = Box<Virtqueue>: Virtqueue { link: LinkedListLink });

impl Drop for Virtqueue {
    fn drop(&mut self) {
        self.free()
    }
}

impl Virtqueue {
    pub fn new(queue_idx: u16, queue_size: usize, notify_off: u16) -> Self {
        // FIXME: Ensure that virtqueue components are aligned.
        unsafe {
            let raw_descriptor_table_ptr = memory::kmem_zalloc(queue_size * 16);
            if raw_descriptor_table_ptr == 0 {
                panic!("out of memory");
            }
            let raw_available_ring_ptr = memory::kmem_zalloc(queue_size * 2 + 6);
            if raw_available_ring_ptr == 0 {
                panic!("out of memory");
            }
            let raw_used_ring_ptr = memory::kmem_zalloc(queue_size * 4 + 6);
            if raw_used_ring_ptr == 0 {
                panic!("out of memory");
            }
            Virtqueue {
                queue_idx,
                queue_size,
                notify_off,
                last_seen_used: Cell::new(0),
                raw_descriptor_table_ptr,
                raw_available_ring_ptr,
                raw_used_ring_ptr,
                link: LinkedListLink::new(),
            }
        }
    }

    pub fn free(&mut self) {
        unsafe {
            // FIXME: Fix duplication of component size calculation with new()
            memory::kmem_free(self.raw_descriptor_table_ptr, self.queue_size * 16);
            memory::kmem_free(self.raw_available_ring_ptr, self.queue_size * 2 + 6);
            memory::kmem_free(self.raw_used_ring_ptr, self.queue_size * 4 + 6);
        }
    }

    pub fn last_seen_used(&self) -> u16 {
        self.last_seen_used.get()
    }

    pub fn advance_last_seen_used(&self) {
        self.last_seen_used.replace(self.last_seen_used.get() + 1);
    }

    pub fn last_used_idx(&self) -> u16 {
        /* FIXME: The used ring is in little endian byte order.  */
        unsafe { (*self.used_ring()).idx }
    }

    /// Add a device-writable buffer to virtqueue that is consumed by us.
    pub fn add_inbuf(&self, addr: usize, len: usize) {
        self.add_buf(addr, len, VIRTQ_DESC_F_WRITE)
    }

    /// Add a device-readable buffer to virtqueue that is produced by us.
    pub fn add_outbuf(&self, addr: usize, len: usize) {
        self.add_buf(addr, len, 0)
    }

    pub fn add_buf(&self, addr: usize, len: usize, flags: u16) {
        unsafe {
            /* FIXME: We only support one virtqueue descriptor.  */
            let idx = 0 as u16;
            (*self.descriptor_table())[idx as usize] = VirtqDesc {
                addr: addr as u64,
                len: len as u32,
                flags,
                next: 0,
            };
            self.add_buf_idx(idx);
        }
    }

    pub fn add_buf_idx(&self, idx: u16) {
        let avail = self.available_ring();
        unsafe { (*avail).ring[((*avail).idx % self.queue_size as u16) as usize] = idx; }
        unsafe { (*avail).idx += 1; }
    }

    pub fn get_used_buf(&self, idx: u16) -> (usize, usize) {
        let used = unsafe { &(*self.used_ring()).ring[idx as usize] };
        let addr = self.get_buf(used.id as u16) as u64;
        return (addr as usize, used.len as usize);
    }

    pub fn get_buf(&self, idx: u16) -> usize {
        unsafe { (*self.descriptor_table())[idx as usize].addr as usize }
    }

    pub fn descriptor_table(&self) -> *mut [VirtqDesc] {
        unsafe { mem::transmute((self.raw_descriptor_table_ptr, self.queue_size)) }
    }

    pub fn available_ring(&self) -> *mut VirtqAvail {
        unsafe { mem::transmute((self.raw_available_ring_ptr, self.queue_size)) }
    }

    pub fn used_ring(&self) -> *mut VirtqUsed {
        unsafe { mem::transmute((self.raw_used_ring_ptr, self.queue_size)) }
    }
}
