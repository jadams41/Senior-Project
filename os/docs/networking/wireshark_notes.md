#### Filters
* Two different types of filters:
    * DisplayFilter (filters what you see in the capture)
    * CaptureFitler (filters what wireshark captures)

* Relevant DisplayFilters:
    * __Show traffic of certain protocol__
        filter = `<protocol_name>`
        examples of protocol_names: `arp`, `icmp`, `tcp`, `http`, `ip`, etc.
    * __show only ipv4 traffic__
        filter = `ip.version == 4`
    * __Show only ipv6 traffic__
        filter = `ipv6`
    * __Show only ethernet frames with specific source or destination__
        filter = `eth.src == <mac_addr> or eth.dst = <mac_addr>`
    
