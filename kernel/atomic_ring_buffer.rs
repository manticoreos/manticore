use core::mem;

#[derive(Clone, Copy, Debug)]
/// An atomic ring buffer.
pub struct AtomicRingBuffer {
    /// The raw pointer to an atomic ring buffer object.
    raw_ptr: usize,
}

impl AtomicRingBuffer {
    /// Constructs a new atomic ring buffer in memory buffer `buf` of size `size`.
    pub fn new(buf: usize, size: usize) -> Self {
        AtomicRingBuffer {
            raw_ptr: unsafe { atomic_ring_buffer_new(buf, size) },
        }
    }

    /// Inserts an element `elem` to this ring buffer.
    pub fn emplace<T>(&self, elem: &T) -> usize {
        unsafe {
            atomic_ring_buffer_emplace(self.raw_ptr, mem::transmute(elem), mem::size_of::<T>())
        }
    }
}

extern "C" {
    pub fn atomic_ring_buffer_new(buf: usize, size: usize) -> usize;
    pub fn atomic_ring_buffer_emplace(ring_buffer: usize, event: usize, size: usize) -> usize;
}
