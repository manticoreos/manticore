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

use intrusive_collections::{Bound, UnsafeRef, RBTree, RBTreeLink, KeyAdapter};
use alloc::allocator::{Alloc, Layout, AllocErr};
use core::intrinsics::transmute;
use core::mem::size_of;
use print;

const KIB: u64 = 1 << 10;
const MIB: u64 = 1 << 20;

const PAGE_SIZE_4K: u64 = 4 * KIB;
const PAGE_SIZE_2M: u64 = 2 * MIB;

/// Memory segment.
///
/// A memory segment is a contiguous span of memory.
struct MemorySegment {
    /// Base address of the memory segment.
    pub base: u64,
    /// Size of the memory segment.
    pub size: u64,
    /// Link in segments red-black tree.
    pub segments_link: RBTreeLink,
}

intrusive_adapter!(SegmentsAdaptor = UnsafeRef<MemorySegment>: MemorySegment { segments_link: RBTreeLink });

impl<'a> KeyAdapter<'a> for SegmentsAdaptor {
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
    /// Memory segments, ordered by base address.
    segments: RBTree<SegmentsAdaptor>,
}

impl MemoryArena {
    const fn new() -> Self {
        MemoryArena { segments: RBTree::new(SegmentsAdaptor::new()) }
    }

    unsafe fn alloc(&mut self, size: u64) -> *mut u8 {
        let mut cur = self.segments.front_mut();
        match cur.get() {
            Some(seg) => {
                if seg.size < size {
                    return transmute(0u64);
                }
                let ret = seg.base;
                if seg.size > size {
                    let new_seg = seg.split(size);
                    let reference = UnsafeRef::from_raw(new_seg);
                    cur.replace_with(reference.clone());
                } else {
                    cur.remove();
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
        let reference = UnsafeRef::from_raw(seg);
        self.segments.insert(reference.clone());
    }

    #[allow(dead_code)]
    fn dump(&self) {
        println!("Segments:");
        for seg in self.segments.iter() {
            println!("  segment: start={}, size={}", seg.base, seg.size);
        }
    }
}

/// The kernel small page arena.
static mut KERNEL_SMALL_PAGE_ARENA: MemoryArena = MemoryArena::new();

static mut KERNEL_LARGE_PAGE_ARENA: MemoryArena = MemoryArena::new();

/// Register a memory span to the kernel arena.
#[no_mangle]
pub extern "C" fn memory_add_span(start: u64, size: u64) {
    let end = start + size;
    let large_start = align_up(start, PAGE_SIZE_2M);
    let large_end = align_down(end, PAGE_SIZE_2M);
    if start != large_start {
        memory_add_span_small(start, large_start);
    }
    if large_start != large_end {
        memory_add_span_large(large_start, large_end);
    }
    if end != large_end {
        memory_add_span_small(large_end, end);
    }
}

fn align_down(value: u64, align: u64) -> u64 {
    value & !(align - 1)
}

fn align_up(value: u64, align: u64) -> u64 {
    align_down(value + align - 1, align)
}

fn memory_add_span_small(start: u64, end: u64) {
    unsafe {
        let mut seg : &mut MemorySegment = transmute(start);
        *seg = MemorySegment::new(start, end-start);
        KERNEL_SMALL_PAGE_ARENA.add_to_freelist(seg);
    }
}

fn memory_add_span_large(start: u64, end: u64) {
    unsafe {
        let mut seg : &mut MemorySegment = transmute(start);
        *seg = MemorySegment::new(start, end-start);
        KERNEL_LARGE_PAGE_ARENA.add_to_freelist(seg);
    }
}

/// Initialize the page allocator.
#[no_mangle]
pub extern "C" fn page_alloc_init() {}

/// Allocate a small page.
#[no_mangle]
pub extern "C" fn page_alloc_small() -> *mut u8 {
    unsafe {
        return KERNEL_SMALL_PAGE_ARENA.alloc(PAGE_SIZE_4K);
    }
}

/// Free a small page.
#[no_mangle]
pub extern "C" fn page_free_small(addr: *mut u8) {
    unsafe {
        return KERNEL_SMALL_PAGE_ARENA.free(addr, PAGE_SIZE_4K);
    }
}

/// Allocate a large page.
#[no_mangle]
pub extern "C" fn page_alloc_large() -> *mut u8 {
    unsafe {
        return KERNEL_LARGE_PAGE_ARENA.alloc(PAGE_SIZE_2M);
    }
}

/// Free a large page.
#[no_mangle]
pub extern "C" fn page_free_large(addr: *mut u8) {
    unsafe {
        return KERNEL_LARGE_PAGE_ARENA.free(addr, PAGE_SIZE_2M);
    }
}

pub struct KAllocator;

impl KAllocator {
    pub const fn new() -> KAllocator {
        KAllocator {}
    }
}

extern "C" {
    pub fn kmem_alloc(size: usize) -> usize;
    pub fn kmem_zalloc(size: usize) -> usize;
    pub fn kmem_free(ptr: *mut u8, size: usize);
}

unsafe impl<'a> Alloc for &'a KAllocator {
    unsafe fn alloc(&mut self, layout: Layout) -> Result<*mut u8, AllocErr> {
        if layout.align() != size_of::<usize>() {
            return Err(AllocErr::Unsupported { details: "Unsupported alignment" });
        }
        let obj = kmem_alloc(layout.size());
        if obj == 0 {
            return Err(AllocErr::Exhausted { request: layout });
        }
        Ok(transmute(obj))
    }

    unsafe fn dealloc(&mut self, ptr: *mut u8, layout: Layout) {
        kmem_free(ptr, layout.size());
    }
}
