pub type MMUMap = usize;

pub const MMU_PROT_READ: usize = 1 << 0;
pub const MMU_PROT_WRITE: usize = 1 << 1;
pub const MMU_PROT_EXEC: usize = 1 << 2;

pub const MMU_USER_PAGE: usize = 1 << 0;

extern "C" {
    pub fn mmu_current_map() -> MMUMap;
    pub fn mmu_load_map(map: MMUMap);
    pub fn mmu_map_range(map: MMUMap, vaddr: usize, paddr: usize, sz: usize, prot: usize, flags: usize) -> i32;
    pub fn mmu_invalidate_tlb();
    pub fn virt_to_phys(addr: usize) -> usize;
    pub fn phys_to_virt(addr: usize) -> usize;
}
