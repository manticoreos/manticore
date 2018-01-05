//! PCI I/O port accessors.

use core::intrinsics::transmute;

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

extern "C" {
    pub fn inb(port: u16) -> u8;
    pub fn inw(port: u16) -> u16;
    pub fn inl(port: u16) -> u32;
    pub fn outb(v: u8, port: u16);
    pub fn outw(v: u16, port: u16);
    pub fn outl(v: u32, port: u16);
}