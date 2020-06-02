//! Virtio network device driver.

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;
use core::intrinsics::transmute;
use core::mem;
use kernel::device::{ConfigOption, Device, DeviceOps, CONFIG_ETHERNET_MAC_ADDRESS, CONFIG_IO_QUEUE};
use kernel::event::{Event, EventListener, EventNotifier};
use kernel::ioport::IOPort;
use kernel::ioqueue::{IOCmd, IOQueue};
use kernel::memory;
use kernel::mmu;
use kernel::print;
use kernel::vm::{VMAddressSpace, VMProt};
use pci::{DeviceID, PCIDevice, PCIDriver, PCI_CAPABILITY_VENDOR, PCI_VENDOR_ID_REDHAT};
use virtqueue::{Virtqueue};

const PCI_DEVICE_ID_VIRTIO_NET: u16 = 0x1041;

#[allow(dead_code)]
const DEVICE_FEATURE_SELECT: usize = 0x00;
#[allow(dead_code)]
const DEVICE_FEATURE: usize = 0x04;
#[allow(dead_code)]
const DRIVER_FEATURE_SELECT: usize = 0x08;
#[allow(dead_code)]
const DRIVER_FEATURE: usize = 0x0c;
#[allow(dead_code)]
const MSIX_CONFIG: usize = 0x10;
#[allow(dead_code)]
const NUM_QUEUES: usize = 0x12;
#[allow(dead_code)]
const DEVICE_STATUS: usize = 0x14;
#[allow(dead_code)]
const CONFIG_GENERATION: usize = 0x15;
#[allow(dead_code)]
const QUEUE_SELECT: usize = 0x16;
#[allow(dead_code)]
const QUEUE_SIZE: usize = 0x18;
#[allow(dead_code)]
const QUEUE_MSIX_VECTOR: usize = 0x1a;
#[allow(dead_code)]
const QUEUE_ENABLE: usize = 0x1c;
#[allow(dead_code)]
const QUEUE_NOTIFY_OFF: usize = 0x1e;
#[allow(dead_code)]
const QUEUE_DESC: usize = 0x20;
#[allow(dead_code)]
const QUEUE_AVAIL: usize = 0x28;
#[allow(dead_code)]
const QUEUE_USED: usize = 0x30;

#[allow(dead_code)]
const VIRTIO_ACKNOWLEDGE: u8 = 1;
#[allow(dead_code)]
const VIRTIO_DRIVER: u8 = 2;
#[allow(dead_code)]
const VIRTIO_FAILED: u8 = 128;
#[allow(dead_code)]
const VIRTIO_FEATURES_OK: u8 = 8;
#[allow(dead_code)]
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

#[allow(dead_code)]
const VIRTIO_PCI_CAP_COMMON_CFG: u8 = 1;
#[allow(dead_code)]
const VIRTIO_PCI_CAP_NOTIFY_CFG: u8 = 2;
#[allow(dead_code)]
const VIRTIO_PCI_CAP_ISR_CFG: u8 = 3;
#[allow(dead_code)]
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
    num_buffers: u16,
}

struct VirtioNetDevice {
    notify_cfg_ioport: IOPort,
    notify_off_multiplier: u32,
    vqs: Vec<Virtqueue>,
    notifier: Rc<EventNotifier>,
    rx_page: usize,
    tx_page: usize,
    mac_addr: Option<MacAddr>,
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

/* FIXME: Implement a virtual memory allocator insted of open-coding addresses here. */
/// The virtual address of the receive packet buffer memory area that is mapped to user space.
const VIRTIO_NET_RX_BUFFER_ADDR: usize = 0x90000000;

const VIRTIO_NET_IO_QUEUE_ADDR: usize = 0xA0000000;

/// The name of this virtio device that is exported to user space.
const VIRTIO_DEV_NAME: &str = "/dev/eth";

impl VirtioNetDevice {
    fn probe(pci_dev: &PCIDevice) -> Option<Rc<Device>> {
        pci_dev.set_bus_master(true);

        pci_dev.enable_msix();

        let notify_cfg_cap = VirtioNetDevice::find_capability(pci_dev, VIRTIO_PCI_CAP_NOTIFY_CFG).unwrap();

        let notify_off_multiplier = pci_dev.func.read_config_u32(notify_cfg_cap.offset + VIRTIO_NOTIFY_OFF_MULTIPLIER);

        let notify_cfg_ioport = notify_cfg_cap.map(pci_dev).unwrap();

        let mut dev = VirtioNetDevice::new(notify_cfg_ioport, notify_off_multiplier);

        let common_cfg_cap = VirtioNetDevice::find_capability(pci_dev, VIRTIO_PCI_CAP_COMMON_CFG).unwrap();

        let ioport = common_cfg_cap.map(pci_dev).unwrap();

        println!("virtio-net: using PCI BAR{} for device configuration", common_cfg_cap.bar_idx);

        //
        // 1. Reset device
        //
        let mut status: u8 = 0;
        unsafe { ioport.write8(status, DEVICE_STATUS) };

        //
        // 2. Set ACKNOWLEDGE status bit
        //
        status |= VIRTIO_ACKNOWLEDGE;
        unsafe { ioport.write8(status, DEVICE_STATUS) };

        //
        // 3. Set DRIVER status bit
        //
        status |= VIRTIO_DRIVER;
        unsafe { ioport.write8(status, DEVICE_STATUS) };

        //
        // 4. Negotiate features
        //
        let features = Features::VIRTIO_NET_F_MAC | Features::VIRTIO_NET_F_MRG_RXBUF;
        unsafe { ioport.write32(features.bits(), DRIVER_FEATURE) };

        //
        // 5. Set the FEATURES_OK status bit
        //
        status |= VIRTIO_FEATURES_OK;
        unsafe { ioport.write8(status, DEVICE_STATUS) };

        //
        // 6. Re-read device status to ensure the FEATURES_OK bit is still set
        //
        if unsafe { ioport.read8(DEVICE_STATUS) } & VIRTIO_FEATURES_OK != VIRTIO_FEATURES_OK {
            panic!("Device does not support our subset of features");
        }

        //
        // 7. Perform device-specific setup
        //
        let num_queues = unsafe { ioport.read16(NUM_QUEUES) };

        println!("virtio-net: {} virtqueues found.", num_queues);

        for queue in 0..num_queues {
            unsafe { ioport.write16(queue as u16, QUEUE_SELECT) };

            let size = unsafe { ioport.read16(QUEUE_SIZE) };

            let notify_off = unsafe { ioport.read16(QUEUE_NOTIFY_OFF) };

            let vq = Virtqueue::new(size as usize, notify_off);

            unsafe {
                ioport.write64(
                    mmu::virt_to_phys(vq.raw_descriptor_table_ptr) as u64,
                    QUEUE_DESC,
                );
                ioport.write64(
                    mmu::virt_to_phys(vq.raw_available_ring_ptr) as u64,
                    QUEUE_AVAIL,
                );
                ioport.write64(mmu::virt_to_phys(vq.raw_used_ring_ptr) as u64, QUEUE_USED);

                // RX queue:
                if queue == VIRTIO_RX_QUEUE_IDX {
                    vq.add_inbuf(mmu::virt_to_phys(dev.rx_page as usize) as usize, memory::PAGE_SIZE_SMALL as usize);
                    let vector = pci_dev.register_irq(queue, VirtioNetDevice::interrupt, transmute(&dev));
                    pci_dev.enable_irq(queue);
                    ioport.write16(queue, QUEUE_MSIX_VECTOR);
                    println!("virtio-net: virtqueue {} is using IRQ vector {}", queue, vector);
                }
                ioport.write16(1 as u16, QUEUE_ENABLE);
            }

            dev.add_vq(vq);
        }

        //
        // 8. Set DRIVER_OK status bit
        //
        status |= VIRTIO_DRIVER_OK;
        unsafe {
            ioport.write8(status, DEVICE_STATUS);
        }

        let dev_features = Features::from_bits_truncate(unsafe { ioport.read32(DEVICE_FEATURE) });

        if dev_features.contains(Features::VIRTIO_NET_F_MAC) {
            if let Some(dev_cfg_cap) = VirtioNetDevice::find_capability(pci_dev, VIRTIO_PCI_CAP_DEVICE_CFG) {
                let dev_cfg_ioport = dev_cfg_cap.map(pci_dev).unwrap();
                let mut mac: [u8; 6] = [0; 6];
                for i in 0..mac.len() {
                    mac[i] = unsafe { dev_cfg_ioport.read8(i) };
                }
                println!(
                    "virtio-net: MAC address is {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
                );
                dev.mac_addr = Some(mac);
            }
        }

        Some(Rc::new(Device::new(VIRTIO_DEV_NAME, RefCell::new(Rc::new(dev)))))
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
        let vq = &self.vqs[VIRTIO_RX_QUEUE_IDX as usize];
        let last_seen_idx = vq.last_seen_used();
        let last_used_idx = vq.last_used_idx();
        if last_seen_idx != last_used_idx {
            /* FIXME: Fix loop when indexes wrap around.  */
            assert!(last_seen_idx < last_used_idx);
            for idx in last_seen_idx..last_used_idx {
                let (buf_addr, buf_len) = vq.get_used_buf(idx);

                let buf_vaddr = unsafe { mmu::phys_to_virt(buf_addr) };
                let hdr: *const VirtioNetHdr = unsafe { transmute(buf_vaddr) };
                assert!(unsafe { (*hdr).num_buffers } == 1);

                let packet_len = buf_len - mem::size_of::<VirtioNetHdr>();
                self.notifier.on_event(Event::PacketIO {
                    addr: VIRTIO_NET_RX_BUFFER_ADDR + mem::size_of::<VirtioNetHdr>(),
                    len: packet_len,
                });

                vq.advance_last_seen_used();

                /* FIXME: We reuse the same buffer, but user space has not consumed it yet.  */
                vq.add_buf_idx(0);
            }
        }
    }

    extern "C" fn interrupt(arg: usize) {
        let dev: *mut VirtioNetDevice = unsafe { transmute(arg) };
        unsafe { (*dev).recv() };
    }

    fn new(notify_cfg_ioport: IOPort, notify_off_multiplier: u32) -> Self {
        VirtioNetDevice {
            notify_cfg_ioport: notify_cfg_ioport,
            notify_off_multiplier,
            vqs: Vec::new(),
            notifier: Rc::new(EventNotifier::new(VIRTIO_DEV_NAME)),
            /* FIXME: Free allocated pages when driver is unloaded.  */
            rx_page: memory::page_alloc_small() as usize,
            tx_page: memory::page_alloc_small() as usize,
            /* FIXME: Check if page allocator returned NULL.  */
            mac_addr: None,
            io_queue: RefCell::new(None),
        }
    }

    fn add_vq(&mut self, vq: Virtqueue) {
        self.vqs.push(vq);
    }

    fn io_submit(&self, cmd: IOCmd) {
        match cmd {
            IOCmd::PacketTX { addr, len } => {
                let hdr = VirtioNetHdr {
                    flags: 0,
                    gso_type: 0,
                    hdr_len: 0,
                    gso_size: 0,
                    csum_start: 0,
                    csum_offset: 0,
                    num_buffers: 0,
                };
                unsafe { rlibc::memcpy(mem::transmute(self.tx_page), mem::transmute(&hdr), mem::size_of::<VirtioNetHdr>()); }
                unsafe { rlibc::memcpy(mem::transmute(self.tx_page + mem::size_of::<VirtioNetHdr>()), addr, len); }
                let vq = &self.vqs[VIRTIO_TX_QUEUE_IDX as usize];
                unsafe { vq.add_outbuf(mmu::virt_to_phys(self.tx_page as usize) as usize, memory::PAGE_SIZE_SMALL as usize); }
                unsafe { self.notify_cfg_ioport.write16(VIRTIO_TX_QUEUE_IDX, (self.notify_off_multiplier * vq.notify_off as u32) as usize); }
            }
        }
    }
}

impl DeviceOps for VirtioNetDevice {
    fn acquire(&self, vmspace: &mut VMAddressSpace, listener: Rc<dyn EventListener>) {
        self.notifier.add_listener(listener);
        let rx_buf_start = VIRTIO_NET_RX_BUFFER_ADDR;
        let rx_buf_size = 4096;
        let rx_buf_end = rx_buf_start + rx_buf_size;
        vmspace.allocate(rx_buf_start, rx_buf_end, VMProt::VM_PROT_READ).expect("allocate failed");
        vmspace.map(rx_buf_start, rx_buf_end, self.rx_page).expect("populate failed");

        let io_buf_start = VIRTIO_NET_IO_QUEUE_ADDR;
        let io_buf_size = 4096;
        let io_buf_end = io_buf_start + io_buf_size;
        vmspace.allocate(io_buf_start, io_buf_end, VMProt::VM_PROT_RW).expect("allocate failed");
        vmspace.populate(io_buf_start, io_buf_end).expect("populate failed");
        let io_queue = IOQueue::new(io_buf_start, io_buf_size);
        self.io_queue.replace(Some(io_queue));
    }

    fn subscribe(&self, _events: &'static str) {
        // TODO: A process receives packets from all flows. Implement flow filtering.
    }

    fn get_config(&self, opt: ConfigOption) -> Option<Vec<u8>> {
        match opt {
            CONFIG_ETHERNET_MAC_ADDRESS => { self.mac_addr.map(|a| a.to_vec() ) },
            CONFIG_IO_QUEUE => {
                if let Some(io_queue) = self.io_queue.borrow_mut().as_mut() {
                    return Some(io_queue.ring_buffer.raw_ptr().to_ne_bytes().to_vec())
                }
                return None;
            },
            _ => { None }
        }
    }

    fn process_io(&self) {
        if let Some(io_queue) = self.io_queue.borrow_mut().as_mut() {
            loop {
                if let Some(cmd) = io_queue.pop() {
                    self.io_submit(cmd);
                } else {
                    break
                }
            }
        }
    }
}

pub static mut VIRTIO_NET_DRIVER: PCIDriver =
    PCIDriver::new(
        DeviceID::new(PCI_VENDOR_ID_REDHAT, PCI_DEVICE_ID_VIRTIO_NET),
        VirtioNetDevice::probe,
    );
