//!
//! Kernel services.
//!

#![feature(drop_types_in_const)]
#![feature(lang_items)]
#![feature(const_fn)]
#![feature(asm)]
#![no_std]

#[macro_use]
extern crate intrusive_collections;
extern crate xmas_elf;
extern crate rlibc;

#[macro_use]
mod print;
mod panic;
mod memory;
mod elf;

pub use memory::memory_add_span;
pub use memory::page_alloc_init;
pub use memory::page_alloc_small;
pub use memory::page_free_small;
pub use memory::page_alloc_large;
pub use memory::page_free_large;
pub use elf::parse_elf;
