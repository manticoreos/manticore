///! Driver model.

use alloc::boxed::Box;
use intrusive_collections::{LinkedList, LinkedListLink};
use core::cell::RefCell;

/// Device.
pub struct Device {
    _ops: RefCell<Box<DeviceOps>>,
    link: LinkedListLink,
}

impl Device {
    pub fn new(ops: Box<DeviceOps>) -> Self {
        Device {
            _ops: RefCell::new(ops),
            link: LinkedListLink::new(),
        }
    }
}

intrusive_adapter!(DeviceAdapter = Box<Device>: Device { link: LinkedListLink });

pub trait DeviceOps {
}

/// Register a device to the kernel.
pub fn register_device(device: Device) {
    unsafe {
        DEVICE_LIST.push_back(Box::new(device));
    }
}

static mut DEVICE_LIST: LinkedList<DeviceAdapter> = LinkedList::new(DeviceAdapter::new());
