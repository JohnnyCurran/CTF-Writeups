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
