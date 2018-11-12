//! PCI I/O port accessors.

use core::intrinsics::transmute;

/// I/O port that is either MMIO or PIO.
pub enum IOPort {
    Memory { base_addr: usize, size: u32 },
    IO { iobase: u16, size: u32 },
}

impl IOPort {
    pub unsafe fn read8(&self, offset: usize) -> u8 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u8 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { iobase, .. } => {
                return pio_read8(iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn write8(&self, value: u8, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u8 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { iobase, .. } => {
                pio_write8(value, iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn read16(&self, offset: usize) -> u16 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u16 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { iobase, .. } => {
                return pio_read16(iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn write16(&self, value: u16, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u16 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { iobase, .. } => {
                pio_write16(value, iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn read32(&self, offset: usize) -> u32 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u32 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { iobase, .. } => {
                return pio_read32(iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn write32(&self, value: u32, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u32 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { iobase, .. } => {
                pio_write32(value, iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn read64(&self, offset: usize) -> u64 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u64 = transmute(base_addr + offset);
                return *addr;
            }
            &IOPort::IO { .. } => {
                unimplemented!();
            }
        }
    }
    pub unsafe fn write64(&self, value: u64, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                let mut addr: *mut u64 = transmute(base_addr + offset);
                *addr = value;
            }
            &IOPort::IO { .. } => {
                unimplemented!();
            }
        }
    }
}

extern "C" {
    fn pio_read8(port: u16) -> u8;
    fn pio_read16(port: u16) -> u16;
    fn pio_read32(port: u16) -> u32;
    fn pio_write8(v: u8, port: u16);
    fn pio_write16(v: u16, port: u16);
    fn pio_write32(v: u32, port: u16);
}
