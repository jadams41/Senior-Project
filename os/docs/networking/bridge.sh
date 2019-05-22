# ************************************************
# *** CHECK CURRENT NETWORKING INTERFACES *** 
# ************************************************
ip addr


# ************************************************
# *** CREATE A NETWORK BRIDGE ***
# ************************************************
sudo ip link add name <name_of_bridge> type bridge # bridge should now show up in `ip addr`

# ************************************************
# *** SET UP BRIDGE ***
# ************************************************
sudo ip link set dev <name_of_bridge> up


# ************************************************
# *** BRING UP BRIDGE AND CONNECT INTERFACE ***
# ************************************************
# make sure interface to connect to bridge is up
sudo ip link set dev <name_of_interface_to_bridge> up
# make interface slave to bridge
sudo ip link set dev <name_of_interface_to_bridge> master <bridge_name>


# *********************************************************** #
#                 *** TEAR DOWN THE BRIDGE ***                #
# *********************************************************** #
# - remove connected interface from the bridge                #
sudo ip link set dev <interface connected to bridge> nomaster #
# - delete the bridge                                         #
sudo ip link dev delete <name of bridge>                      #
# *********************************************************** #

# ******************************************************* #
#             *** USEFUL `brctl` COMMANDS ***             #
# ******************************************************* #
# - List all brctl commands                               #
brctl                                                     #
# - Show all bridge devices                               #
brctl show                                                #
# - Show mac addresses of all devices connected to bridge #
brctl showmacs <bridge_name>                              #
# ******************************************************* #
