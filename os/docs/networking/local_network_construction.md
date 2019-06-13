## Setting up the OS's Surrounding Local Network

#### Interfaces
    1) `ens33`
        * Host machine's main interface
        * Brought up automatically as main network card by host when automatic network services are run
        * _Only interface with direct connection to the internet_
    2) `tap5`
        * a `tuntap` `tap` (layer 2 passthrough) virtual interface
        * provides host access/connection to the virtual network interface created by `qemu`
    3) Unnamed, virtual `rtl8139` NIC
        * Created by `qemu` when booting the OS
        * _Currently uses qemu's default MAC address, but can be changed_
    4) `br5`
        * Used to bridge `tap5` <-> `ens33`
        * Manually configured in my setup as main network interface (replacing `ens33`)

#### Setting up the Configuration that I Have Gotten to Work
    _NOTE: Considering that all of the interfaces in this setup are virtualized (even the host's - it is a virtual machine itself), it seems easier to manually configure all of these options on host startup instead of trying to setup permanent configurations._

###### Configuration Process from Fresh Host Boot
    1) Bring down and remove assigned ip address from host's default Network Interface
````bash 
    # WARNING, will lose network connection during this step
    # take down ens33
    sudo ifconfig ens33 down
    
    # bring up with no ipv4
    sudo ifconfig ens33 up 0.0.0.0
```
    2) Create and configure `tap5` interface
```bash
    # create the device
    sudo ip tuntap add tap5 mode tap
    
    # remove any default ip addresses assigned to it during creation
    sudo ip addr tap5 down
    sudo ip addr tap5 up 0.0.0.0
    
    # seems to always assign an ipv6 address at creation, I usually remove this, but don't think it's necessary 
```
    3) Delete any routing information left over from automatic setup
```bash
    # see list of currently configured routes
    sudo ip route
    
    # delete all routes (including default gateway)
    sudo ip route del <ipv4 address route>/<subnet mask>
```
    4) Create and configure `br5`
```bash
    # create the bridge
    sudo brctl addbr br5
    
    # turn off STP (creates a lot of noise on the network and there shouldn't be any loops in this simple layer-2 network)
    sudo brctl stp br5 off
    
    # add ens33 and tap5 to the bridge
    sudo brctl addif br5 ens33
    sudo brctl addif br5 tap5
    
    # request ip address from dhcp server for br5 (no other interfaces). 
    # NOTE: this should also automatically create necessary routes
    sudo dhclient -v br5
    
    # test network connection by pinging ip address outside of br5's subnet
    ping 8.8.8.8
```
    5) Supply necessary information to qemu
```bash
    # qemu commandline option to indicate tap5 should be used as network backend
    -netdev tap,id=mynet0,ifname=tap5,script=no,downscript=no
    
    # qemu commandline option to indicate that we want to emulate a rtl8139 NIC
   	-net nic,model=rtl8139,netdev=mynet0
    
    # This is weird, but for some reason I can only get "normal", fully functional network behavior when I set the destination MAC addr to br5 for all ethernet traffic
```
