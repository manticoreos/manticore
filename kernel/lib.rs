//!
//! Kernel services.
//!

#![feature(allocator_api)]
#![feature(box_syntax)]
#![feature(const_fn)]
#![feature(asm)]
#![no_std]

#[macro_use]
extern crate alloc;
#[macro_use]
extern crate bitflags;
#[macro_use]
extern crate intrusive_collections;
extern crate null_terminated;
extern crate rlibc;
extern crate xmas_elf;

pub mod atomic_ring_buffer;
pub mod event;
pub mod errno;
#[macro_use]
pub mod print;
pub mod memory;
pub mod mmu;
pub mod vm;
pub mod process;
pub mod sched;
pub mod device;
pub mod ioport;
pub mod ioqueue;
pub mod user_access;

pub use memory::memory_add_span;
pub use memory::page_alloc_init;
pub use memory::page_alloc_small;
pub use memory::page_free_small;
pub use memory::page_alloc_large;
pub use memory::page_free_large;
pub use process::process_spawn;

use memory::KAllocator;

#[global_allocator]
static mut ALLOCATOR: KAllocator = KAllocator::new();
