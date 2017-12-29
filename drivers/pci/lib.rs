//!
//! Manticore PCI subsystem.
//!

#![no_std]

#[macro_use]
extern crate kernel;

use core::intrinsics::transmute;
use kernel::print;

const PCI_VENDOR_ID: u8 = 0x00;
const PCI_DEVICE_ID: u8 = 0x02;
const PCI_CLASS_REVISION: u8 = 0x08;
const PCI_CFG_REVISION_ID: u8 = 0x08;
const PCI_CFG_CLASS_CODE: u8 = 0x08;
const PCI_CFG_SUBCLASS: u8 = 0x08;
const PCI_CFG_PROG_IF: u8 = 0x08;
const PCI_CONFIG_HEADER_TYPE: u8 = 0x0e;
const PCI_CONFIG_BARS: u8 = 0x10;
const PCI_CONFIG_SECONDARY_BUS: u8 = 0x19;

const PCI_HEADER_TYPE_MASK: u8 = 0x03;
const PCI_HEADER_TYPE_DEVICE: u8 = 0x00;
const PCI_HEADER_TYPE_BRIDGE: u8 = 0x01;
const PCI_HEADER_TYPE_PCCARD: u8 = 0x02;

/// I/O port that is either MMIO or PIO.
pub enum IOPort {
    Memory { base_addr: usize, size: u32 },
    IO { iobase: u16, size: u32 },
}

impl IOPort {
    unsafe fn read8(&self, offset: usize) -> u8 {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u8 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { iobase, size } => {
                return inb(iobase + (offset as u16));
            }
        }
    }
    unsafe fn write8(&self, value: u8, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u8 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { iobase, size } => {
                outb(value, iobase + (offset as u16));
            }
        }
    }
    unsafe fn read16(&self, offset: usize) -> u16 {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u16 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { iobase, size } => {
                return inw(iobase + (offset as u16));
            }
        }
    }
    unsafe fn write16(&self, value: u16, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u16 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { iobase, size } => {
                outw(value, iobase + (offset as u16));
            }
        }
    }
    unsafe fn read32(&self, offset: usize) -> u32 {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u32 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { iobase, size } => {
                return inl(iobase + (offset as u16));
            }
        }
    }
    unsafe fn write32(&self, value: u32, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u32 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { iobase, size } => {
                outl(value, iobase + (offset as u16));
            }
        }
    }
    unsafe fn read64(&self, offset: usize) -> u64 {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u64 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { iobase, size } => {
                unimplemented!();
            }
        }
    }
    unsafe fn write64(&self, value: u64, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, size } => {
                let mut addr: *mut u64 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { iobase, size } => {
                unimplemented!();
            }
        }
    }
}

/// PCI device identification
#[derive(Debug)]
pub struct DeviceID {
    pub vendor_id: u16,
    pub device_id: u16,
    pub revision_id: u8,
    pub header_type: u8,
    pub class_code: u8,
    pub subclass: u8,
    pub prog_if: u8,
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
    unsafe fn remap(&self) -> IOPort {
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

    fn decode(bus: u16, slot: u16, func: u16, bar_idx: usize) -> (Option<BAR>, usize) {
        let offset = PCI_CONFIG_BARS + (bar_idx << 2) as u8;
        let raw_bar = unsafe { pci_config_read_u32(bus, slot, func, offset) };
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
                    let next_offset = PCI_CONFIG_BARS + ((bar_idx + 1) << 2) as u8;
                    let next_bar = unsafe { pci_config_read_u32(bus, slot, func, next_offset) };
                    let base_addr: u64 = ((raw_bar & !0xf) as u64) | ((next_bar as u64) << 32);
                    (Some(base_addr), Some(Locatable::Bits64))
                }
                _ => (None, None),
            };
            match (base_addr, locatable) {
                (Some(base_addr), Some(locatable)) => {
                    let bar = unsafe {
                        pci_config_write_u32(bus, slot, func, offset, !0);
                        let size = !pci_config_read_u32(bus, slot, func, offset) + 1;
                        pci_config_write_u32(bus, slot, func, offset, raw_bar);
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
            let bar = unsafe {
                pci_config_write_u32(bus, slot, func, offset, !0);
                let size = !pci_config_read_u32(bus, slot, func, offset) + 1;
                pci_config_write_u32(bus, slot, func, offset, raw_bar);
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
struct PCIDevice {
    pub bus: u16,
    pub slot: u16,
    pub func: u16,
    pub dev_id: DeviceID,
    pub bars: [Option<BAR>; 6],
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
        let vendor_id = unsafe { pci_config_read_u16(bus, slot, 0, PCI_VENDOR_ID) };
        if vendor_id == 0xffff {
            continue;
        }
        result |= pci_probe_slot(bus, slot);
    }
    return result;
}

fn pci_probe_slot(bus: u16, slot: u16) -> bool {
    let mut result = false;
    for func in 0..8 {
        let class_revision = unsafe { pci_config_read_u32(bus, slot, func, PCI_CLASS_REVISION) };
        if class_revision == 0xffffffff {
            continue;
        }
        let header_type = unsafe { pci_config_read_u8(bus, slot, func, PCI_CONFIG_HEADER_TYPE) };
        if header_type & PCI_HEADER_TYPE_MASK == PCI_HEADER_TYPE_BRIDGE {
            let secondary_bus =
                unsafe { pci_config_read_u8(bus, slot, func, PCI_CONFIG_SECONDARY_BUS) };
            pci_probe_bus(secondary_bus as u16);
            continue;
        }
        result = true;
        let vendor_id = unsafe { pci_config_read_u16(bus, slot, func, PCI_VENDOR_ID) };
        let device_id = unsafe { pci_config_read_u16(bus, slot, func, PCI_DEVICE_ID) };
        let revision_id = unsafe { pci_config_read_u8(bus, slot, func, PCI_CFG_REVISION_ID) };
        let class_code = unsafe { pci_config_read_u8(bus, slot, func, PCI_CFG_CLASS_CODE) };
        let subclass = unsafe { pci_config_read_u8(bus, slot, func, PCI_CFG_SUBCLASS) };
        let prog_if = unsafe { pci_config_read_u8(bus, slot, func, PCI_CFG_PROG_IF) };
        let mut bars = [None, None, None, None, None, None];
        let max_bars = match header_type & PCI_HEADER_TYPE_MASK {
            PCI_HEADER_TYPE_DEVICE => 6,
            PCI_HEADER_TYPE_PCCARD => 2,
            _ => 0,
        };
        let mut bar_idx = 0;
        while bar_idx < max_bars {
            let (bar, next_idx) = BAR::decode(bus, slot, func, bar_idx);
            bars[bar_idx] = bar;
            bar_idx = next_idx;
        }
        let dev = PCIDevice {
            bus: bus,
            slot: slot,
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
        };
        println!(
            "  {:02x}:{:02x}.{:x} {:02x}{:02x}: {:04x}:{:04x} (rev {:x})",
            bus,
            slot,
            func,
            dev.dev_id.class_code,
            dev.dev_id.subclass,
            vendor_id,
            device_id,
            dev.dev_id.revision_id
        );
        for bar_opt in dev.bars.into_iter() {
            match bar_opt {
                &Some(BAR::Memory {
                          base_addr,
                          size,
                          prefetchable,
                          ref locatable,
                      }) => {
                    let bit_width = match locatable {
                        &Locatable::Bits32 => "32-bit",
                        &Locatable::Bits64 => "64-bit",
                    };
                    let prefetch = match prefetchable {
                        true => "prefetchable",
                        false => "non-prefetchable",
                    };
                    println!(
                        "\tMemory at {:x} ({}, {}) [size={}K]",
                        base_addr,
                        bit_width,
                        prefetch,
                        size / 1024
                    );
                }
                &Some(BAR::IO { iobase, size }) => {
                    println!("\tI/O ports at {:x} [size={}]", iobase, size);
                }
                _ => (),
            }
        }
    }
    return result;
}

extern "C" {
    pub fn pci_config_read_u8(bus: u16, slot: u16, func: u16, offset: u8) -> u8;
    pub fn pci_config_read_u16(bus: u16, slot: u16, func: u16, offset: u8) -> u16;
    pub fn pci_config_read_u32(bus: u16, slot: u16, func: u16, offset: u8) -> u32;
    pub fn pci_config_write_u32(bus: u16, slot: u16, func: u16, offset: u8, value: u32);
    pub fn inb(port: u16) -> u8;
    pub fn inw(port: u16) -> u16;
    pub fn inl(port: u16) -> u32;
    pub fn outb(v: u8, port: u16);
    pub fn outw(v: u16, port: u16);
    pub fn outl(v: u32, port: u16);
    pub fn ioremap(paddr: usize, size: usize) -> usize;
}