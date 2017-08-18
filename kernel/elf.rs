use core::intrinsics::transmute;
use xmas_elf::ElfFile;
use xmas_elf::program;
use rlibc::memcpy;
use core::slice;
use memory;

type MMUMap = usize;

const MMU_USER_PAGE : u64 = 1 << 0;

extern "C" {
    pub fn mmu_current_map() -> MMUMap;
    pub fn mmu_map_range(map: MMUMap, vaddr: u64, paddr: u64, sz: u64, flags: u64) -> i32;
    pub fn mmu_invalidate_tlb();
    pub fn virt_to_phys(addr: u64) -> u64;
}

/// Parse an ELF executable.
#[no_mangle]
pub extern "C" fn parse_elf(start: u64, end: u64) -> u64 {
    let size = (end - start) as usize;
    let ptr: *const u8 = unsafe { transmute(start) };
    let buf = unsafe { slice::from_raw_parts(ptr, size) };
    let elf_file = ElfFile::new(&buf);
    let entry_point = elf_file.header.pt2.unwrap().entry_point();
    let ph_iter = elf_file.program_iter();
    for phdr in ph_iter {
        match phdr.get_type().unwrap() {
            program::Type::Load => unsafe {
                let paddr = memory::page_alloc_small();
                memcpy(transmute(paddr),
                       transmute(start + phdr.offset()),
                       phdr.mem_size() as usize);
                let map = mmu_current_map();
                let err = mmu_map_range(map,
                                        phdr.virtual_addr(),
                                        virt_to_phys(transmute(paddr)),
                                        phdr.mem_size(),
                                        MMU_USER_PAGE);
                if err != 0 {
                    panic!("mmu_map_range failed: {}", err);
                }
                mmu_invalidate_tlb();
            },
            _ => {}
        }
    }
    entry_point
}
