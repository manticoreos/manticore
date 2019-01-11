
use alloc::boxed::Box;
use core::intrinsics::transmute;
use core::slice;
use memory;
use mmu;
use vm::{VMAddressSpace, VMProt, VM_PROT_RW, VM_PROT_READ, VM_PROT_WRITE, VM_PROT_EXEC};
use xmas_elf::ElfFile;
use xmas_elf::program;

pub struct Process {
    pub vmspace: VMAddressSpace,
}

impl Process {
    pub fn new(mmu_map: mmu::MMUMap) -> Self {
        Process { vmspace: VMAddressSpace::new(mmu_map) }
    }
}

/// Create a new process.
#[no_mangle]
pub unsafe extern "C" fn process_run(image_start: *const u8, image_size: usize) {
    // FIXME: Allocate a new MMU translation table.
    let mmu_map = mmu::mmu_current_map();
    let mut process = Box::new(Process::new(mmu_map));

    let buf = slice::from_raw_parts(image_start, image_size);
    let elf_file = ElfFile::new(&buf).unwrap();
    let ph_iter = elf_file.program_iter();
    for phdr in ph_iter {
        match phdr.get_type().unwrap() {
            program::Type::Load => {
                let prot = elf_phdr_flags_to_prot(phdr.flags());
                let start = phdr.virtual_addr() as usize;
                let size = phdr.mem_size() as usize;
                let end = memory::align_up((start + size) as u64, memory::PAGE_SIZE_SMALL) as usize;
                process.vmspace.allocate(start, end, prot).expect(
                    "allocate failed",
                );
                let image_start: u64 = transmute(image_start);
                let src_start: u64 = image_start + phdr.offset();
                let src_end = src_start + phdr.file_size();
                process
                    .vmspace
                    .populate_from(start, end, src_start as usize, src_end as usize)
                    .expect("populate_from failed");
            }
            _ => {}
        }
    }

    let stack_top = 0x40000000;
    let stack_size = 4096;
    let stack_start = stack_top - stack_size;
    process
        .vmspace
        .allocate(stack_start, stack_top, VM_PROT_RW)
        .expect("allocate failed");
    process.vmspace.populate(stack_start, stack_top).expect(
        "populate failed",
    );

    let entry_point = elf_file.header.pt2.entry_point();
    switch_to_userspace(entry_point, stack_top as u64);
}

fn elf_phdr_flags_to_prot(flags: program::Flags) -> VMProt {
    let mut prot = VMProt::empty();
    if flags.is_read() {
        prot |= VM_PROT_READ;
    }
    if flags.is_write() {
        prot |= VM_PROT_WRITE;
    }
    if flags.is_execute() {
        prot |= VM_PROT_EXEC;
    }
    return prot;
}

extern "C" {
    pub fn switch_to_userspace(entry: u64, stack: u64);
}
