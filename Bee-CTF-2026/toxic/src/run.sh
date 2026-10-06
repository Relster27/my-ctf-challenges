#!/bin/sh

cd /app/

#python3 pow.py ask 0
python3 pow.py ask 31337
if [ $? -ne 0 ]; then
  exit 1
fi

qemu-system-x86_64 \
    -m 128M \
    -kernel ./bzImage \
    -initrd ./initramfs.cpio.gz \
    -append "console=ttyS0 quiet loglevel=0 kaslr pti=on oops=panic panic=-1" \
    -cpu qemu64,+smep,+smap \
    -smp 1 \
    -monitor /dev/null \
    -nographic \
    -no-reboot \
    -snapshot \
    -drive file=flag.txt,format=raw