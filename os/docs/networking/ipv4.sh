# ************************************************************************ #
#                        *** CHECK IP ADDRESSES ***                        #
# ************************************************************************ #
# - Show system private ip addresses                                       #
hostname -I                                                                #
#                                                                          #
# - Show public ip address (using dns lookup)                              #
dig +short myip.opendns.com @resolver1.opendns.com                         #
#                                                                          #
# - Show detailed ip information for all interfaces                        #
ip addr                                                                    #
# - Show ipv4 with subnet mask of specific interface                       #
#                                                                          #
ip addr show <interface> | grep 'inet ' | awk '{print $2}'                 #
# - Show ipv4 address of specific interface                                #
ip addr show <interface> | grep 'inet ' | awk '{print $2}' | cut -f1 -d'/' #
#                                                                          #
# - Show subnet mask of specific interface                                 #
ip addr show <interface> | grep 'inet ' | awk '{print $2}' | cut -f2 -d'/' #
#                                                                          #
# ************************************************************************ #

# *********************************************************** #
#            *** MODIFY SYTEM IP CONFIGURATION ***            # 
# *********************************************************** #
# Manually set ip address for interface                       #
sudo ifconfig <interface> <new_ip_addr> netmask <new_netmask> #
#                                                             #
# Manually set interface's default gateway                    #
sudo route add default gw <default_gw_ip> <iface>             #
#                                                             #
# - Request ipv4 address from dhcp server                     #
dhclient -v <interface>                                       #
#                                                             #
#                                                             #
#                                                             #
#                                                             #
#                                                             #
#                                                             #
#                                                             #

# *********************************************************** #
