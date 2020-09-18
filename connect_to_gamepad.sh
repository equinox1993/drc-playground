#!/bin/bash

source config/constants.sh

if [[ "$(whoami)" != "root" ]]; then
  echo "Try again as root"
  exit 1
fi

iw reg set US
ip addr flush dev "$WLAN"
# 192.168.1.10: console; 192.168.1.11: gamepad
ip a a 192.168.1.10/24 dev "$WLAN"
ip l set mtu 1800 dev "$WLAN"

"$NETBOOT_BIN" 192.168.1.255 192.168.1.10 192.168.1.11 "$GAMEPAD_MAC" &
NETBOOT_PID="$!"

TMPDIR="$(mktemp -d)"

cat wiiu_ap_normal.conf |
    sed -e "s/^interface=.*/interface=$WLAN/" |
    sed -e "s/^ssid=.*/ssid=$CONSOLE_SSID/" |
    sed -e "s/^wpa_psk=.*/wpa_psk=$CONSOLE_PSK/" > "$TMPDIR/wiiu_ap_normal.conf"

"$HOSTAPT_BIN" -dd "$TMPDIR/wiiu_ap_normal.conf"

kill "$NETBOOT_PID"

rm -r "$TMPDIR"
