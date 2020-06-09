# D-Link Camera Hacking Part 1: UART Discovery & Obtaining a shell

In this write up, we'll be taking a look at the [D-Link DCS-8010LH](https://www.dlink.com/en/products/dcs-8010lh-mydlink-hd-wi-fi-camera)

The goals of hacking this camera are to:

1. Obtain a shell on the device
2. Explore the filesystem from the shell
3. Extract the firmware off of the flash memory

## Teardown

I began by taking apart the camera to hunt for debug ports - Here's what the board looks like with its cover removed:

![Debug Ports](https://imgur.com/aK1x81u.jpg)

![Flash EEPROM](https://imgur.com/KE5oi7h.jpg)

The highlighted parts are:

Yellow: A Realtek chip of some sort. Contains the Realtek logo with the characters `RTS3902L`, `I2A08P6`, `G1061 TAIWAN` on it. Googling for these part numbers either reveals nothing of interest or a [forum posting](http://bbs.ntpcb.com/read-htm-tid-135134.html) which appears to contain the die / pinout for the chip. It is, however, in Chinese, which is not very helpful in my initial analysis.

Green: These appear to be the UART serial ports we were hunting for. They are helpfully labelled 3.3V, RX, TX, and GND.

Blue: This is a [Winbond W25Q128FV](https://www.winbond.com/resource-files/w25q128fv_revhh1_100913_website1.pdf) flash memory chip.

## UART

Let's focus in more on the UART ports. Using a [USB to TTL Serial Cable](https://www.adafruit.com/product/954) based on the [SI Labs CP2102](https://www.silabs.com/interface/usb-bridges/classic/device.cp2102) We're able to connect to the debug port.

One interesting caveat of connecting to this board's debug port was the pins were actually slightly mis-labeled. Normally, the TX line of the board connects to the RX line of the USB/TTL, and the RX line of the board connects to the TX line of the USB/TTL chip. This was not the case with this board. RX connected to RX and TX connected to TX. I was able to verify this using a multimeter to confirm the port labeled `RX` was experiencing lots of voltage fluctuations during the boot process - an indicator it was writing data out.

After I switched the RX to connect to RX and TX to connect to TX, we were able to start receiving data.

## BAUD Rates

Once I started receiving data from the board during boot, it was all gibberish. There are involved ways of determining baud rautes using logic analyzers but the easy way is to simply guess a bunch of common rates. With the help of [pyserial](https://github.com/pyserial/pyserial/blob/master/serial/tools/miniterm.py) we finally stumbled upon a baud rate of 57600:

```bash
johnny@dlinkcamera:$ miniterm.py /dev/cu.SLAB_USBtoUART 57600
--- Miniterm on /dev/cu.SLAB_USBtoUART  57600,8,N,1 ---
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---

U-Boot 2014.01-rc2-V1.1 (Jan 29 2018 - 11:59:50)
Board: IPCAM RTS3901 CPU: 500M :rx5281 prid=0xdc02
DRAM:  128 MiB @ 1066 MHz
Skipping flash_init
```

Now we've established the baud rate is `57600`. Let's see if we can get shell access.

## Shell Access

Letting the boot cycle continue, we are eventually able to interrupt the boot process which spawns us a U Boot shell:

<pre>
U-Boot 2014.01-rc2-V1.1 (Jan 29 2018 - 11:59:50)

Board: IPCAM RTS3901 CPU: 500M :rx5281 prid=0xdc02
DRAM:  128 MiB @ 1066 MHz
Skipping flash&#5;init
Flash: 0 Bytes
flash status is 0, 2, 0
SF: Detected W25Q128FV with page size 256 Bytes, erase size 64 KiB, total 16 MiB
Using default environment

In:    serial
Out:   serial
Err:   serial
MMC:   MMC: no card present
rtsmmc: 0
MMC: no card present
&#42;&#42; Bad device mmc 0 &#42;&#42;
Net:   Realtek PCIe GBE Family Controller mcfg = 0024
no hw config header
new&#5;ethaddr = 00:00:00:00:00:00
r8168#0
no hw config header
Hit any key to stop autoboot:  0 
rlxboot# 
</pre>

Waiting further into the boot process, we see this:

<pre>
 &#5;&#5;&#5;&#5;&#5;        &#5;       &#5;     &#5;       
|  &#5;&#5; \      | |     | |   (&#5;)      
| |  | | &#5;&#5;&#5; | |&#5; &#5;&#5; | |&#5;&#5;  &#5; &#5; &#5;&#5;  
| |  | |/ &#5; \| | '&#5; \| '&#5; \| | '&#5; \ 
| |&#5;&#5;| | (&#5;) | | |&#5;) | | | | | | | |
|&#5;&#5;&#5;&#5;&#5;/ \&#5;&#5;&#5;/|&#5;| .&#5;&#5;/|&#5;| |&#5;|&#5;|&#5;| |&#5;|
		| |                  
		|&#5;|                  

rtk&#5;btusb: btusb&#5;flush add delay 
rtk&#5;btusb: btusb&#5;close
rtk&#5;btcoex: Close BTCOEX
rtk&#5;btcoex: release udp socket
rtk&#5;btcoex: -x

<b>Please press Enter to activate this console. ~ #</b> 
~ # ====================HttpServer&#5;FUN====================
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$      DLink Project    8010LH     $$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
</pre>

By hitting enter, we are granted access to a shell, `/bin/sh`, running on the camera! We're able to explore the filesystem of the camera.

```bash
~ # ls
bin      etc      lib      mnt      overlay  rom      sys      usr
dev      init     media    mydlink  proc     root     tmp      var
~ # 
```

In part 2, we'll explore more of the camera file system and see what interesting things we can find.
