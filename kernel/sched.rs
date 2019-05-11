//! Process scheduler.
//!
//! The `sched` module contains a round-robin process scheduler.

use alloc::rc::Rc;
use intrusive_collections::LinkedList;
use process::{Process, ProcessAdapter, ProcessState, TaskState};

/// Current running process.
static mut CURRENT: Option<Rc<Process>> = None;

fn set_current(proc: Rc<Process>) {
    unsafe { CURRENT = Some(proc) };
}

fn take_current() -> Option<Rc<Process>> {
    unsafe { CURRENT.take() }
}

/// Queue of runnable processes.
static mut RUNQUEUE: LinkedList<ProcessAdapter> = LinkedList::new(ProcessAdapter::new());

/// Queue of waiting processes.
static mut WAITQUEUE: LinkedList<ProcessAdapter> = LinkedList::new(ProcessAdapter::new());

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
                next.vmspace.switch_to();
                unsafe { switch_to(prev_ts, next_ts) };
            }
        } else {
            next.vmspace.switch_to();
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
