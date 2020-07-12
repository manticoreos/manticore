use core::mem;

#[derive(Clone, Copy, Debug)]
/// An atomic ring buffer.
pub struct AtomicRingBuffer {
    /// The raw pointer to an atomic ring buffer object.
    raw_ptr: usize,
}

impl AtomicRingBuffer {
    /// Constructs a new atomic ring buffer in memory buffer `buf` of size `size`.
    pub fn new<T>(buf: usize, buf_size: usize) -> Self {
        AtomicRingBuffer {
            raw_ptr: unsafe { atomic_ring_buffer_new(buf, buf_size, mem::size_of::<T>()) },
        }
    }

    /// Inserts an element `elem` to this ring buffer.
    pub fn emplace<T>(&self, elem: &T) -> usize {
        unsafe {
            atomic_ring_buffer_emplace(self.raw_ptr, mem::transmute(elem))
        }
    }

    /// Returns a pointer to the first element of this ring buffer.
    pub fn front<T>(&self) -> Option<*mut T> {
        let raw_elem = unsafe { atomic_ring_buffer_front(self.raw_ptr) };
        if raw_elem == 0 {
            return None;
        }
        let elem: *mut T = unsafe { mem::transmute(raw_elem) };
        Some(elem)
    }

    /// Removes the first element of this ring buffer.
    pub fn pop(&self) {
        unsafe {
            atomic_ring_buffer_pop(self.raw_ptr);
        }
    }

    /// Returns the raw pointer to the underlying atomic ring buffer object.
    pub fn raw_ptr(&self) -> usize {
        self.raw_ptr
    }
}

extern "C" {
    pub fn atomic_ring_buffer_new(buf: usize, buf_size: usize, element_size: usize) -> usize;
    pub fn atomic_ring_buffer_emplace(ring_buffer: usize, event: usize) -> usize;
    pub fn atomic_ring_buffer_front(queue: usize) -> usize;
    pub fn atomic_ring_buffer_pop(queue: usize);
}
