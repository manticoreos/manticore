use alloc::rc::Rc;
use core::intrinsics::transmute;
use core::slice;
use core::cell::{Cell, RefCell};
use device::{NAMESPACE, Device, DeviceDesc, DeviceSpace};
use event::{Event, EventListener, EventQueue};
use errno::{Error, Result, EINVAL};
use intrusive_collections::LinkedListLink;
use memory;
use mmu;
use sched;
use vm::{VMAddressSpace, VMProt};
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

pub struct Process {
    pub state: RefCell<ProcessState>,
    pub task_state: TaskState,
    pub vmspace: RefCell<VMAddressSpace>,
    pub device_space: RefCell<DeviceSpace>,
    pub page_fault_fixup: Cell<u64>,
    pub event_queue: RefCell<EventQueue>,
    pub link: LinkedListLink,
}

intrusive_adapter!(pub ProcessAdapter = Rc<Process>: Process { link: LinkedListLink });

impl Process {
    pub fn new(task_state: TaskState, vmspace: VMAddressSpace, event_queue: EventQueue) -> Self {
        Process {
            state: RefCell::new(ProcessState::RUNNABLE),
            task_state: task_state,
            vmspace: RefCell::new(vmspace),
            device_space: RefCell::new(DeviceSpace::new()),
            page_fault_fixup: Cell::new(0),
            event_queue: RefCell::new(event_queue),
            link: LinkedListLink::new(),
        }
    }

    pub fn entry_point(&self) -> u64 {
        unsafe { task_state_entry_point(self.task_state) }
    }

    pub fn stack_top(&self) -> u64 {
        unsafe { task_state_stack_top(self.task_state) }
    }

    pub fn acquire(&self, name: &'static str) -> Result<(Rc<Device>, DeviceDesc)> {
        if let Some(device) = unsafe { NAMESPACE.lookup(name) } {
            let desc = self.device_space.borrow_mut().attach(device.clone());
            return Ok((device, desc));
        }
        return Err(Error::new(EINVAL));
    }
}

impl Drop for Process {
    fn drop(&mut self) {
        unsafe { task_state_delete(self.task_state); }
    }
}

impl EventListener for Process {
    fn on_event(&self, ev: Event) {
        self.event_queue.borrow_mut().emplace(ev);
    }
}

extern "C" {
    pub fn events_init(queue: usize);
    pub fn events_emplace(queue: usize, addr: usize, len: usize);
    pub fn task_state_new(entry_point: usize, stack_top: usize) -> TaskState;
    pub fn task_state_delete(task_state: TaskState);
    pub fn task_state_entry_point(task_state: TaskState) -> u64;
    pub fn task_state_stack_top(task_state: TaskState) -> u64;
}

/// Create a new process.
#[no_mangle]
pub unsafe extern "C" fn process_spawn(image_start: *const u8, image_size: usize) -> i32 {
    let mmu_map = mmu::mmu_current_map();
    let mut vmspace = VMAddressSpace::new(mmu_map);

    let entry_point = parse_elf_image(image_start, image_size, &mut vmspace);

    let stack_top = 0x40000000;
    let stack_size = memory::PAGE_SIZE_LARGE as usize;
    let stack_start = stack_top - stack_size;
    if let Err(e) = vmspace.allocate_fixed(stack_start, stack_top, VMProt::VM_PROT_RW) {
        return e.errno();
    }
    if let Err(e) = vmspace.populate(stack_start, stack_top) {
        return e.errno();
    }
    /* FIXME: Implement a virtual memory allocator insted of open-coding addresses here. */
    let event_buf_start = 0x80000000;
    let event_buf_size = 4096;
    let event_buf_end = event_buf_start + event_buf_size;
    if let Err(e) = vmspace.allocate_fixed(event_buf_start, event_buf_end, VMProt::VM_PROT_RW) {
        return e.errno();
    }
    if let Err(e) = vmspace.populate(event_buf_start, event_buf_end) {
        return e.errno();
    }
    let event_queue = EventQueue::new(event_buf_start, event_buf_size);

    let task_state = task_state_new(entry_point, stack_top);

    let process = Rc::new(Process::new(task_state, vmspace, event_queue));

    sched::enqueue(process);

    return 0;
}

fn parse_elf_image(image_start: *const u8, image_size: usize, vmspace: &mut VMAddressSpace) -> usize {
    let buf = unsafe { slice::from_raw_parts(image_start, image_size) };
    let elf_file = ElfFile::new(&buf).unwrap();
    let ph_iter = elf_file.program_iter();
    for phdr in ph_iter {
        match phdr.get_type().unwrap() {
            program::Type::Load => {
                let prot = elf_phdr_flags_to_prot(phdr.flags());
                let start = phdr.virtual_addr() as usize;
                let size = phdr.mem_size() as usize;
                let end = memory::align_up((start + size) as u64, memory::PAGE_SIZE_SMALL) as usize;
                vmspace.allocate_fixed(start, end, prot).expect("allocate failed");
                let image_start: u64 = unsafe { transmute(image_start) };
                let src_start: u64 = image_start + phdr.offset();
                let src_end = src_start + phdr.file_size();
                vmspace.populate_from(start, end, src_start as usize, src_end as usize).expect("populate_from failed");
            }
            _ => {}
        }
    }
    if let Some(section) = elf_file.find_section_by_name(".bss") {
        unsafe { memset(transmute(section.address()), 0, section.size() as usize) };
    }
    return elf_file.header.pt2.entry_point() as usize;
}

fn elf_phdr_flags_to_prot(flags: program::Flags) -> VMProt {
    let mut prot = VMProt::empty();
    if flags.is_read() {
        prot |= VMProt::VM_PROT_READ;
    }
    if flags.is_write() {
        prot |= VMProt::VM_PROT_WRITE;
    }
    if flags.is_execute() {
        prot |= VMProt::VM_PROT_EXEC;
    }
    return prot;
}
