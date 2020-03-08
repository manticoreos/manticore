///! Driver model.

use alloc::boxed::Box;
use alloc::vec::Vec;
use intrusive_collections::{LinkedList, LinkedListLink};
use core::cell::RefCell;
use core::cmp;
use vm::VMAddressSpace;
use null_terminated::NulStr;
use errno::EINVAL;

use ioqueue::IOCmd;
use user_access;

// Keep this up-to-date with include/uapi/manticore/config_abi.h.
pub const CONFIG_ETHERNET_MAC_ADDRESS: i32 = 0;

/// Device configuration option.
pub type ConfigOption = i32;

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
    fn io_submit(&self, cmd: IOCmd);
    fn get_config(&self, option: ConfigOption) -> Option<Vec<u8>>;
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

#[no_mangle]
pub extern "C" fn device_get_config(dev_name: &'static NulStr, opt: i32, buf: *mut u8, len: usize) -> i32 {
    for dev in unsafe { DEVICE_LIST.iter() } {
        if dev.dev_name == &dev_name[..] {
            if let Some(value) = dev._ops.borrow().get_config(opt) {
                let to_copy = cmp::min(len, value.len());
                unsafe { user_access::memcpy_to_user(buf, value.as_ptr(), to_copy); }
                return 0
            }
        }
    }
    return -EINVAL
}

static mut DEVICE_LIST: LinkedList<DeviceAdapter> = LinkedList::new(DeviceAdapter::NEW);
