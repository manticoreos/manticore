//! Kernel memory allocator.
//!
//! The `memory` module contains a kernel memory allocator that is
//! loosely based on the Vmem allocator (Bonwick, 2001).
//!
//! # References
//!
//! Bonwick, Jeff, and Jonathan Adams. "Magazines and Vmem: Extending
//! the Slab Allocator to Many CPUs and Arbitrary Resources." USENIX
//! Annual Technical Conference, General Track. 2001.

use intrusive_collections::{Bound, IntrusiveRef, LinkedList, LinkedListLink, RBTree, RBTreeLink, TreeAdaptor};
use core::intrinsics::transmute;
use print;

const MIB: u64 = 1 << 20;
const PAGE_SIZE_2M: u64 = 2 * MIB;

/// Memory segment.
///
/// A memory segment is a contiguous span of memory.
struct MemorySegment {
    /// Base address of the memory segment.
    pub base: u64,
    /// Size of the memory segment.
    pub size: u64,
    /// Link in freelist.
    pub freelist_link: LinkedListLink,
    /// Link in segments red-black tree.
    pub segments_link: RBTreeLink,
}

intrusive_adaptor!(FreelistAdaptor = MemorySegment { freelist_link: LinkedListLink });

intrusive_adaptor!(SegmentsAdaptor = MemorySegment { segments_link: RBTreeLink });

impl<'a> TreeAdaptor<'a> for SegmentsAdaptor {
    type Key = u64;
    fn get_key(&self, x: &'a MemorySegment) -> u64 {
        x.base
    }
}

impl MemorySegment {
    /// Constructs a new `MemorySegment`.
    const fn new(base: u64, size: u64) -> MemorySegment {
        MemorySegment {
            base: base,
            size: size,
            freelist_link: LinkedListLink::new(),
            segments_link: RBTreeLink::new(),
        }
    }

    /// Splits `size` bytes from the beginning of this memory segment.
    ///
    /// Note! The `self` memory segment is invalid after a call to the `split` function.
    fn split(&self, size: u64) -> &MemorySegment {
        unsafe {
            let mut seg : &mut MemorySegment = transmute(self.base + size);
            *seg = MemorySegment::new(self.base + size, self.size - size);
            return seg;
        }
    }
}

/// Memory arena.
///
/// A memory arena is a collection of memory segments.
struct MemoryArena {
    /// Freelist of memory segments.
    freelist: LinkedList<FreelistAdaptor>,
    /// Memory segments, ordered by base address.
    segments: RBTree<SegmentsAdaptor>,
}

impl MemoryArena {
    unsafe fn alloc(&mut self, size: u64) -> *mut u8 {
        match self.freelist.pop_front() {
            Some(seg_ref) => {
                let seg = seg_ref.as_ref();
                self.segments.find_mut(&seg.base).remove();
                if seg.size < size {
                    self.add_to_freelist(seg);
                    return transmute(0u64);
                }
                let ret = seg.base;
                if seg.size > size {
                    let new_seg = seg.split(size);
                    self.add_to_freelist(new_seg);
                }
                return transmute(ret);
            }
            None => return transmute(0u64),
        }
    }

    unsafe fn free(&mut self, raw_addr: *mut u8, mut size: u64) {
        let mut addr : u64 = transmute(raw_addr);
        {
            // Attempt to coalesce with next segment:
            let mut cursor = self.segments.find_mut(&(addr + size));
            if let Some(next) = cursor.get() {
                if addr + size == next.base {
                    cursor.remove();
                    self.freelist.cursor_mut_from_ptr(next).remove();
                    size += next.size;
                }
            }
        }
        {
            // Attempt to coalesce with previous segment:
            let mut cursor = self.segments.upper_bound_mut(Bound::Excluded(&addr));
            if let Some(prev) = cursor.get() {
                if prev.base + prev.size == addr {
                    cursor.remove();
                    self.freelist.cursor_mut_from_ptr(prev).remove();
                    addr = prev.base;
                    size += prev.size;
                }
            }
        }
        let mut seg : &mut MemorySegment = transmute(addr);
        *seg = MemorySegment::new(addr, size);
        self.add_to_freelist(seg);
    }

    unsafe fn add_to_freelist(&mut self, seg: &MemorySegment) {
        let reference = IntrusiveRef::from_raw(seg);
        self.freelist.push_back(reference.clone());
        self.segments.insert(reference.clone());
    }

    #[allow(dead_code)]
    fn dump(&self) {
        println!("Freelist:");
        for seg in self.freelist.iter() {
            println!("  segment: start={}, size={}", seg.base, seg.size);
        }
        println!("Segments:");
        for seg in self.segments.iter() {
            println!("  segment: start={}, size={}", seg.base, seg.size);
        }
    }
}

/// The kernel memory arena.
static mut KERNEL_ARENA: MemoryArena = MemoryArena {
    freelist: LinkedList::new(FreelistAdaptor),
    segments: RBTree::new(SegmentsAdaptor),
};

/// Register a memory span to the kernel arena.
#[no_mangle]
pub extern "C" fn memory_add_span(start: u64, size: u64) {
    unsafe {
        let mut seg : &mut MemorySegment = transmute(start);
        *seg = MemorySegment::new(start, size);
        KERNEL_ARENA.add_to_freelist(seg);
    }
}

/// Initialize the page allocator.
#[no_mangle]
pub extern "C" fn page_alloc_init() {}

/// Allocate a page.
#[no_mangle]
pub extern "C" fn page_alloc() -> *mut u8 {
    unsafe {
        return KERNEL_ARENA.alloc(PAGE_SIZE_2M);
    }
}

/// Free a page.
#[no_mangle]
pub extern "C" fn page_free(addr: *mut u8) {
    unsafe {
        return KERNEL_ARENA.free(addr, PAGE_SIZE_2M);
    }
}
