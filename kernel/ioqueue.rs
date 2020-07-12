//! I/O queue subsystem.
//!
//! The I/O queue subsystem is a communications channel between user space and
//! the kernel for I/O commands. User space submits I/O commands to an I/O
//! queue, which are performed by the kernel asynchronously.

use atomic_ring_buffer::AtomicRingBuffer;

#[derive(Clone, Debug)]
pub enum Opcode {
    Submit,
}

#[derive(Clone, Debug)]
pub struct IOCmd {
    pub opcode: Opcode,
    pub addr: *mut u8,
    pub len: usize,
}

#[repr(C)]
/// An raw I/O command data structure.
///
/// NOTE! When modifying this data structure, please make sure it matches the C
/// definition in `include/uapi/manticore/io_queue_abi.h`.
struct RawIOCmd {
    opcode: u32,
    addr: *mut u8,
    len: usize,
}

const RAW_IO_OPCODE_SUBMIT: u32 = 0x01;

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
            let opcode = unsafe {
                match (*raw_io_cmd).opcode {
                    RAW_IO_OPCODE_SUBMIT => Some(Opcode::Submit),
                    _ => None,
                }
            };
            let (addr, len) = unsafe { ((*raw_io_cmd).addr, (*raw_io_cmd).len) };
            self.ring_buffer.pop::<RawIOCmd>();
            return opcode.map(|opcode| IOCmd {
                opcode: opcode,
                addr: addr,
                len: len,
            });
        } else {
            None
        }
    }
}
