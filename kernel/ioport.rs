//! PCI I/O port accessors.

/// I/O port that is either MMIO or PIO.
#[derive(Clone, Copy, Debug)]
pub enum IOPort {
    Memory { base_addr: usize, size: u32 },
    IO { iobase: u16, size: u32 },
}

impl IOPort {
    /// Remaps this I/O port to a subset of the I/O port.
    ///
    /// The function returns an `Option<IOPort>` where the the `IOPort` has its base address
    /// shifted by `offset` and size of `new_size`. If the requested remapped I/O port region does
    /// not fit the original I/O port, `None` is returned.
    pub fn remap(&self, offset: usize, new_size: u32) -> Option<IOPort> {
        match self {
            &IOPort::Memory { base_addr, size } => {
                if offset > (base_addr + size as usize) as usize {
                    return None
                }
                Some(IOPort::Memory { base_addr: base_addr + offset, size: new_size })
            },
            &IOPort::IO { iobase, size } => {
                if offset > (iobase as u32 + size) as usize {
                    return None
                }
                Some(IOPort::IO { iobase: iobase + offset as u16, size: new_size })
            }
        }
    }

    pub fn read8(&self, offset: usize) -> u8 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return unsafe { mmio_read8((base_addr + offset) as u64) };
            }
            &IOPort::IO { iobase, .. } => {
                return unsafe { pio_read8(iobase + (offset as u16)) };
            }
        }
    }
    pub fn write8(&self, value: u8, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                unsafe { mmio_write8(value, (base_addr + offset) as u64) };
            }
            &IOPort::IO { iobase, .. } => {
                unsafe { pio_write8(value, iobase + (offset as u16)) };
            }
        }
    }
    pub fn read16(&self, offset: usize) -> u16 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return unsafe { mmio_read16((base_addr + offset) as u64) };
            }
            &IOPort::IO { iobase, .. } => {
                return unsafe { pio_read16(iobase + (offset as u16)) };
            }
        }
    }
    pub fn write16(&self, value: u16, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                unsafe { mmio_write16(value, (base_addr + offset) as u64) };
            }
            &IOPort::IO { iobase, .. } => {
                unsafe { pio_write16(value, iobase + (offset as u16)) };
            }
        }
    }
    pub fn read32(&self, offset: usize) -> u32 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return unsafe { mmio_read32((base_addr + offset) as u64) };
            }
            &IOPort::IO { iobase, .. } => {
                return unsafe { pio_read32(iobase + (offset as u16)) };
            }
        }
    }
    pub fn write32(&self, value: u32, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                unsafe { mmio_write32(value, (base_addr + offset) as u64) };
            }
            &IOPort::IO { iobase, .. } => {
                unsafe { pio_write32(value, iobase + (offset as u16)) };
            }
        }
    }
    pub fn read64(&self, offset: usize) -> u64 {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                return unsafe { mmio_read64((base_addr + offset) as u64) };
            }
            &IOPort::IO { .. } => {
                unimplemented!();
            }
        }
    }
    pub fn write64(&self, value: u64, offset: usize) {
        match self {
            &IOPort::Memory { base_addr, .. } => {
                unsafe { mmio_write64(value, (base_addr + offset) as u64); }
            }
            &IOPort::IO { .. } => {
                unimplemented!();
            }
        }
    }
}

extern "C" {
    pub fn pio_read8(port: u16) -> u8;
    pub fn pio_read16(port: u16) -> u16;
    pub fn pio_read32(port: u16) -> u32;
    pub fn pio_write8(v: u8, port: u16);
    pub fn pio_write16(v: u16, port: u16);
    pub fn pio_write32(v: u32, port: u16);
    pub fn mmio_read8(addr: u64) -> u8;
    pub fn mmio_read16(addr: u64) -> u16;
    pub fn mmio_read32(addr: u64) -> u32;
    pub fn mmio_read64(addr: u64) -> u64;
    pub fn mmio_write8(v: u8, addr: u64);
    pub fn mmio_write16(v: u16, addr: u64);
    pub fn mmio_write32(v: u32, addr: u64);
    pub fn mmio_write64(v: u64, addr: u64);
}
