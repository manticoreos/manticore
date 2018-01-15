pub type MMUMap = usize;

/// MMU page flags
pub const MMU_USER_PAGE: usize = 1 << 0;

extern "C" {
    pub fn mmu_map_range(map: MMUMap, vaddr: usize, paddr: usize, sz: usize, flags: usize) -> i32;
    pub fn mmu_invalidate_tlb();
    pub fn virt_to_phys(addr: usize) -> usize;
}
