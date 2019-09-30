#!/usr/bin/env bash
cat /dev/zero | tr '\0' '\377' | dd iflag=count_bytes count=4M of=target.bin
dd conv=notrunc of=target.bin oflag=seek_bytes seek=$((0x0     )) if=stc.bin
dd conv=notrunc of=target.bin oflag=seek_bytes seek=$((0x300000)) if=data.bin
dd conv=notrunc of=target.bin oflag=seek_bytes seek=$((0x3FB000)) if=eeprom.bin
dd conv=notrunc of=target.bin oflag=seek_bytes seek=$((0x3FC000)) if=rfcal.bin
dd conv=notrunc of=target.bin oflag=seek_bytes seek=$((0x3FD000)) if=wifi.bin
