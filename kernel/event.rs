//! Event subsystem for the kernel.
//!
//! This subsystem provides a mechanism for other kernel components to notify
//! user space processes of events. Event subscribers (user space process) and
//! event notifiers (kernel component such as a device driver) are decoupled
//! from each other via the `EVENTS` object. Subscribers call the `subscribe` to
//! express their interest in some events (user space processes call the
//! `subscribe` system call). Notifiers first register their subsystem with the
//! `register` function, and then notify of events using the `notify` function.

use alloc::rc::Rc;
use alloc::vec;
use alloc::vec::Vec;
use core::cell::RefCell;
use core::mem;
use errno::EINVAL;
use intrusive_collections::{KeyAdapter, RBTree, RBTreeLink};
use device::subscribe_device;
use vm::VMAddressSpace;

/// A kernel event.
#[derive(Clone, Debug)]
pub enum Event {
    PacketIO { addr: usize, len: usize },
}

/// A raw kernel event (needs to match definition in include/uapi/manticore/events.h).
#[repr(C)]
struct RawEvent {
    type_: usize,
    addr: usize,
    len: usize
}

const EVENT_PACKET_IO: usize = 0x01;

/// An event queue between kernel and user space.
#[derive(Debug)]
pub struct EventQueue {
    pub ring_buffer: usize,
}

impl EventQueue {
    pub fn new(buf: usize, size: usize) -> EventQueue {
        EventQueue {
            ring_buffer: unsafe { atomic_ring_buffer_new(buf, size) },
        }
    }

    pub fn emplace(&mut self, event: Event) {
        let raw_event = match event {
            Event::PacketIO { addr, len } => {
                RawEvent { type_: EVENT_PACKET_IO, addr: addr, len: len }
            }
        };
        unsafe { atomic_ring_buffer_emplace(self.ring_buffer, mem::transmute(&raw_event), mem::size_of::<RawEvent>()); }
    }
}

extern "C" {
    pub fn atomic_ring_buffer_new(buf: usize, size: usize) -> usize;
    pub fn atomic_ring_buffer_emplace(spc_queue: usize, event: usize, size: usize) -> usize;
}

/// An event listener.
pub trait EventListener {
    fn on_event(&self, ev: Event);
}

/// An event notifier.
pub struct EventNotifier {
    name: &'static str,
    listeners: RefCell<Vec<Rc<dyn EventListener>>>,
    link: RBTreeLink,
}

intrusive_adapter!(pub EventNotifierAdapter = Rc<EventNotifier>: EventNotifier { link: RBTreeLink });

impl<'a> KeyAdapter<'a> for EventNotifierAdapter {
    type Key = &'static str;
    fn get_key(&self, x: &'a EventNotifier) -> Self::Key {
        x.name
    }
}

impl EventNotifier {
    pub fn new(name: &'static str) -> Self {
        Self {
            name: name,
            listeners: RefCell::new(vec![]),
            link: RBTreeLink::new(),
        }
    }

    pub fn add_listener(&self, listener: Rc<dyn EventListener>) {
        self.listeners.borrow_mut().push(listener);
    }

    pub fn on_event(&self, ev: Event) {
        for ref mut listener in self.listeners.borrow().iter() {
            listener.on_event(ev.clone());
        }
    }
}

/// The kernel events subsystem.
pub struct Events {
    notifiers: RBTree<EventNotifierAdapter>,
}

impl Events {
    pub const fn new() -> Self {
        Events {
            notifiers: RBTree::new(EventNotifierAdapter::NEW),
        }
    }

    pub fn register(&mut self, notifier: Rc<EventNotifier>) {
        self.notifiers.insert(notifier);
    }

    pub fn subscribe(&mut self, name: &'static str, vmspace: &mut VMAddressSpace, listener: Rc<dyn EventListener>) -> i32 {
        subscribe_device(name, vmspace);
        let cursor = self.notifiers.find_mut(name);
        if let Some(notifier) = cursor.get() {
            notifier.add_listener(listener);
            return 0
        }
        return -EINVAL
    }
}

/// The `EVENTS` object is used by kernel components to register notifiers and
/// subscribe listeners.
pub static mut EVENTS: Events = Events::new();
