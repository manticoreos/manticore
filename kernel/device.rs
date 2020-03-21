use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;
use event::EventListener;
use intrusive_collections::{KeyAdapter, RBTree, RBTreeLink};
use ioqueue::IOCmd;
use vm::VMAddressSpace;

// Keep this up-to-date with include/uapi/manticore/config_abi.h.
pub const CONFIG_ETHERNET_MAC_ADDRESS: i32 = 0;

/// A device descriptor.
pub struct DeviceDesc(i32);

/// A device configuration option.
pub type ConfigOption = i32;

impl DeviceDesc {
    pub fn from_user(desc: i32) -> DeviceDesc {
        DeviceDesc(desc)
    }

    pub fn to_user(&self) -> i32 {
        self.0
    }

    pub fn to_idx(&self) -> Option<usize> {
        if self.0 >= 0 {
            Some(self.0 as usize)
        } else {
            None
        }
    }
}

pub trait DeviceOps {
    fn acquire(&self, vmspace: &mut VMAddressSpace, listener: Rc<dyn EventListener>);
    fn subscribe(&self, events: &'static str);
    fn io_submit(&self, cmd: IOCmd);
    fn get_config(&self, option: ConfigOption) -> Option<Vec<u8>>;
}

pub struct Device {
    name: &'static str,
    ops: RefCell<Rc<dyn DeviceOps>>,
    link: RBTreeLink,
}

intrusive_adapter!(pub DeviceAdapter = Rc<Device>: Device { link: RBTreeLink });

impl Device {
    pub fn new(name: &'static str, ops: RefCell<Rc<dyn DeviceOps>>) -> Self {
        Device {
            name: name,
            ops: ops,
            link: RBTreeLink::new(),
        }
    }

    pub fn acquire(&self, vmspace: &mut VMAddressSpace, listener: Rc<dyn EventListener>) {
        self.ops.borrow().acquire(vmspace, listener);
    }

    pub fn subscribe(&self, events: &'static str) {
        self.ops.borrow().subscribe(events);
    }

    pub fn get_config(&self, option: ConfigOption) -> Option<Vec<u8>> {
        return self.ops.borrow().get_config(option);
    }
}

impl<'a> KeyAdapter<'a> for DeviceAdapter {
    type Key = &'static str;

    fn get_key(&self, x: &'a Device) -> Self::Key {
        x.name
    }
}

pub struct Namespace {
    devices: RBTree<DeviceAdapter>,
}

impl Namespace {
    pub const fn new() -> Self {
        Namespace {
            devices: RBTree::new(DeviceAdapter::NEW),
        }
    }

    pub fn add(&mut self, device: Rc<Device>) {
        self.devices.insert(device);
    }

    pub fn lookup(&self, name: &'static str) -> Option<Rc<Device>> {
        let cursor = self.devices.find(name);
        return cursor.clone_pointer();
    }
}

pub static mut NAMESPACE: Namespace = Namespace::new();

pub struct DeviceSpace {
    desc_table: Vec<Rc<Device>>,
}

impl DeviceSpace {
    pub fn new() -> Self {
        DeviceSpace { desc_table: vec![] }
    }

    pub fn attach(&mut self, device: Rc<Device>) -> DeviceDesc {
        let idx = self.desc_table.len();
        self.desc_table.push(device);
        return DeviceDesc::from_user(idx as i32);
    }

    pub fn lookup(&self, desc: DeviceDesc) -> Option<Rc<Device>> {
        if let Some(idx) = desc.to_idx() {
            if idx > self.desc_table.len() {
                return None;
            }
            return Some(self.desc_table[idx].clone());
        }
        return None;
    }
}

/// Register a device to the kernel.
pub fn register_device(device: Rc<Device>) {
    unsafe {
        NAMESPACE.add(device);
    }
}
