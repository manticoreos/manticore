//!
//! Manticore kernel.
//!

#![feature(lang_items)]
#![feature(alloc_error_handler)]
#![feature(panic_info_message)]
#![no_std]

#[macro_use]
extern crate kernel;
#[cfg(target_arch = "x86_64")]
extern crate pci;
#[cfg(target_arch = "x86_64")]
extern crate virtio;

use core::panic::PanicInfo;
use kernel::print;

extern "C" {
    pub fn panic(msg: *const u8);
}

#[cfg(not(test))]
#[panic_handler]
#[no_mangle]
pub extern "C" fn panic_handler(info: &PanicInfo) -> ! {
    let (file, line) = match info.location() {
        Some(location) => (location.file(), location.line()),
        None => ("<unknown>", 0),
    };
    if let Some(msg) = info.message() {
        println!("Panic occurred at {}:{}: {}", file, line, msg);
    } else {
        println!("Panic occurred at {}:{}", file, line);
    }
    unsafe {
        panic("Halting\0".as_ptr());
    }
    loop {}
}

#[cfg(not(test))]
#[alloc_error_handler]
#[no_mangle]
pub extern "C" fn oom(_: ::core::alloc::Layout) -> ! {
    panic!("out of memory");
}

#[cfg(not(test))]
#[lang = "eh_personality"]
#[no_mangle]
pub extern "C" fn rust_eh_personality() {}

#[cfg(target_arch = "x86_64")]
pub use pci::pci_probe;

#[cfg(target_arch = "x86_64")]
pub use virtio::virtio_register_drivers;
