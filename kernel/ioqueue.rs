//! I/O queue subsystem.
//!
//! The I/O queue subsystem is a communications channel between user space and
//! the kernel for I/O commands. User space submits I/O commands to an I/O
//! queue, which are performed by the kernel asynchronously.

use atomic_ring_buffer::AtomicRingBuffer;

#[derive(Clone, Debug)]
pub enum IOCmd {
    PacketTX { addr: *mut u8, len: usize },
}

#[repr(C)]
/// An raw I/O command data structure.
///
/// NOTE! When modifying this data structure, please make sure it matches the C
/// definition in `include/uapi/manticore/io_queue.h`.
struct RawIOCmd {
    addr: *mut u8,
    len: usize,
}

#[derive(Debug)]
/// An I/O command submission queue.
///
/// User space issues commands to this queue to instruct the kernel to perform I/O commands.
pub struct IOQueue {
    pub ring_buffer: AtomicRingBuffer,
}

impl IOQueue {
    /// Constructs a new I/O queue in memory buffer `buf` of size `size`.
    pub fn new(buf: usize, size: usize) -> IOQueue {
        IOQueue {
            ring_buffer: AtomicRingBuffer::new(buf, size),
        }
    }

    /// Removes the first I/O command from the I/O queue.
    pub fn pop(&mut self) -> Option<IOCmd> {
        if let Some(raw_io_cmd) = self.ring_buffer.front::<RawIOCmd>() {
            let cmd = unsafe {
                IOCmd::PacketTX {
                    addr: (*raw_io_cmd).addr,
                    len: (*raw_io_cmd).len,
                }
            };
            self.ring_buffer.pop::<RawIOCmd>();
            return Some(cmd);
        } else {
            None
        }
    }
}
