//! Virtual memory manager.

use alloc::boxed::Box;
use core::cell::Cell;
use core::mem;
use errno::{Error, Result, EINVAL, ENOMEM};
use intrusive_collections::{Bound, RBTree, RBTreeLink, KeyAdapter};
use memory;
use mmu;
use rlibc::memcpy;

/// VM region protection flags
bitflags! {
    pub struct VMProt: u32 {
        const VM_PROT_READ  = 0b00000001;
        const VM_PROT_WRITE = 0b00000010;
        const VM_PROT_EXEC  = 0b00000100;
        const VM_PROT_RX    = VM_PROT_READ.bits | VM_PROT_EXEC.bits;
        const VM_PROT_RW    = VM_PROT_READ.bits | VM_PROT_WRITE.bits;
        const VM_PROT_RWX   = VM_PROT_READ.bits | VM_PROT_WRITE.bits | VM_PROT_EXEC.bits;
    }
}

/// VM region
///
/// This type represents a contiguous memory region in a virtual address
/// space, which shares the same memory permissions.
#[derive(Debug)]
pub struct VMRegion {
    pub start: usize,
    pub end: usize,
    pub prot: VMProt,
    pub page: Cell<Option<usize>>,
    pub link: RBTreeLink,
}

intrusive_adapter!(pub VMRegionAdapter = Box<VMRegion>: VMRegion { link: RBTreeLink });

impl<'a> KeyAdapter<'a> for VMRegionAdapter {
    type Key = usize;
    fn get_key(&self, x: &'a VMRegion) -> usize {
        x.start
    }
}

impl Drop for VMRegion {
    fn drop(&mut self) {
        self.delete();
    }
}

impl VMRegion {
    pub fn new(start: usize, end: usize, prot: VMProt) -> Self {
        VMRegion {
            start: start,
            end: end,
            prot: prot,
            page: Cell::new(None),
            link: RBTreeLink::new(),
        }
    }

    pub fn delete(&self) {
        if let Some(page) = self.page.get() {
            memory::page_free_small(page as *mut u8);
            self.page.set(None);
        }
    }

    pub fn mmu_prot(&self) -> usize {
        let mut prot: usize = 0;
        if self.prot.contains(VM_PROT_READ) {
            prot |= mmu::MMU_PROT_READ;
        }
        if self.prot.contains(VM_PROT_WRITE) {
            prot |= mmu::MMU_PROT_WRITE;
        }
        if self.prot.contains(VM_PROT_EXEC) {
            prot |= mmu::MMU_PROT_EXEC;
        }
        return prot;
    }
}

/// VM address space
///
/// This type provides mapping for a virtual address space. It keeps
/// track of VM regions that are present in an address space and
/// allows the address space to be manipulated. The type also holds a
/// pointer to hardware MMU translation table, which is kept up-to-date
/// with the hardware-independent virtual address space.
#[derive(Debug)]
pub struct VMAddressSpace {
    pub vm_regions: RBTree<VMRegionAdapter>,
    pub mmu_map: mmu::MMUMap,
}

impl Drop for VMAddressSpace {
    fn drop(&mut self) {
        self.delete();
    }
}

impl VMAddressSpace {
    pub fn new(mmu_map: usize) -> Self {
        VMAddressSpace {
            vm_regions: RBTree::new(VMRegionAdapter::new()),
            mmu_map: mmu_map,
        }
    }

    pub fn delete(&mut self) {
        self.vm_regions.clear();
    }

    pub fn allocate(&mut self, start: usize, end: usize, prot: VMProt) -> Result<()> {
        let size = end - start;
        if size != memory::PAGE_SIZE_SMALL as usize {
            return Err(Error::new(EINVAL));
        }
        {
            let iter = self.vm_regions.range(
                Bound::Included(&start),
                Bound::Excluded(&end),
            );
            if iter.count() > 0 {
                return Err(Error::new(EINVAL));
            }
        }
        self.vm_regions.insert(
            Box::new(VMRegion::new(start, end, prot)),
        );
        Ok(())
    }

    pub fn deallocate(&mut self, start: usize, end: usize) -> Result<()> {
        let mut cur = self.vm_regions.find_mut(&start);
        if let Some(region) = cur.get() {
            if region.end != end {
                return Err(Error::new(EINVAL));
            }
            cur.remove();
            Ok(())
        } else {
            return Err(Error::new(EINVAL));
        }
    }

    pub fn populate(&mut self, start: usize, end: usize) -> Result<()> {
        let cur = self.vm_regions.find(&start);
        if let Some(region) = cur.get() {
            if region.end != end {
                return Err(Error::new(EINVAL));
            }
            let err = unsafe {
                let page = memory::page_alloc_small();
                if page as usize == 0 {
                    return Err(Error::new(ENOMEM));
                }
                region.page.set(Some(page as usize));
                mmu::mmu_map_range(
                    self.mmu_map,
                    start,
                    mmu::virt_to_phys(mem::transmute(page)),
                    end - start,
                    region.mmu_prot(),
                    mmu::MMU_USER_PAGE,
                )
            };
            if err != 0 {
                return Err(Error::new(err));
            }
            return Ok(());
        } else {
            return Err(Error::new(EINVAL));
        }
    }

    pub fn populate_from(
        &mut self,
        start: usize,
        end: usize,
        src_start: usize,
        src_end: usize,
    ) -> Result<()> {
        let size = end - start;
        let src_size = src_end - src_start;
        if size < src_size {
            return Err(Error::new(EINVAL));
        }
        let cur = self.vm_regions.find(&start);
        if let Some(region) = cur.get() {
            if region.end != end {
                return Err(Error::new(EINVAL));
            }
            let err = unsafe {
                let page = memory::page_alloc_small();
                if page as usize == 0 {
                    return Err(Error::new(ENOMEM));
                }
                region.page.set(Some(page as usize));
                memcpy(mem::transmute(page), mem::transmute(src_start), size);
                mmu::mmu_map_range(
                    self.mmu_map,
                    start,
                    mmu::virt_to_phys(mem::transmute(page)),
                    size,
                    region.mmu_prot(),
                    mmu::MMU_USER_PAGE,
                )
            };
            if err != 0 {
                return Err(Error::new(err));
            }
            return Ok(());
        } else {
            return Err(Error::new(EINVAL));
        }
    }
}
