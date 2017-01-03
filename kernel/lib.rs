//!
//! Kernel services.
//!

#![feature(lang_items)]
#![feature(const_fn)]
#![no_std]

#[macro_use]
extern crate intrusive_collections;

extern crate rlibc;

#[macro_use]
mod print;
mod panic;
