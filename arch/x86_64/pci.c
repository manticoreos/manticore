#include <arch/ioport.h>
#include <stddef.h>
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc
#define PCI_CONFIG_ADDRESS_ENABLE 0x80000000UL

static uintptr_t pci_config_addr(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset)
{
	uintptr_t addr;
	addr = PCI_CONFIG_ADDRESS_ENABLE;
	addr |= ((uintptr_t)bus) << 16;
	addr |= ((uintptr_t)slot) << 11;
	addr |= ((uintptr_t)func) << 8;
	addr |= ((uintptr_t)offset) & 0xfc;
	return addr;
}

uint8_t pci_config_read_u8(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset)
{
	size_t addr = pci_config_addr(bus, slot, func, offset);
	outl(addr, PCI_CONFIG_ADDRESS);
	return inb(PCI_CONFIG_DATA + (offset & 3));
}

uint16_t pci_config_read_u16(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset)
{
	size_t addr = pci_config_addr(bus, slot, func, offset);
	outl(addr, PCI_CONFIG_ADDRESS);
	return inw(PCI_CONFIG_DATA + (offset & 2));
}

uint32_t pci_config_read_u32(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset)
{
	size_t addr = pci_config_addr(bus, slot, func, offset);
	outl(addr, PCI_CONFIG_ADDRESS);
	return inl(PCI_CONFIG_DATA);
}

void pci_config_write_u16(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset, uint16_t value)
{
	size_t addr = pci_config_addr(bus, slot, func, offset);
	outl(addr, PCI_CONFIG_ADDRESS);
	outw(value, PCI_CONFIG_DATA + (offset & 2));
}

void pci_config_write_u32(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset, uint32_t value)
{
	size_t addr = pci_config_addr(bus, slot, func, offset);
	outl(addr, PCI_CONFIG_ADDRESS);
	outl(value, PCI_CONFIG_DATA);
}
