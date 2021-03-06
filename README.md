This is the repository containing my work on Senior Project at Cal Poly.

## Project Information
__Author:__ J. Ethan Adams (jadams30@calpoly.edu, j.ethan.adams@gmail.com)

__Project Advisor:__ Dr. John Bellardo (http://users.csc.calpoly.edu/~bellardo/index.php)

----------

## Project Background
This project is a fork of my `csc454` project where I developed much of the basic functionality of a simple `x86_64` Operating System. Work for senior project will pick up where I left off in this class (to see a description of my previous work, see https://github.com/jadams41/Senior-Project/blob/master/archived/CPE454_README.md)

----------

## Project Scope and Overview
The goal of this to develop a custom, functional `x86_64` *unix-like* Operating System.

#### Project Objectives
1. OS includes robust implementation of all functionality outlined in the project for `csc454`.
2. Fully implemented and functional networking stack (capable of communicating across layer-3 networks via `TCP/IP`).
3. OS can be built for physical hardware targets. All implemented OS features will be supported and verifiably functional for each supported hardware target.

_NOTE: more objectives will likely be added once `Objective 2` has been accomplished._

#### Current Functionality
1. Kernel Boot Procedure (assembly):
   1. Create stack and store in `esp` register
   2. Test for Multiboot-compliant bootloader and store Multiboot Information Structure pointer
   3. Check for `CPUID` and Long Mode availability
   4. Create initial page tables and enable paging
   5. Enter Long Mode
   6. Call `kmain` with stored multiboot information
2. Initailize VGA buffer and basic VGA output capability
3. Initialize PS/2 driver to handle keyboard input
4. Enable Interrupts
   1. Initialize Programmable Interrupt Controller and enable cascading between master and slave PICs. Mask all interrupts.
   2. Initialize `idt` and load with `lidt`.
   3. Initialize `ist`(most interrupts use the general interrupt stack while GPF, DF, PF, and Syscalls are given unique interrupt stacks).
   4. Unmask and install ISRs for PS/2, Serial, DF, ATA, GPF, PF, and Syscall interrupts.
5. Configure Serial Communication Driver (connected to host machine for debugging purposes). All following kernel output will be printed to VGA buffer and sent to host.
6. Process Multiboot tags and extract relevant information (memory map and elf sybmol tags).
7. Create new 4 level page table and initialize set up virtual memory (and dynamic memory allocation).
   1. Kernel Heap is initalized.
   2. Allocated virtual memory is paged "on demand" (no physical page used for an allocated virtual page until memory is accessed and page fault is raised).
8. Probe for ATA devices and initialize if found.
   1. Probe for filesystems on each found device and represent each as "Virtual Filesystem".
9. Probe for PCI devices and initialize if found.
   1. *Currently only focused on initializing rtl8139 NIC*
10. Initialize found Network Interface Controller.
   1. Process and modify relevant information stored in PCI configuration space.

##### Demos of Current Functionality:
###### Receiving Network traffic
![](demos/receive_internet_traffic.gif)

###### Receiving and Parsing ARP packets
*TODO: Add demo of this functionality*

###### Received ARP Packet Handling Functionality
*TODO: Add demo of this functionality*

---------

## Project Planning

### Milestones
#### Milestone 0: *Set up Senior Project and Document Project Scope*

  1. **Take care of administrative tasks for setting up Senior Project**
      - [X] Create project proposal documenting scope and objectives.
      - [X] Procure project advisor.
      - [X] Submit Pre-Enrollment Proposal to officially enroll in Senior Project I.
  2. **Set up Project**
      - [X] Create GitHub Repository for Documenting project information
      - [X] Populate project's `README` with initial information regarding project background, scope & objectives, and tentative timeline of critical work.
---------

#### Milestone 1: *Necessary Maintenance and Preparations for New Development*

  1. **Revise previous work.**
      - [X] Review all of previous work - make sure functionality and limitations are understood and documented.
      - [X] Fix identified erroneous/limited implementation.
  2. **Create a more robust and stable development environment.**
      - [X] Review and document functionality of current build system (make).
      - [ ] Migrate build of project (operating system) to use `bazel`. **Ignoring this for now**
      - [X] Document how to build and run project with new build system.
      - [ ] Research, implement, and document project testing strategy.
      - [ ] _Explore possibility of migrating project from `C` to `C++`._
  3. **Complete all `cpe454` functionality milestones**
      - [X] Review and document initial project's accomplishment of milestones.
      - [X] Fix all partially implemented functionality milestones.
      - [ ] Implement remaining non-attempted milestones.
---------

#### Milestone 2: *Implementation of Network Stack*
1. **Develop Peripheral Component Interconnect (`PCI`) driver _(Needed to discover, configure, and interact with Network Interface Card)_**
   - [X] Research and document relevant PCI functionality and control.
      - [X] Implement basic PCI driver (capable of reading from and writing to PCI configuration space).
      - [X] Implement `pci_probe` (discovery and handling of all devices connected to PCI bus):
         - [X] Detect all devices connected to the PCI bus.
         - [X] Develop super structure to represent PCI device (`PCIDevice`) with child structures to handle different types of PCI devices. Access to PCI control should be handled through function pointers set in super `PCIDevice` structure.
         - [X] Integrate functionality: discover all connected devices, store relevant information in `PCIDevice`s, return list of discovered `PCIDevice`s.
2. **Implement Network Interface Card (`NIC`) driver**
   - [X] Research and document different NIC models.
      - [X] Choose initial NIC to develop driver for and document rationale.
      - [X] Implement primitive driver functionality:
         - [X] NIC initalization (utilizing pci control in `PCIDevice` struct returned from `pci_probe`)
         - [X] Basic implementation of asynchronous NIC control (transmitting and receiving packets).
         - [X] Enable NIC's interrupts and install ISRs which use asynchronous NIC control functionality.
      - [X] Test basic NIC functionality (transmit and receive ethernet frames):
         - [X] Research and document strategies for networking between `qemu` VM's and host machine.
         - [X] TEST: Guest receives ethernet frame from host VM:
            - [X] Verify that interrupt fires after frame is sent from host -> VM
            - [X] Verify that frame's header information and data can be properly extracted in VM.
         - [X] TEST: Guest VM sends ethernet frame to host:
            - [X] Verify that ethernet frame sent from guest is received on the host VM.
            - [X] Verify that frame's header information and data are consistent (when received on host).
3. **Build Network Stack**
   - [X] **Link Layer**:
      - [X] `Ethernet`
         - [X] Discover and store interface's MAC Address
         - [X] Receive and parse Ethernet frames (test with `arping` verify with `wireshark` running on bridge interface)
         - [X] Create and send Ethernet frames (test by sending broadcast frame, verify packet validity with `wireshark`)
      - [X] `ARP`: _Address Resolution Protocol_
         - [X] Create `ARP` infrastructure
            - [X] Structs for representing/extracting info/creating ARP packets
            - [X] Create primitive ARP table for keeping track of received ARP traffic
         - [X] Test ARP Infrastructure
            - [X] Receive and correctly parse ARP traffic (test with `arping -I <bridge_iface> -b <ip_on_bridge_subnet>` and verify with `wireshark` running on bridge interface connected to os)
         - [X] Send test ARP traffic (both `ARP_REQUEST` and `ARP_REPLY`)
             - [X] Verify that sent packets valid (by running `wireshark` on bridge interface connected to the os).
             - [X] `ARP_REQUEST` the bridge interface's IP address and make sure `ARP_REPLY` is received from bridge interface.
         - [X] Flush out desired kernel ARP functionality:
            - [X] Update `arp_table` when received ARP traffic contains new/updated `ipv4->mac_addr` mapping.
            - [X] Handle all received ARP traffic automatically:
            - [X] Upon receiving `ARP_REQUEST` for os's (mock) ipv4, create and send correct `ARP_REPLY` with our information. Verify by:
               1. `arping` for os's mock ip
               2. ensuring that the os receives/handles the `ARP_REQUEST` and creates+sends `ARP_REPLY` back.
               3. Seeing the reply is received by `arping`
               4. Checking host's arp table (using `arp`) and checking for entry: `os_mock_ip -> os_mac_addr`
   - [ ] **Internet Layer**
      - [ ] `IPv4`: _Internet Protocol Version 4_
         - [X] Create Internet Protocol infrastructure:
            - [X] Create more robust network device infrastructure and data structures (like Linux's `net_device`) for configuring/representing/storing IP information.
            - [X] Structs+Functions for representing/parsing/creating IP packets.
         - [X] Send and receive/parse mock IP traffic (maybe use `ICMP` or `UDP` "traffic" for this).
         - [X] *Figure out what kernel data structures are needed for IP layer*
      - [X] `ICMP`: _Internet Control Message Protocol_
         - [X] Create ICMP infrastructure:
            - [X] OS recognizes and handles `ICMP` packets (probably only need control messages associated with `ping`).
            - [X] OS can send relevant `ICMP` requests (probably only need to support control messages associated with `ping`).
         - [X] Flush out desired kernel `ICMP` functionality:
            - [X] OS automatically handles all incoming `ICMP` traffic
               - [X] OS extracts and uses relevant information from received `ICMP` traffic upon arrival.
               - [X] OS automatically replies appropriately to all received `ICMP` requests (that are supported).
            - [ ] "Userspace" accessible `ping` functionality (can probably be done in a kernel thread):
               - [ ] As similar as possible to running `ping` on linux command line
               - [ ] Outputs same way as linux `ping` utility (to make testing/validating easier).
   - [ ] **Transport Layer**
      - [X] `UDP`: _Universal Datagram Protocol_
      - [ ] `TCP`: _Transmission Control Protocol_
   - [ ] **Application Layer**
      - [ ] `DHCP`: _Dynamic Host Configuration Protocol_
         - *NOTE: rush to get here ASAP (build only what is needed in order to get DHCP DISCOVER->REQUEST functionality working in order to support dynamic IP addressineg of the OS).*
      - [ ] `HTTP`: _HyperText Transmission Protocol_
         - *NOTE: Final objective for Milestone 3*
---------

#### _Potential Future Milestones_
  1. Implement better graphics support.
  2. Improve process control capabilities.
  3. Implement disk-encryption support.

----------
