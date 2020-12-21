//!
//! Manticore PCI subsystem.
//!

#![no_std]
#![feature(const_fn)]

extern crate alloc;
#[macro_use]
extern crate kernel;
#[macro_use]
extern crate intrusive_collections;

use alloc::rc::Rc;
use core::intrinsics::transmute;
use intrusive_collections::{LinkedList, LinkedListLink, UnsafeRef};
use kernel::device::{register_device, Device};
use kernel::ioport::IOPort;
use kernel::print;

const MSIX_ENTRY_SIZE: usize = 16;
const MSIX_ENTRY_ADDR: usize = 0;
const MSIX_ENTRY_DATA: usize = 8;
const MSIX_ENTRY_CONTROL: usize = 12;

#[repr(C)]
pub struct MSIMessage {
    msg_addr: u64,
    msg_data: u32,
}

impl MSIMessage {
    fn compose(vector: u8) -> MSIMessage {
        let mut msi_msg = MSIMessage {
            msg_addr: 0,
            msg_data: 0,
        };
        unsafe {
            apic_compose_msi_msg(&mut msi_msg, vector, 0);
        }
        msi_msg
    }
}

extern "C" {
    pub fn apic_compose_msi_msg(msg: *mut MSIMessage, vector: u8, dest_id: u8);
}

pub const PCI_VENDOR_ID_REDHAT: u16 = 0x1af4;
pub const PCI_DEVICE_ID_ANY: u16 = 0xffff;

/* 7.5.1. Type 0/1 Common Configuration Space */
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
const PCI_CMD_INTX_DISABLE: u16 = 1 << 10;
const PCI_CMD_BAR_MEM_ENABLE: u16 = 1 << 1;
const PCI_CMD_BAR_IO_ENABLE: u16 = 1 << 0;

const PCI_STATUS_CAPABILITIES_LIST: u8 = 1 << 4;

const PCI_CAP_ID_OFFSET: u8 = 0x00;
const PCI_CAP_NEXT_OFFSET: u8 = 0x01;

pub const PCI_CAPABILITY_VENDOR: u8 = 0x09;
const PCI_CAPABILITY_MSIX: u8 = 0x11;

// Message control for MSI-X:
const PCI_MSIX_MSG_CTRL: u8 = 0x02;
const PCI_MSIX_MSG_CTRL_MSIX_ENABLE: u16 = 1 << 15;
const PCI_MSIX_MSG_CTRL_FUNCTION_MASK: u16 = 1 << 14;
const PCI_MSIX_MSG_CTRL_TABLE_SIZE: u16 = (1 << 11) - 1;

// Table offset/Table BIR for MSI-X:
const PCI_MSIX_TABLE_OFFSET: u8 = 0x04;
const PCI_MSIX_TABLE_OFFSET_MASK: u32 = 0x7ffffff8;
const PCI_MSIX_TABLE_BIR_MASK: u32 = 0x00000007;

// Vector Control for MSI-X Table Entries:
const PCI_MSIX_ENTRY_VECTOR_CTRL_MASK_BIT: u32 = 1 << 0;

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
    IO {
        iobase: u16,
        size: u32,
    },
}

impl BAR {
    pub unsafe fn remap(&self) -> IOPort {
        match self {
            &BAR::Memory {
                base_addr, size, ..
            } => {
                let addr = ioremap(base_addr as usize, size as usize);
                return IOPort::Memory {
                    base_addr: addr as usize,
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
pub struct MSIX {
    pub offset: u8,
    pub table_bar: u8,
    pub table_offset: u64,
}

#[derive(Debug)]
pub struct PCIDevice {
    pub func: PCIFunction,
    pub dev_id: DeviceID,
    pub bars: [Option<IOPort>; 6],
    pub msix: Option<MSIX>,
}

impl PCIDevice {
    pub fn probe(func: PCIFunction, header_type: u8) -> Rc<PCIDevice> {
        let vendor_id = func.read_config_u16(PCI_CFG_VENDOR_ID);
        let device_id = func.read_config_u16(PCI_CFG_DEVICE_ID);
        let revision_id = func.read_config_u8(PCI_CFG_REVISION_ID);
        let class_code = func.read_config_u8(PCI_CFG_CLASS_CODE);
        let subclass = func.read_config_u8(PCI_CFG_SUBCLASS);
        let prog_if = func.read_config_u8(PCI_CFG_PROG_IF);
        let mut bars: [Option<IOPort>; 6] = [None, None, None, None, None, None];
        let max_bars = match header_type & PCI_HEADER_TYPE_MASK {
            PCI_HEADER_TYPE_DEVICE => 6,
            PCI_HEADER_TYPE_PCCARD => 2,
            _ => 0,
        };
        let mut bar_idx = 0;
        while bar_idx < max_bars {
            let (bar, next_idx) = BAR::decode(&func, bar_idx);
            bars[bar_idx] = bar.map(|bar| unsafe { bar.remap() });
            bar_idx = next_idx;
        }
        let msix = func.find_capability(PCI_CAPABILITY_MSIX).map(|off| {
            let msix_table = func.read_config_u32(off + PCI_MSIX_TABLE_OFFSET);
            let bir = (msix_table & PCI_MSIX_TABLE_BIR_MASK) as u8;
            let table_off = (msix_table & PCI_MSIX_TABLE_OFFSET_MASK) as u64;
            MSIX {
                offset: off,
                table_bar: bir,
                table_offset: table_off,
            }
        });

        let mut cmd = func.read_config_u16(PCI_CFG_COMMAND);
        cmd |= PCI_CMD_BAR_MEM_ENABLE;
        cmd |= PCI_CMD_BAR_IO_ENABLE;
        func.write_config_u16(PCI_CFG_COMMAND, cmd);

        let pci_dev = Rc::new(PCIDevice {
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
            msix: msix,
        });
        for driver in unsafe { PCI_DRIVER_LIST.iter() } {
            if driver.dev_id.vendor_id != pci_dev.dev_id.vendor_id {
                continue
            }
            if driver.dev_id.device_id != pci_dev.dev_id.device_id {
                continue
            }
            if let Some(dev) = (driver.probe)(pci_dev.clone()) {
                register_device(dev);
            }
        }
        return pci_dev;
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

    pub fn enable_msix(&self) {
        if let Some(ref msix) = self.msix {
            self.disable_intx();

            let mut control = self.func.read_config_u16(msix.offset + PCI_MSIX_MSG_CTRL);

            control |= PCI_MSIX_MSG_CTRL_MSIX_ENABLE;
            control |= PCI_MSIX_MSG_CTRL_FUNCTION_MASK;
            self.func
                .write_config_u16(msix.offset + PCI_MSIX_MSG_CTRL, control);

            let table_size = control & PCI_MSIX_MSG_CTRL_TABLE_SIZE + 1;
            for id in 0..table_size {
                self.mask_msix_entry(id);
            }

            control = self.func.read_config_u16(msix.offset + PCI_MSIX_MSG_CTRL);
            control &= !PCI_MSIX_MSG_CTRL_FUNCTION_MASK;
            self.func
                .write_config_u16(msix.offset + PCI_MSIX_MSG_CTRL, control);
        }
    }

    pub fn disable_intx(&self) {
        let mut cmd = self.func.read_config_u16(PCI_CFG_COMMAND);
        cmd |= PCI_CMD_INTX_DISABLE;
        self.func.write_config_u16(PCI_CFG_COMMAND, cmd);
    }

    pub fn register_irq(&self, entry: u16, handler: extern "C" fn(arg: usize), arg: usize) -> i32 {
        let vector = unsafe { request_irq(handler, transmute(arg)) };
        let msi_msg = MSIMessage::compose(vector as u8);
        self.write_msix_entry(entry, msi_msg.msg_addr, msi_msg.msg_data);
        vector
    }

    pub fn enable_irq(&self, entry: u16) {
        self.unmask_msix_entry(entry);
    }

    pub fn write_msix_entry(&self, id: u16, addr: u64, data: u32) {
        if let Some(ioport) = self.get_msix_table() {
            let entry_addr = self.get_entry_addr(id);
            ioport.write64(addr, entry_addr + MSIX_ENTRY_ADDR);
            ioport.write32(data, entry_addr + MSIX_ENTRY_DATA);
        }
    }

    pub fn mask_msix_entry(&self, id: u16) {
        let mut ctrl_data = self.read_msix_entry_ctrl(id);
        ctrl_data |= PCI_MSIX_ENTRY_VECTOR_CTRL_MASK_BIT;
        self.write_msix_entry_ctrl(id, ctrl_data);
    }

    pub fn unmask_msix_entry(&self, id: u16) {
        let mut ctrl_data = self.read_msix_entry_ctrl(id);
        ctrl_data &= !(PCI_MSIX_ENTRY_VECTOR_CTRL_MASK_BIT);
        self.write_msix_entry_ctrl(id, ctrl_data);
    }

    fn read_msix_entry_ctrl(&self, id: u16) -> u32 {
        if let Some(ioport) = self.get_msix_table() {
            let entry_addr = self.get_entry_addr(id);
            let ctrl = entry_addr + MSIX_ENTRY_CONTROL;
            return ioport.read32(ctrl);
        }
        return 0;
    }

    fn write_msix_entry_ctrl(&self, id: u16, ctrl_data: u32) {
        if let Some(ioport) = self.get_msix_table() {
            let entry_addr = self.get_entry_addr(id);
            let ctrl = entry_addr + MSIX_ENTRY_CONTROL;
            ioport.write32(ctrl_data, ctrl);
        }
    }

    fn get_entry_addr(&self, id: u16) -> usize {
        return id as usize * MSIX_ENTRY_SIZE;
    }

    pub fn get_msix_table(&self) -> &Option<IOPort> {
        if let Some(ref msix) = self.msix {
            return &self.bars[msix.table_bar as usize];
        }
        return &None;
    }
}

extern "C" {
    pub fn request_irq(handler: extern "C" fn(arg: usize), arg: usize) -> i32;
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
        let dev = PCIDevice::probe(func, header_type);
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
            if dev.msix.is_some() { "[msix]" } else { "" },
        );
    }
    return result;
}

type PCIProbe = fn(Rc<PCIDevice>) -> Option<Rc<Device>>;

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

static mut PCI_DRIVER_LIST: LinkedList<PCIDriverAdapter> = LinkedList::new(PCIDriverAdapter::NEW);

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
    pub fn ioremap(paddr: usize, size: usize) -> u64;
}
