//!
//! Kernel services.
//!

#![feature(lang_items)]
#![feature(const_fn)]
#![feature(compiler_builtins_lib)]
#![no_std]

#[macro_use]
extern crate intrusive_collections;

extern crate compiler_builtins;

extern crate rlibc;

#[macro_use]
mod print;
mod panic;
