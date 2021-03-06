//! Virtio network device driver.

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;
use core::mem;
use kernel::errno::Result;
use kernel::device::{ConfigOption, Device, DeviceOps, CONFIG_ETHERNET_MAC_ADDRESS, CONFIG_IO_QUEUE};
use kernel::event::{Event, EventListener, EventNotifier};
use kernel::ioport::IOPort;
use kernel::ioqueue::{IOCmd, Opcode, IOQueue};
use kernel::memory;
use kernel::mmu;
use kernel::print;
use kernel::vm::{VMAddressSpace, VMProt};
use pci::{DeviceID, PCIDevice, PCIDriver, PCI_CAPABILITY_VENDOR, PCI_VENDOR_ID_REDHAT};
use virtqueue::{Virtqueue};

const PCI_DEVICE_ID_VIRTIO_NET: u16 = 0x1041;

#[allow(dead_code)]
const DEVICE_FEATURE_SELECT: usize = 0x00;
const DEVICE_FEATURE: usize = 0x04;
#[allow(dead_code)]
const DRIVER_FEATURE_SELECT: usize = 0x08;
const DRIVER_FEATURE: usize = 0x0c;
#[allow(dead_code)]
const MSIX_CONFIG: usize = 0x10;
const NUM_QUEUES: usize = 0x12;
const DEVICE_STATUS: usize = 0x14;
#[allow(dead_code)]
const CONFIG_GENERATION: usize = 0x15;
const QUEUE_SELECT: usize = 0x16;
const QUEUE_SIZE: usize = 0x18;
const QUEUE_MSIX_VECTOR: usize = 0x1a;
const QUEUE_ENABLE: usize = 0x1c;
const QUEUE_NOTIFY_OFF: usize = 0x1e;
const QUEUE_DESC: usize = 0x20;
const QUEUE_AVAIL: usize = 0x28;
const QUEUE_USED: usize = 0x30;

const VIRTIO_ACKNOWLEDGE: u8 = 1;
const VIRTIO_DRIVER: u8 = 2;
#[allow(dead_code)]
const VIRTIO_FAILED: u8 = 128;
const VIRTIO_FEATURES_OK: u8 = 8;
const VIRTIO_DRIVER_OK: u8 = 4;
#[allow(dead_code)]
const DEVICE_NEEDS_RESET: u8 = 64;

bitflags! {
    pub struct Features: u32 {
        const VIRTIO_NET_F_CSUM = 1<<0;
        const VIRTIO_NET_F_GUEST_CSUM = 1<<1;
        const VIRTIO_NET_F_CTRL_GUEST_OFFLOADS = 1<<2;
        const VIRTIO_NET_F_MAC = 1<<5;
        const VIRTIO_NET_F_GUEST_TSO4 = 1<<7;
        const VIRTIO_NET_F_GUEST_TSO6 = 1<<8;
        const VIRTIO_NET_F_GUEST_ECN = 1<<9;
        const VIRTIO_NET_F_GUEST_UFO = 1<<10;
        const VIRTIO_NET_F_HOST_TSO4 = 1<<11;
        const VIRTIO_NET_F_HOST_TSO6 = 1<<12;
        const VIRTIO_NET_F_HOST_ECN = 1<<13;
        const VIRTIO_NET_F_HOST_UFO = 1<<14;
        const VIRTIO_NET_F_MRG_RXBUF = 1<<15;
        const VIRTIO_NET_F_STATUS = 1<<16;
        const VIRTIO_NET_F_CTRL_VQ = 1<<17;
        const VIRTIO_NET_F_CTRL_RX = 1<<18;
        const VIRTIO_NET_F_CTRL_VLAN = 1<<19;
        const VIRTIO_NET_F_GUEST_ANNOUNCE = 1<<21;
        const VIRTIO_NET_F_MQ = 1<<22;
        const VIRTIO_NET_F_CTRL_MAC_ADDR = 1<<23;
    }
}

const VIRTIO_PCI_CAP_COMMON_CFG: u8 = 1;
const VIRTIO_PCI_CAP_NOTIFY_CFG: u8 = 2;
#[allow(dead_code)]
const VIRTIO_PCI_CAP_ISR_CFG: u8 = 3;
const VIRTIO_PCI_CAP_DEVICE_CFG: u8 = 4;
#[allow(dead_code)]
const VIRTIO_PCI_CAP_PCI_CFG: u8 = 5;

/* 4.1.4 Virtio Structure PCI Capabilities */
const VIRTIO_PCI_CAP_CFG_TYPE: u8 = 3;
const VIRTIO_PCI_CAP_BAR: u8 = 4;
const VIRTIO_PCI_CAP_OFFSET: u8 = 8;
const VIRTIO_PCI_CAP_LENGTH: u8 = 12;

/* 4.1.4.4 Notification structure layout */
const VIRTIO_NOTIFY_OFF_MULTIPLIER: u8 = 16;

const VIRTIO_RX_QUEUE_IDX: u16 = 0;
const VIRTIO_TX_QUEUE_IDX: u16 = 1;

type MacAddr = [u8; 6];

#[repr(C)]
#[derive(Debug)]
struct VirtioNetHdr {
    flags: u8,
    gso_type: u8,
    hdr_len: u16,
    gso_size: u16,
    csum_start: u16,
    csum_offset: u16,
}

struct VirtioNetDevice {
    pci_dev: Rc<PCIDevice>,
    notify_cfg_ioport: IOPort,
    notify_off_multiplier: u32,
    vqs: RefCell<Vec<Virtqueue>>,
    notifier: Rc<EventNotifier>,
    rx_page: usize,
    rx_page_size: usize,
    tx_page: usize,
    tx_page_size: usize,
    mac_addr: RefCell<Option<MacAddr>>,
    rx_buffer_addr: RefCell<Option<usize>>,
    io_queue: RefCell<Option<IOQueue>>,
}

/// Virtio PCI capability structure.
struct VirtioPCICap {
    pub offset: u8,
    pub bar_idx: u8,
}

impl VirtioPCICap {
    fn new(offset: u8, bar_idx: u8) -> Self {
      Self {
          offset,
          bar_idx,
      }
    }

    fn map(&self, pci_dev: &PCIDevice) -> Option<IOPort> {
        let offset = pci_dev.func.read_config_u32(self.offset + VIRTIO_PCI_CAP_OFFSET);
        let length = pci_dev.func.read_config_u32(self.offset + VIRTIO_PCI_CAP_LENGTH);
        pci_dev.bars[self.bar_idx as usize].map(|bar| { bar.remap(offset as usize, length) }).flatten()
    }
}

/// The name of this virtio device that is exported to user space.
const VIRTIO_DEV_NAME: &str = "/dev/eth";

impl VirtioNetDevice {
    fn probe(pci_dev: Rc<PCIDevice>) -> Option<Rc<Device>> {
        pci_dev.set_bus_master(true);

        pci_dev.enable_msix();

        let notify_cfg_cap = VirtioNetDevice::find_capability(&pci_dev, VIRTIO_PCI_CAP_NOTIFY_CFG)?;

        let notify_off_multiplier = pci_dev.func.read_config_u32(notify_cfg_cap.offset + VIRTIO_NOTIFY_OFF_MULTIPLIER);

        let notify_cfg_ioport = notify_cfg_cap.map(&pci_dev)?;

        let dev = Rc::new(VirtioNetDevice::new(pci_dev, notify_cfg_ioport, notify_off_multiplier));

        let common_cfg_cap = VirtioNetDevice::find_capability(&dev.pci_dev, VIRTIO_PCI_CAP_COMMON_CFG)?;

        let ioport = common_cfg_cap.map(&dev.pci_dev)?;

        println!("virtio-net: using PCI BAR{} for device configuration", common_cfg_cap.bar_idx);

        //
        // 1. Reset device
        //
        let mut status: u8 = 0;
        ioport.write8(status, DEVICE_STATUS);

        //
        // 2. Set ACKNOWLEDGE status bit
        //
        status |= VIRTIO_ACKNOWLEDGE;
        ioport.write8(status, DEVICE_STATUS);

        //
        // 3. Set DRIVER status bit
        //
        status |= VIRTIO_DRIVER;
        ioport.write8(status, DEVICE_STATUS);

        //
        // 4. Negotiate features
        //
        let features = Features::VIRTIO_NET_F_MAC;
        ioport.write32(features.bits(), DRIVER_FEATURE);

        //
        // 5. Set the FEATURES_OK status bit
        //
        status |= VIRTIO_FEATURES_OK;
        ioport.write8(status, DEVICE_STATUS);

        //
        // 6. Re-read device status to ensure the FEATURES_OK bit is still set
        //
        if ioport.read8(DEVICE_STATUS) & VIRTIO_FEATURES_OK != VIRTIO_FEATURES_OK {
            panic!("Device does not support our subset of features");
        }

        //
        // 7. Perform device-specific setup
        //
        let num_queues = ioport.read16(NUM_QUEUES);

        println!("virtio-net: {} virtqueues found.", num_queues);

        let mut vqs = Vec::new();

        for queue in 0..num_queues {
            ioport.write16(queue, QUEUE_SELECT);

            let size = ioport.read16(QUEUE_SIZE);

            let notify_off = ioport.read16(QUEUE_NOTIFY_OFF);

            let vq = Virtqueue::new(queue, size as usize, notify_off);

            ioport.write64(unsafe { mmu::virt_to_phys(vq.raw_descriptor_table_ptr) as u64 }, QUEUE_DESC);
            ioport.write64(unsafe { mmu::virt_to_phys(vq.raw_available_ring_ptr) as u64 }, QUEUE_AVAIL);
            ioport.write64(unsafe { mmu::virt_to_phys(vq.raw_used_ring_ptr) as u64 }, QUEUE_USED);

            // RX queue:
            if queue == VIRTIO_RX_QUEUE_IDX {
                vq.add_inbuf(unsafe { mmu::virt_to_phys(dev.rx_page as usize) as usize }, dev.rx_page_size);
                let vector = unsafe { dev.pci_dev.register_irq(queue, VirtioNetDevice::interrupt, mem::transmute(Rc::as_ptr(&dev))) };
                dev.pci_dev.enable_irq(queue);
                ioport.write16(queue, QUEUE_MSIX_VECTOR);
                println!("virtio-net: virtqueue {} is using IRQ vector {}", queue, vector);
            }
            ioport.write16(1 as u16, QUEUE_ENABLE);

            vqs.push(vq);
        }

        dev.vqs.replace(vqs);

        //
        // 8. Set DRIVER_OK status bit
        //
        status |= VIRTIO_DRIVER_OK;
        ioport.write8(status, DEVICE_STATUS);

        let dev_features = Features::from_bits_truncate(ioport.read32(DEVICE_FEATURE));

        if dev_features.contains(Features::VIRTIO_NET_F_MAC) {
            if let Some(dev_cfg_cap) = VirtioNetDevice::find_capability(&dev.pci_dev, VIRTIO_PCI_CAP_DEVICE_CFG) {
                let dev_cfg_ioport = dev_cfg_cap.map(&dev.pci_dev).unwrap();
                let mut mac: [u8; 6] = [0; 6];
                for i in 0..mac.len() {
                    mac[i] = dev_cfg_ioport.read8(i);
                }
                println!(
                    "virtio-net: MAC address is {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
                );
                dev.mac_addr.replace(Some(mac));
            }
        }

        Some(Rc::new(Device::new(VIRTIO_DEV_NAME, RefCell::new(dev))))
    }

    fn find_capability(pci_dev: &PCIDevice, cfg_type: u8) -> Option<VirtioPCICap> {
        let mut capability = pci_dev.func.find_capability(PCI_CAPABILITY_VENDOR);
        while let Some(offset) = capability {
            let ty = pci_dev.func.read_config_u8(offset + VIRTIO_PCI_CAP_CFG_TYPE);
            let bar_idx = pci_dev.func.read_config_u8(offset + VIRTIO_PCI_CAP_BAR);
            if ty == cfg_type && bar_idx < 0x05 {
                return Some(VirtioPCICap::new(offset, bar_idx))
            }
            capability = pci_dev.func.find_next_capability(PCI_CAPABILITY_VENDOR, offset);
        }
        None
    }

    fn recv(&self) {
        let vq = &self.vqs.borrow()[VIRTIO_RX_QUEUE_IDX as usize];
        let last_seen_idx = vq.last_seen_used();
        let last_used_idx = vq.last_used_idx();
        let mut idx = last_seen_idx;
        loop {
            if idx == last_used_idx {
                break
            }
            let (buf_addr, buf_len) = vq.get_used_buf(idx % vq.queue_size as u16);

            let buf_vaddr = unsafe { mmu::phys_to_virt(buf_addr) };
            let _hdr: *const VirtioNetHdr = unsafe { mem::transmute(buf_vaddr) };

            if let Some(rx_buffer_addr) = *self.rx_buffer_addr.borrow() {
                let packet_len = buf_len - mem::size_of::<VirtioNetHdr>();
                self.notifier.on_event(Event::PacketIO {
                    addr: rx_buffer_addr + mem::size_of::<VirtioNetHdr>(),
                    len: packet_len,
                });
            }

            vq.advance_last_seen_used();

            idx += 1;
        }
    }

    extern "C" fn interrupt(arg: usize) {
        let dev: *mut VirtioNetDevice = unsafe { mem::transmute(arg) };
        unsafe { (*dev).recv() };
    }

    fn new(pci_dev: Rc<PCIDevice>, notify_cfg_ioport: IOPort, notify_off_multiplier: u32) -> Self {
        VirtioNetDevice {
            pci_dev,
            notify_cfg_ioport,
            notify_off_multiplier,
            vqs: RefCell::new(Vec::new()),
            notifier: Rc::new(EventNotifier::new(VIRTIO_DEV_NAME)),
            /* FIXME: Free allocated pages when driver is unloaded.  */
            /* FIXME: Check if page allocator returned NULL.  */
            rx_page: memory::page_alloc_small() as usize,
            rx_page_size: memory::PAGE_SIZE_SMALL as usize,
            tx_page: memory::page_alloc_small() as usize,
            tx_page_size: memory::PAGE_SIZE_SMALL as usize,
            mac_addr: RefCell::new(None),
            rx_buffer_addr: RefCell::new(None),
            io_queue: RefCell::new(None),
        }
    }

    fn process_io_one(&self, cmd: IOCmd) {
        match cmd.opcode {
            Opcode::Submit => {
                unsafe { rlibc::memset(mem::transmute(self.tx_page), 0, mem::size_of::<VirtioNetHdr>()); }
                unsafe { rlibc::memcpy(mem::transmute(self.tx_page + mem::size_of::<VirtioNetHdr>()), cmd.addr, cmd.len); }
                let vq = &self.vqs.borrow()[VIRTIO_TX_QUEUE_IDX as usize];
                assert!(cmd.len <= self.tx_page_size);
                unsafe { vq.add_outbuf(mmu::virt_to_phys(self.tx_page as usize) as usize, mem::size_of::<VirtioNetHdr>() + cmd.len); }
                self.notify(vq);
            },
            Opcode::Complete => {
                let vq = &self.vqs.borrow()[VIRTIO_RX_QUEUE_IDX as usize];
                vq.add_inbuf(unsafe { mmu::virt_to_phys(self.rx_page as usize) as usize }, self.rx_page_size);
                self.notify(vq);
            }
        }
    }

    fn notify(&self, queue: &Virtqueue) {
        let notify_off = (self.notify_off_multiplier * queue.notify_off as u32) as usize;
        self.notify_cfg_ioport.write16(queue.queue_idx, notify_off);
    }
}

impl DeviceOps for VirtioNetDevice {
    fn acquire(&self, vmspace: &mut VMAddressSpace, listener: Rc<dyn EventListener>) -> Result<()> {
        self.notifier.add_listener(listener);

        let (rx_buf_start, rx_buf_end) = vmspace.allocate(self.rx_page_size, memory::PAGE_SIZE_SMALL as usize, VMProt::VM_PROT_READ)?;
        vmspace.map(rx_buf_start, rx_buf_end, self.rx_page)?;
        self.rx_buffer_addr.replace(Some(rx_buf_start));

        let io_buf_size = 4096;
        let (io_buf_start, io_buf_end) = vmspace.allocate(io_buf_size, memory::PAGE_SIZE_SMALL as usize, VMProt::VM_PROT_RW)?;
        vmspace.populate(io_buf_start, io_buf_end)?;
        let io_queue = IOQueue::new(io_buf_start, io_buf_size);
        self.io_queue.replace(Some(io_queue));

        Ok(())
    }

    fn subscribe(&self, _events: &'static str) {
        // TODO: A process receives packets from all flows. Implement flow filtering.
    }

    fn get_config(&self, opt: ConfigOption) -> Option<Vec<u8>> {
        match opt {
            CONFIG_ETHERNET_MAC_ADDRESS => { self.mac_addr.borrow().as_ref().map(|a| a.to_vec() ) },
            CONFIG_IO_QUEUE => {
                if let Some(io_queue) = self.io_queue.borrow_mut().as_mut() {
                    return Some(io_queue.ring_buffer.raw_ptr().to_ne_bytes().to_vec())
                }
                None
            },
            _ => { None }
        }
    }

    fn process_io(&self) {
        if let Some(io_queue) = self.io_queue.borrow_mut().as_mut() {
            while let Some(cmd) = io_queue.pop() {
                self.process_io_one(cmd);
            }
        }
    }
}

pub static mut VIRTIO_NET_DRIVER: PCIDriver =
    PCIDriver::new(
        DeviceID::new(PCI_VENDOR_ID_REDHAT, PCI_DEVICE_ID_VIRTIO_NET),
        VirtioNetDevice::probe,
    );
