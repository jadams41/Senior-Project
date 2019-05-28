## Network Device Discovery
#### IP discovery (ping scan)
```bash
sudo nmap -sn <network ip address>/<network subnet mask>
```

#### HW_Address Discovery (arp scan)
```bash
sudo arp-scan --interface=<interface name> --localnet
```
