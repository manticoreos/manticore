///! Driver model.

use alloc::boxed::Box;
use intrusive_collections::{LinkedList, LinkedListLink};
use core::cell::RefCell;
use vm::VMAddressSpace;

/// Device.
pub struct Device {
    dev_name: &'static str,
    _ops: RefCell<Box<dyn DeviceOps>>,
    link: LinkedListLink,
}

impl Device {
    pub fn new(dev_name: &'static str, ops: Box<dyn DeviceOps>) -> Self {
        Device {
            dev_name: dev_name,
            _ops: RefCell::new(ops),
            link: LinkedListLink::new(),
        }
    }
}

intrusive_adapter!(DeviceAdapter = Box<Device>: Device { link: LinkedListLink });

pub trait DeviceOps {
    fn subscribe(&self, vmspace: &mut VMAddressSpace);
}

/// Register a device to the kernel.
pub fn register_device(device: Device) {
    unsafe {
        DEVICE_LIST.push_back(Box::new(device));
    }
}

pub fn subscribe_device(dev_name: &'static str, vmspace: &mut VMAddressSpace) {
    unsafe {
        for dev in DEVICE_LIST.iter() {
            if dev.dev_name == dev_name {
                dev._ops.borrow().subscribe(vmspace);
            }
        }
    }
}

static mut DEVICE_LIST: LinkedList<DeviceAdapter> = LinkedList::new(DeviceAdapter::NEW);
