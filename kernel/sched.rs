//! Process scheduler.
//!
//! The `sched` module contains a round-robin process scheduler.

use alloc::rc::Rc;
use core::cmp;
use device::DeviceDesc;
use errno::EINVAL;
use intrusive_collections::LinkedList;
use null_terminated::NulStr;
use process::{Process, ProcessAdapter, ProcessState, TaskState};
use vm::VMProt;
use user_access;
use device;

/// Current running process.
static mut CURRENT: Option<Rc<Process>> = None;

fn set_current(proc: Rc<Process>) {
    unsafe { CURRENT = Some(proc) };
}

fn get_current() -> Rc<Process> {
    unsafe {
        if let Some(ref current) = CURRENT {
            current.clone()
        } else {
            panic!("No current process");
        }
    }
}

fn take_current() -> Option<Rc<Process>> {
    unsafe { CURRENT.take() }
}

/// Queue of runnable processes.
static mut RUNQUEUE: LinkedList<ProcessAdapter> = LinkedList::new(ProcessAdapter::NEW);

/// Queue of waiting processes.
static mut WAITQUEUE: LinkedList<ProcessAdapter> = LinkedList::new(ProcessAdapter::NEW);

pub fn enqueue(proc: Rc<Process>) {
    proc.state.replace(ProcessState::RUNNABLE);
    unsafe {
        RUNQUEUE.push_back(proc);
    }
}

fn dequeue() -> Option<Rc<Process>> {
    unsafe { RUNQUEUE.pop_front() }
}

/// Schedule processes.
#[no_mangle]
pub extern "C" fn schedule() {
    let prev = take_current().map(|prev| {
        match *prev.state.borrow() {
            ProcessState::WAITING => unsafe {
                WAITQUEUE.push_back(prev.clone());
            },
            _ => {
                enqueue(prev.clone());
            }
        }
        prev
    });
    if let Some(next) = dequeue() {
        // FIXME: make prev RUNNABLE and next RUNNING
        set_current(next.clone());
        next.state.replace(ProcessState::RUNNING);
        let next_ts = next.task_state;
        if let Some(prev) = prev {
            let prev_ts = prev.task_state;
            if prev_ts != next_ts {
                next.vmspace.borrow().switch_to();
                unsafe { switch_to(prev_ts, next_ts) };
            }
        } else {
            next.vmspace.borrow().switch_to();
            unsafe { switch_to_first(next_ts) };
        }
    } else if let Some(prev) = prev {
        unsafe {
            switch_to(prev.task_state, idle_task);
        }
    } else {
        unsafe {
            switch_to_first(idle_task);
        }
    }
}

extern "C" {
    pub fn switch_to(old: TaskState, new: TaskState);
    pub fn switch_to_first(ts: TaskState);
    pub static idle_task: TaskState;
}

#[no_mangle]
pub extern "C" fn process_acquire(name: &'static NulStr) -> i32 {
    let current = get_current();
    match current.acquire(&name[..]) {
        Ok((device, desc)) => {
            if let Err(e) = device.acquire(&mut current.vmspace.borrow_mut(), current.clone()) {
                return e.errno();
            }
            desc.to_user()
        }
        Err(e) => e.errno(),
    }
}

#[no_mangle]
pub extern "C" fn process_subscribe(raw_desc: i32, events: &'static NulStr) -> i32 {
    let current = get_current();
    let desc = DeviceDesc::from_user(raw_desc);
    if let Some(device) = current.device_space.borrow().lookup(desc) {
        device.subscribe(&events[..]);
    }
    -EINVAL
}

#[no_mangle]
pub unsafe extern "C" fn process_get_config(raw_desc: i32, opt: i32, buf: *mut u8, len: usize) -> i32 {
    let current = get_current();
    let desc = DeviceDesc::from_user(raw_desc);
    if let Some(device) = current.device_space.borrow().lookup(desc) {
        if let Some(value) = device.get_config(opt) {
            let to_copy = cmp::min(len, value.len());
            user_access::memcpy_to_user(buf, value.as_ptr(), to_copy);
            return 0;
        }
    }
    -EINVAL
}

/// Make the current process wait for an event.
#[no_mangle]
pub extern "C" fn process_wait() {
    let current = get_current();
    current.state.replace(ProcessState::WAITING);
    device::process_io();
    schedule();
}

#[no_mangle]
pub extern "C" fn process_getevents() -> usize {
    let current = get_current();
    return current.event_queue.borrow().ring_buffer.raw_ptr();
}

#[no_mangle]
pub extern "C" fn process_vmspace_alloc(size: u64, vmr_start: *mut u64) -> i32 {
    let current = get_current();
    return match current.vmspace.borrow_mut().allocate(size as usize, VMProt::VM_PROT_RW) {
        Ok((start, _)) => {
            unsafe { *vmr_start = start as u64 };
            0
        },
        Err(e) => e.errno(),
    };
}

#[no_mangle]
pub extern fn page_fault_set_fixup(fixup: u64)
{
    unsafe {
        if let Some(ref mut current) = CURRENT {
            current.page_fault_fixup.replace(fixup);
        }
    }
}

#[no_mangle]
pub extern fn page_fault_get_fixup() -> u64
{
    unsafe {
        if let Some(ref mut current) = CURRENT {
            current.page_fault_fixup.get()
        } else {
            0
        }
    }
}

#[no_mangle]
pub extern "C" fn wake_up_processes() {
    loop {
        unsafe {
            if let Some(proc) = WAITQUEUE.pop_front() {
                enqueue(proc);
            } else {
                break;
            }
        }
    }
}
