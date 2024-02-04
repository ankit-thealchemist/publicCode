| Supported Targets | ESP32 | ESP32-S3 | ESP32-C3 |
| :------------------ | ------- | ---------- | ---------- |

# Ethernet Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## Overview

This example demonstrates basic usage of `Ethernet driver` together with `tcpip_adapter`. The work flow of the example could be as follows:

1. Install Ethernet driver
2. Send DHCP requests and wait for a DHCP lease
3. If get IP address successfully, then you will be able to ping the device

If you have a new Ethernet application to go (for example, connect to IoT cloud via Ethernet), try this as a basic template, then add your own code.

## How to use example

### Hardware Required

* The breakout board of ethernet module w5500
* The ESP32-S3 dev board.

#### Pin Assignment

* SCLK 12
* MOSI 11
* MISO 13
* CS 10
* INT 4
* RESET 5

### Configure the project

```
idf.py menuconfig
```

### Build, Flash, and Run

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT build flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

## Example Output

```bash
I (322) w5500.mac: version=0
I (332) esp_eth.netif.netif_glue: 02:00:00:12:34:56
I (332) esp_eth.netif.netif_glue: ethernet attached to netif
I (332) eth_example: Ethernet Started
I (4332) eth_example: Ethernet Link Up
I (4332) eth_example: Ethernet HW Addr 02:00:00:12:34:56
I (5332) esp_netif_handlers: eth0 ip: 10.42.0.216, mask: 255.255.255.0, gw: 10.42.0.1
I (5332) eth_example: Ethernet Got IP Address
I (5332) eth_example: ~~~~~~~~~~~
I (5332) eth_example: ETHIP:10.42.0.216
I (5342) eth_example: ETHMASK:255.255.255.0
I (5342) eth_example: ETHGW:10.42.0.1
I (5352) eth_example: ~~~~~~~~~~~
```

Now you can ping your ESP32 in the terminal by entering `ping 10.42.0.216` (it depends on the actual IP address you get).

```bash
PING 10.42.0.216 (10.42.0.216) 56(84) bytes of data.
64 bytes from 10.42.0.216: icmp_seq=1 ttl=255 time=1.65 ms
64 bytes from 10.42.0.216: icmp_seq=2 ttl=255 time=1.69 ms
64 bytes from 10.42.0.216: icmp_seq=3 ttl=255 time=1.66 ms
64 bytes from 10.42.0.216: icmp_seq=4 ttl=255 time=1.68 ms
64 bytes from 10.42.0.216: icmp_seq=5 ttl=255 time=1.67 ms
64 bytes from 10.42.0.216: icmp_seq=6 ttl=255 time=1.70 ms
64 bytes from 10.42.0.216: icmp_seq=7 ttl=255 time=1.66 ms
64 bytes from 10.42.0.216: icmp_seq=8 ttl=255 time=1.66 ms
64 bytes from 10.42.0.216: icmp_seq=9 ttl=255 time=1.62 ms
64 bytes from 10.42.0.216: icmp_seq=10 ttl=255 time=1.67 ms
64 bytes from 10.42.0.216: icmp_seq=11 ttl=255 time=1.68 ms
```
