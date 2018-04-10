#!/bin/sh

#wget https://releases.linaro.org/components/toolchain/binaries/5.1-2015.08/arm-eabi/gcc-linaro-5.1-2015.08-x86_64_arm-eabi.tar.xz -O thing.tar.xz
#tar xf thing.tar.xz gcc-linaro-5.1-2015.08-x86_64_arm-eabi/
#mv gcc-linaro-5.1-2015.08-x86_64_arm-eabi /usr/local

make build

#make create-disk

make launch-qemu-no-gdb & ./tests/disk.sh & ./tests/inOut.sh & wait
