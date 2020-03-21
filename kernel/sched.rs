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
use user_access;

/// Current running process.
static mut CURRENT: Option<Rc<Process>> = None;

fn set_current(proc: Rc<Process>) {
    unsafe { CURRENT = Some(proc) };
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
    } else {
        if let Some(prev) = prev {
            unsafe {
                switch_to(prev.task_state, idle_task);
            }
        } else {
            unsafe {
                switch_to_first(idle_task);
            }
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
    unsafe {
        if let Some(ref current) = CURRENT {
            return match current.acquire(&name[..]) {
                Ok((device, desc)) => {
                    device.acquire(&mut current.vmspace.borrow_mut(), current.clone());
                    desc.to_user()
                },
                Err(e) => e.errno(),
            };
        } else {
            panic!("No current process");
        }
    }
}

#[no_mangle]
pub extern "C" fn process_subscribe(raw_desc: i32, events: &'static NulStr) -> i32 {
    unsafe {
        if let Some(ref current) = CURRENT {
            let desc = DeviceDesc::from_user(raw_desc);
            if let Some(device) = current.device_space.borrow().lookup(desc) {
                device.subscribe(&events[..]);
            }
            return -EINVAL;
        } else {
            panic!("No current process");
        }
    }
}

#[no_mangle]
pub extern "C" fn process_get_config(raw_desc: i32, opt: i32, buf: *mut u8, len: usize) -> i32 {
    unsafe {
        if let Some(ref current) = CURRENT {
            let desc = DeviceDesc::from_user(raw_desc);
            if let Some(device) = current.device_space.borrow().lookup(desc) {
                if let Some(value) = device.get_config(opt) {
                    let to_copy = cmp::min(len, value.len());
                    user_access::memcpy_to_user(buf, value.as_ptr(), to_copy);
                    return 0
                }
            }
            return -EINVAL;
        } else {
            panic!("No current process");
        }
    }
}

/// Make the current process wait for an event.
#[no_mangle]
pub extern "C" fn process_wait() {
    unsafe {
        if let Some(ref mut current) = CURRENT {
            current.state.replace(ProcessState::WAITING);
            schedule();
        } else {
            panic!("No current process");
        }
    }
}

#[no_mangle]
pub extern "C" fn process_getevents() -> usize {
    unsafe {
        if let Some(ref mut current) = CURRENT {
            return current.event_queue.borrow().ring_buffer.raw_ptr();
        } else {
            panic!("No current process");
        }
    }
}

#[no_mangle]
pub extern "C" fn process_get_io_queue() -> usize {
    unsafe {
        if let Some(ref mut current) = CURRENT {
            return current.io_queue.borrow().ring_buffer.raw_ptr();
        } else {
            panic!("No current process");
        }
    }
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
            return current.page_fault_fixup.get()
        } else {
            return 0
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
