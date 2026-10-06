#!/bin/sh

cd /app

set -e

qemu-system-x86_64 \
    -m 128M \
    -kernel ./bzImage \
    -drive file=./rootfs.ext4,format=raw,if=virtio \
    -append "root=/dev/vda rw rootfstype=ext4 init=/sbin/init console=ttyS0 quiet loglevel=0 kaslr kpti=on oops=panic panic=-1" \
    -cpu qemu64,+smep,+smap \
    -smp 1 \
    -monitor /dev/null \
    -nographic \
    -no-reboot \
    -snapshot \
