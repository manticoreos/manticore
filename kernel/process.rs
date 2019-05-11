use alloc::rc::Rc;
use core::intrinsics::transmute;
use core::slice;
use core::cell::RefCell;
use intrusive_collections::LinkedListLink;
use memory;
use mmu;
use sched;
use vm::{VMAddressSpace, VMProt, VM_PROT_EXEC, VM_PROT_READ, VM_PROT_RW, VM_PROT_WRITE};
use xmas_elf::program;
use xmas_elf::ElfFile;
use rlibc::memset;

pub type TaskState = usize;

#[derive(Debug)]
pub enum ProcessState {
    RUNNABLE,
    RUNNING,
    WAITING,
}

#[derive(Debug)]
pub struct Process {
    pub state: RefCell<ProcessState>,
    pub task_state: TaskState,
    pub vmspace: VMAddressSpace,
    pub link: LinkedListLink,
}

intrusive_adapter!(pub ProcessAdapter = Rc<Process>: Process { link: LinkedListLink });

impl Process {
    pub fn new(task_state: TaskState, vmspace: VMAddressSpace) -> Self {
        Process {
            state: RefCell::new(ProcessState::RUNNABLE),
            task_state: task_state,
            vmspace: vmspace,
            link: LinkedListLink::new(),
        }
    }

    pub fn entry_point(&self) -> u64 {
        unsafe { task_state_entry_point(self.task_state) }
    }

    pub fn stack_top(&self) -> u64 {
        unsafe { task_state_stack_top(self.task_state) }
    }
}

impl Drop for Process {
    fn drop(&mut self) {
        unsafe { task_state_delete(self.task_state); }
    }
}

extern "C" {
    pub fn task_state_new(entry_point: usize, stack_top: usize) -> TaskState;
    pub fn task_state_delete(task_state: TaskState);
    pub fn task_state_entry_point(task_state: TaskState) -> u64;
    pub fn task_state_stack_top(task_state: TaskState) -> u64;
}

/// Create a new process.
#[no_mangle]
pub unsafe extern "C" fn process_run(image_start: *const u8, image_size: usize) {
    let mmu_map = mmu::mmu_current_map();
    let mut vmspace = VMAddressSpace::new(mmu_map);

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
                vmspace.allocate(start, end, prot).expect("allocate failed");
                let image_start: u64 = transmute(image_start);
                let src_start: u64 = image_start + phdr.offset();
                let src_end = src_start + phdr.file_size();
                vmspace.populate_from(start, end, src_start as usize, src_end as usize).expect("populate_from failed");
            }
            _ => {}
        }
    }
    if let Some(section) = elf_file.find_section_by_name(".bss") {
        memset(transmute(section.address()), 0, section.size() as usize);
    }
    let stack_top = 0x40000000;
    let stack_size = 4096;
    let stack_start = stack_top - stack_size;
    vmspace.allocate(stack_start, stack_top, VM_PROT_RW).expect("allocate failed");
    vmspace.populate(stack_start, stack_top).expect("populate failed");
    let task_state = task_state_new(elf_file.header.pt2.entry_point() as usize, stack_top);
    let process = Rc::new(Process::new(task_state, vmspace));
    sched::enqueue(process);
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
