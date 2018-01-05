//!
//! Manticore kernel.
//!

#![feature(lang_items)]
#![no_std]

#[macro_use]
extern crate kernel;
#[cfg(target_arch = "x86_64")]
extern crate pci;

use core::fmt::Arguments;
use kernel::print;

extern "C" {
    pub fn panic(msg: *const u8);
}

#[cfg(not(test))]
#[lang = "panic_fmt"]
#[no_mangle]
pub extern "C" fn panic_fmt(fmt: Arguments, file: &str, line: u32) {
    println!("Panic: {}:{}: {}", file, line, fmt);
    unsafe {
        panic("Halting\0".as_ptr());
    }

}

#[cfg(not(test))]
#[lang = "eh_personality"]
#[no_mangle]
pub extern "C" fn rust_eh_personality() {}

#[cfg(target_arch = "x86_64")]
pub use pci::pci_probe;
