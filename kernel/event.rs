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
use intrusive_collections::{KeyAdapter, RBTreeLink};
use atomic_ring_buffer::AtomicRingBuffer;

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

const EVENT_PACKET_RX: usize = 0x01;

/// An event queue between kernel and user space.
#[derive(Debug)]
pub struct EventQueue {
    pub ring_buffer: AtomicRingBuffer,
}

impl EventQueue {
    pub fn new(buf: usize, size: usize) -> EventQueue {
        EventQueue {
            ring_buffer: AtomicRingBuffer::new::<RawEvent>(buf, size),
        }
    }

    pub fn emplace(&mut self, event: Event) {
        let raw_event = match event {
            Event::PacketIO { addr, len } => {
                RawEvent { type_: EVENT_PACKET_RX, addr, len }
            }
        };
        self.ring_buffer.emplace(&raw_event);
    }
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
            name,
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
