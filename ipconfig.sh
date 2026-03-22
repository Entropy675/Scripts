#!/bin/bash
# for qubes cloned HVM auto local ip grabbing
 
# Configuration
USER_HOME="/home/entropy/Desktop"
CACHE_FILE="$USER_HOME/assigned_ip.txt"
INTERFACE="enX0"
GATEWAY="10.138.12.94"
TEST_TARGET="8.8.8.8"
 
# 1. Check for the "Identity File" on the Desktop
if [ -f "$CACHE_FILE" ]; then
    SELECTED_IP=$(cat "$CACHE_FILE")
    echo "Found cached IP on Desktop: $SELECTED_IP"
else
    echo "First boot for this clone. Searching for valid IP..."
    
    # Ensure interface is up to start probing
    ip link set $INTERFACE up
    
    for x in {0..255}; do
        TRY_IP="10.137.0.$x"
        
        # Assign the IP temporarily
        ip addr flush dev $INTERFACE
        ip addr add $TRY_IP/32 dev $INTERFACE > /dev/null 2>&1
        ip route add $GATEWAY dev $INTERFACE scope link > /dev/null 2>&1
        ip route add default via $GATEWAY > /dev/null 2>&1
        
        # Settlement delay to prevent race conditions
        sleep 0.5
        
        # Ping test with 1s timeout
        if ping -c 1 -W 1 $TEST_TARGET > /dev/null 2>&1; then
            SELECTED_IP=$TRY_IP
            echo "$SELECTED_IP" > "$CACHE_FILE"
            chown entropy:entropy "$CACHE_FILE"
            echo "Successfully identified and saved: $SELECTED_IP"
            break
        fi
    done
fi
 
# 2. Apply Final Network State
if [ -n "$SELECTED_IP" ]; then
    ip addr flush dev $INTERFACE
    ip addr add $SELECTED_IP/32 dev $INTERFACE
    ip link set $INTERFACE up
    ip route add $GATEWAY dev $INTERFACE scope link 2>/dev/null
    ip route add default via $GATEWAY 2>/dev/null
 
    # DNS Configuration
    echo "nameserver 8.8.8.8" > /etc/resolv.conf
    echo "nameserver 1.1.1.1" >> /etc/resolv.conf
    
    # 3. Restart Sunshine (runs as user)
    # We use the explicit path to the user's bus to ensure systemd-user communication
    USER_ID=$(id -u entropy)
    sudo -u entropy DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$USER_ID/bus \
    # systemctl --user restart sunshine
    
    echo "Network and Sunshine service are ready."
else
    echo "Error: No valid IP could be determined in the 10.137.0.x range."
    exit 1
fi
 
