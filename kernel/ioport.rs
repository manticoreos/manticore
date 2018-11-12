//! PCI I/O port accessors.

/// I/O port that is either MMIO or PIO.
pub enum IOPort {
    Memory { base_addr: usize, size: u32 },
    IO { iobase: u16, size: u32 },
}

impl IOPort {
    pub unsafe fn read8(&self, offset: usize) -> u8 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return mmio_read8((base_addr + offset) as u64);
            }
            &IOPort::IO { iobase, .. } => {
                return pio_read8(iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn write8(&self, value: u8, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                mmio_write8(value, (base_addr + offset) as u64);
            }
            &IOPort::IO { iobase, .. } => {
                pio_write8(value, iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn read16(&self, offset: usize) -> u16 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return mmio_read16((base_addr + offset) as u64);
            }
            &IOPort::IO { iobase, .. } => {
                return pio_read16(iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn write16(&self, value: u16, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                mmio_write16(value, (base_addr + offset) as u64);
            }
            &IOPort::IO { iobase, .. } => {
                pio_write16(value, iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn read32(&self, offset: usize) -> u32 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return mmio_read32((base_addr + offset) as u64);
            }
            &IOPort::IO { iobase, .. } => {
                return pio_read32(iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn write32(&self, value: u32, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                mmio_write32(value, (base_addr + offset) as u64);
            }
            &IOPort::IO { iobase, .. } => {
                pio_write32(value, iobase + (offset as u16));
            }
        }
    }
    pub unsafe fn read64(&self, offset: usize) -> u64 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return mmio_read64((base_addr + offset) as u64);
            }
            &IOPort::IO { .. } => {
                unimplemented!();
            }
        }
    }
    pub unsafe fn write64(&self, value: u64, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                mmio_write64(value, (base_addr + offset) as u64);
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
    fn mmio_read8(addr: u64) -> u8;
    fn mmio_read16(addr: u64) -> u16;
    fn mmio_read32(addr: u64) -> u32;
    fn mmio_read64(addr: u64) -> u64;
    fn mmio_write8(v: u8, addr: u64);
    fn mmio_write16(v: u16, addr: u64);
    fn mmio_write32(v: u32, addr: u64);
    fn mmio_write64(v: u64, addr: u64);
}
