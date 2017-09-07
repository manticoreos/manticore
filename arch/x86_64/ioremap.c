#include <kernel/mmu.h>

#include <stddef.h>

///
/// Map an I/O memory region to kernel virtual address space.
///
/// This function maps an I/O memory region to virtual address space so that the I/O memory region can be accessed by
/// the kernel.
///
/// \param paddr I/O memory region start address.
/// \param size I/O memory region size.
/// \return I/O memory region start address in virtual address space.
///
void *ioremap(phys_t io_mem_start, size_t io_mem_size)
{
	virt_t ret = io_mem_start;
	mmu_map_t map = mmu_current_map();
	int err = mmu_map_range(map, ret, io_mem_start, io_mem_size, 0);
	if (err) {
		return NULL;
	}
	return (void *)ret;
}
