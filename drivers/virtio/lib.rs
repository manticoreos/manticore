//! Virtio drivers.

#![no_std]
#![feature(option_flattening)]

extern crate alloc;
#[macro_use]
extern crate bitflags;
extern crate pci;
#[macro_use]
extern crate intrusive_collections;
#[macro_use]
extern crate kernel;

pub mod virtqueue;
pub mod net;

#[no_mangle]
pub extern "C" fn virtio_register_drivers() {
    unsafe {
        pci::pci_register_driver(&net::VIRTIO_NET_DRIVER);
    }
}
