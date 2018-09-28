//!
//! Manticore PCI subsystem.
//!

#![no_std]
#![feature(alloc)]
#![feature(const_fn)]

extern crate alloc;
#[macro_use]
extern crate kernel;
#[macro_use]
extern crate intrusive_collections;

pub mod ioport;

use intrusive_collections::{LinkedList, LinkedListLink, UnsafeRef};
use ioport::IOPort;
use kernel::device::{Device, register_device};
use kernel::print;

pub const PCI_VENDOR_ID_REDHAT: u16 = 0x1af4;
pub const PCI_DEVICE_ID_ANY: u16 = 0xffff;

const PCI_CFG_VENDOR_ID: u8 = 0x00;
const PCI_CFG_DEVICE_ID: u8 = 0x02;
const PCI_CFG_COMMAND: u8 = 0x04;
const PCI_CFG_STATUS: u8 = 0x06;
const PCI_CFG_CLASS_REVISION: u8 = 0x08;
const PCI_CFG_REVISION_ID: u8 = 0x08;
const PCI_CFG_CLASS_CODE: u8 = 0x08;
const PCI_CFG_SUBCLASS: u8 = 0x08;
const PCI_CFG_PROG_IF: u8 = 0x08;
const PCI_CFG_HEADER_TYPE: u8 = 0x0e;
const PCI_CFG_BARS: u8 = 0x10;
const PCI_CFG_SECONDARY_BUS: u8 = 0x19;
const PCI_CFG_CAPABILITIES_PTR: u8 = 0x34;

const PCI_HEADER_TYPE_MASK: u8 = 0x03;
const PCI_HEADER_TYPE_DEVICE: u8 = 0x00;
const PCI_HEADER_TYPE_BRIDGE: u8 = 0x01;
const PCI_HEADER_TYPE_PCCARD: u8 = 0x02;

const PCI_CMD_BUS_MASTER: u16 = 1 << 2;

const PCI_STATUS_CAPABILITIES_LIST: u8 = 1 << 4;

const PCI_CAP_ID_OFFSET: u8 = 0x00;
const PCI_CAP_NEXT_OFFSET: u8 = 0x01;

const PCI_CAPABILITY_MSIX: u8 = 0x11;

/// PCI device identification
#[derive(Debug, Clone)]
pub struct DeviceID {
    pub vendor_id: u16,
    pub device_id: u16,
    pub revision_id: u8,
    pub header_type: u8,
    pub class_code: u8,
    pub subclass: u8,
    pub prog_if: u8,
}

impl DeviceID {
    pub const fn new(vendor_id: u16, device_id: u16) -> Self {
        DeviceID {
            vendor_id: vendor_id,
            device_id: device_id,
            revision_id: 0,
            header_type: 0,
            class_code: 0,
            subclass: 0,
            prog_if: 0,
        }
    }
}

#[derive(Debug, Clone)]
pub enum Locatable {
    Bits32,
    Bits64,
}

#[derive(Debug, Clone)]
pub enum BAR {
    Memory {
        base_addr: u64,
        size: u32,
        prefetchable: bool,
        locatable: Locatable,
    },
    IO { iobase: u16, size: u32 },
}

impl BAR {
    pub unsafe fn remap(&self) -> IOPort {
        match self {
            &BAR::Memory { base_addr, size, .. } => {
                let addr = ioremap(base_addr as usize, size as usize);
                return IOPort::Memory {
                    base_addr: addr,
                    size: size,
                };
            }
            &BAR::IO { iobase, size, .. } => {
                return IOPort::IO {
                    iobase: iobase as u16,
                    size: size,
                };
            }
        }
    }

    fn decode(func: &PCIFunction, bar_idx: usize) -> (Option<BAR>, usize) {
        let offset = PCI_CFG_BARS + (bar_idx << 2) as u8;
        let raw_bar = func.read_config_u32(offset);
        if raw_bar == 0 {
            (None, bar_idx + 1)
        } else if raw_bar & 0x01 == 0 {
            let bar_type = (raw_bar & 0b110) >> 1;
            let (base_addr, locatable) = match bar_type {
                0b00 => {
                    let base_addr: u64 = (raw_bar & !0xf) as u64;
                    (Some(base_addr), Some(Locatable::Bits32))
                }
                0b10 => {
                    let next_offset = PCI_CFG_BARS + ((bar_idx + 1) << 2) as u8;
                    let next_bar = func.read_config_u32(next_offset);
                    let base_addr: u64 = ((raw_bar & !0xf) as u64) | ((next_bar as u64) << 32);
                    (Some(base_addr), Some(Locatable::Bits64))
                }
                _ => (None, None),
            };
            match (base_addr, locatable) {
                (Some(base_addr), Some(locatable)) => {
                    let bar = {
                        func.write_config_u32(offset, !0);
                        let size = !func.read_config_u32(offset) + 1;
                        func.write_config_u32(offset, raw_bar);
                        let prefetchable = raw_bar & 0b1000 != 0;
                        BAR::Memory {
                            base_addr: base_addr,
                            size: size,
                            prefetchable: prefetchable,
                            locatable: locatable.clone(),
                        }
                    };
                    let next_idx = match locatable {
                        Locatable::Bits32 => bar_idx + 1,
                        Locatable::Bits64 => bar_idx + 2,
                    };
                    (Some(bar), next_idx)
                }
                _ => (None, bar_idx + 1),
            }
        } else {
            let bar = {
                func.write_config_u32(offset, !0);
                let size = !func.read_config_u32(offset) + 1;
                func.write_config_u32(offset, raw_bar);
                BAR::IO {
                    iobase: (raw_bar & !0x03) as u16,
                    size: size,
                }
            };
            (Some(bar), bar_idx + 1)
        }
    }
}

#[derive(Debug)]
pub struct PCIFunction {
    pub bus_id: u16,
    pub slot_id: u16,
    pub func_id: u16,
}

impl PCIFunction {
    pub fn new(bus_id: u16, slot_id: u16, func_id: u16) -> Self {
        PCIFunction {
            bus_id: bus_id,
            slot_id: slot_id,
            func_id: func_id,
        }
    }

    pub fn find_capability(&self, cap_id: u8) -> Option<u8> {
        let status = self.read_config_u8(PCI_CFG_STATUS);
        if status & PCI_STATUS_CAPABILITIES_LIST == 0 {
            return None;
        }
        let mut offset = self.read_config_u8(PCI_CFG_CAPABILITIES_PTR);
        while offset != 0 {
            let id = self.read_config_u8(offset + PCI_CAP_ID_OFFSET);
            if id == cap_id {
                return Some(offset);
            }
            offset = self.read_config_u8(offset + PCI_CAP_NEXT_OFFSET);
        }
        None
    }

    pub fn find_next_capability(&self, cap_id: u8, pos: u8) -> Option<u8> {
        let mut offset = self.read_config_u8(pos + PCI_CAP_NEXT_OFFSET);
        while offset != 0 {
            let id = self.read_config_u8(offset + PCI_CAP_ID_OFFSET);
            if id == cap_id {
                return Some(offset);
            }
            offset = self.read_config_u8(offset + PCI_CAP_NEXT_OFFSET);
        }
        None
    }

    pub fn read_config_u8(&self, offset: u8) -> u8 {
        unsafe { pci_config_read_u8(self.bus_id, self.slot_id, self.func_id, offset) }
    }

    pub fn write_config_u8(&self, offset: u8, value: u8) {
        unsafe { pci_config_write_u8(self.bus_id, self.slot_id, self.func_id, offset, value) }
    }

    pub fn read_config_u16(&self, offset: u8) -> u16 {
        unsafe { pci_config_read_u16(self.bus_id, self.slot_id, self.func_id, offset) }
    }

    pub fn write_config_u16(&self, offset: u8, value: u16) {
        unsafe { pci_config_write_u16(self.bus_id, self.slot_id, self.func_id, offset, value) }
    }

    pub fn read_config_u32(&self, offset: u8) -> u32 {
        unsafe { pci_config_read_u32(self.bus_id, self.slot_id, self.func_id, offset) }
    }

    pub fn write_config_u32(&self, offset: u8, value: u32) {
        unsafe { pci_config_write_u32(self.bus_id, self.slot_id, self.func_id, offset, value) }
    }
}

#[derive(Debug)]
pub struct PCIDevice {
    pub func: PCIFunction,
    pub dev_id: DeviceID,
    pub bars: [Option<BAR>; 6],
    pub msix_offset: Option<u8>,
}

impl PCIDevice {
    pub fn parse_config(func: PCIFunction, header_type: u8) -> PCIDevice {
        let vendor_id = func.read_config_u16(PCI_CFG_VENDOR_ID);
        let device_id = func.read_config_u16(PCI_CFG_DEVICE_ID);
        let revision_id = func.read_config_u8(PCI_CFG_REVISION_ID);
        let class_code = func.read_config_u8(PCI_CFG_CLASS_CODE);
        let subclass = func.read_config_u8(PCI_CFG_SUBCLASS);
        let prog_if = func.read_config_u8(PCI_CFG_PROG_IF);
        let mut bars = [None, None, None, None, None, None];
        let max_bars = match header_type & PCI_HEADER_TYPE_MASK {
            PCI_HEADER_TYPE_DEVICE => 6,
            PCI_HEADER_TYPE_PCCARD => 2,
            _ => 0,
        };
        let mut bar_idx = 0;
        while bar_idx < max_bars {
            let (bar, next_idx) = BAR::decode(&func, bar_idx);
            bars[bar_idx] = bar;
            bar_idx = next_idx;
        }
        let msix_offset = func.find_capability(PCI_CAPABILITY_MSIX);
        PCIDevice {
            func: func,
            dev_id: DeviceID {
                vendor_id: vendor_id,
                device_id: device_id,
                revision_id: revision_id,
                header_type: header_type,
                class_code: class_code,
                subclass: subclass,
                prog_if: prog_if,
            },
            bars: bars,
            msix_offset: msix_offset,
        }
    }

    pub fn probe(&self) {
        unsafe {
            for driver in PCI_DRIVER_LIST.iter() {
                if driver.dev_id.vendor_id == self.dev_id.vendor_id {
                    let dev = (driver.probe)(self);
                    register_device(dev);
                }
            }
        }
    }

    pub fn set_bus_master(&self, master: bool) {
        let mut cmd = self.func.read_config_u16(PCI_CFG_COMMAND);
        if master {
            cmd |= PCI_CMD_BUS_MASTER;
        } else {
            cmd &= !PCI_CMD_BUS_MASTER;
        }
        self.func.write_config_u16(PCI_CFG_COMMAND, cmd);
    }
}

#[no_mangle]
pub extern "C" fn pci_probe() {
    println!("Probing PCI devices ...");

    for bus in 0..256 {
        if !pci_probe_bus(bus) {
            break;
        }
    }
}

fn pci_probe_bus(bus: u16) -> bool {
    let mut result = false;
    for slot in 0..32 {
        let vendor_id = unsafe { pci_config_read_u16(bus, slot, 0, PCI_CFG_VENDOR_ID) };
        if vendor_id == 0xffff {
            continue;
        }
        result |= pci_probe_slot(bus, slot);
    }
    return result;
}

fn pci_probe_slot(bus_id: u16, slot_id: u16) -> bool {
    let mut result = false;
    for func_id in 0..8 {
        let func = PCIFunction::new(bus_id, slot_id, func_id);
        let class_revision = func.read_config_u32(PCI_CFG_CLASS_REVISION);
        if class_revision == 0xffffffff {
            continue;
        }
        let header_type = func.read_config_u8(PCI_CFG_HEADER_TYPE);
        if header_type & PCI_HEADER_TYPE_MASK == PCI_HEADER_TYPE_BRIDGE {
            let secondary_bus = func.read_config_u8(PCI_CFG_SECONDARY_BUS);
            pci_probe_bus(secondary_bus as u16);
            continue;
        }
        result = true;
        let dev = PCIDevice::parse_config(func, header_type);
        println!(
            "  {:02x}:{:02x}.{:x} {:02x}{:02x}: {:04x}:{:04x} (rev {:x}) {}",
            dev.func.bus_id,
            dev.func.slot_id,
            dev.func.func_id,
            dev.dev_id.class_code,
            dev.dev_id.subclass,
            dev.dev_id.vendor_id,
            dev.dev_id.device_id,
            dev.dev_id.revision_id,
            if dev.msix_offset.is_some() { "[msix]" } else { "" },
        );
        dev.probe();
    }
    return result;
}

type PCIProbe = fn(&PCIDevice) -> Device;

pub struct PCIDriver {
    dev_id: DeviceID,
    probe: PCIProbe,
    link: LinkedListLink,
}

intrusive_adapter!(PCIDriverAdapter = UnsafeRef<PCIDriver>: PCIDriver { link: LinkedListLink });

impl PCIDriver {
    pub const fn new(dev_id: DeviceID, probe: PCIProbe) -> Self {
        PCIDriver {
            dev_id: dev_id,
            probe: probe,
            link: LinkedListLink::new(),
        }
    }
}

static mut PCI_DRIVER_LIST: LinkedList<PCIDriverAdapter> = LinkedList::new(PCIDriverAdapter::new());

pub fn pci_register_driver(driver: &PCIDriver) {
    unsafe {
        PCI_DRIVER_LIST.push_back(UnsafeRef::from_raw(driver));
    }
}

extern "C" {
    pub fn pci_config_read_u8(bus: u16, slot: u16, func: u16, offset: u8) -> u8;
    pub fn pci_config_write_u8(bus: u16, slot: u16, func: u16, offset: u8, value: u8);
    pub fn pci_config_read_u16(bus: u16, slot: u16, func: u16, offset: u8) -> u16;
    pub fn pci_config_write_u16(bus: u16, slot: u16, func: u16, offset: u8, value: u16);
    pub fn pci_config_read_u32(bus: u16, slot: u16, func: u16, offset: u8) -> u32;
    pub fn pci_config_write_u32(bus: u16, slot: u16, func: u16, offset: u8, value: u32);
    pub fn ioremap(paddr: usize, size: usize) -> usize;
}
