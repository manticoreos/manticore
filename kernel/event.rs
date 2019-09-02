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
use errno::EINVAL;
use intrusive_collections::{KeyAdapter, RBTree, RBTreeLink};

/// A kernel event.
#[derive(Clone, Debug)]
pub enum Event {
    PacketIO { addr: usize, len: usize },
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
        for listener in self.listeners.borrow().iter() {
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

    pub fn subscribe(&mut self, name: &'static str, listener: Rc<dyn EventListener>) -> i32 {
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
