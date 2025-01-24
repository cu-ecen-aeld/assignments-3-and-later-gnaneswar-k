#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # DONE: Add your kernel build steps here
    ## Deep clean the kernel build tree.
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper

    ## Setup the default config.
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig

    ## Build the kernel image.
    ## The "-j" flag essentially specifies the number of parallel builds to run.
    make -j3 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all

    ## Build modules. 
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

    ## Build device tree.
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cp -v ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# DONE: Create necessary base directories
mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log 

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone https://git.busybox.net/busybox
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # DONE:  Configure busybox
    echo "Configuring busybox"
    make distclean
    make defconfig
else
    cd busybox
fi

# DONE: Make and install busybox
echo "Building busybox"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

# DONE: Add library dependencies to rootfs

## Get the compiler sysroot path.
SYSROOT_PATH=$(${CROSS_COMPILE}gcc --print-sysroot)

echo "Copying dependencies"
## Copy the program interpreter.
cp -v ${SYSROOT_PATH}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib/
## Copy the libraries.
cp -v ${SYSROOT_PATH}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64/
cp -v ${SYSROOT_PATH}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64/
cp -v ${SYSROOT_PATH}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64/

# DONE: Make device nodes
echo "Creating device nodes"
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 600 ${OUTDIR}/rootfs/dev/console c 5 1

echo "Setting up finder related utilities"
cd ${FINDER_APP_DIR}

# DONE: Clean and build the writer utility
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# DONE: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -v ${FINDER_APP_DIR}/writer ${OUTDIR}/rootfs/home/
cp -v ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home/
cp -v ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home/
cp -v ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home/
cp -v -r ${FINDER_APP_DIR}/../conf ${OUTDIR}/rootfs/home/

echo "Final setup"
# DONE: Chown the root directory
cd ${OUTDIR}/rootfs
sudo chown -R root:root *

# DONE: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip -f initramfs.cpio

