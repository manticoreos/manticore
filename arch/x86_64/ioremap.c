#include <kernel/mmu.h>

#include <kernel/page-alloc.h>
#include <kernel/align.h>

#include <stddef.h>

///
/// Kernel virtual memory end address.
///
/// The ioremap() function uses this address to allocate virtual memory for I/O memory maps.
///
virt_t kernel_vm_end;

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
	virt_t ret = kernel_vm_end;
	kernel_vm_end += align_up(io_mem_size, PAGE_SIZE_SMALL);
	mmu_map_t map = mmu_current_map();
	int err = mmu_map_range(map, ret, io_mem_start, io_mem_size, MMU_PROT_READ | MMU_PROT_WRITE, MMU_NOCACHE);
	if (err) {
		return NULL;
	}
	return (void *)ret;
}
