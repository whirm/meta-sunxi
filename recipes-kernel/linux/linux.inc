DESCRIPTION = "Linux Kernel"
SECTION = "kernel"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

INC_PR = "r0"

inherit kernel siteinfo

# Enable OABI compat for people stuck with obsolete userspace
ARM_KEEP_OABI ?= "0"

# Set the verbosity of kernel messages during runtime
# You can define CMDLINE_DEBUG in your local.conf or distro.conf to override this behaviour
CMDLINE_DEBUG ?= "loglevel=3"

# Kernel bootlogo is distro-specific (default is OE logo).
# Logo resolution (qvga, vga, ...) is machine-specific.
LOGO_SIZE ?= '${@base_conditional("MACHINE_GUI_CLASS", "bigscreen", "vga", "qvga", d)}'
# To use this, add file://${LOGO_SIZE}/logo_linux_clut224.ppm.bz2 or similar
# to your kernel recipe, and then structure your logos for each resolution
# accordingly.

LOCALVERSION ?= ""

#kernel_conf_variable CMDLINE "\"${CMDLINE} ${CMDLINE_DEBUG}\""
kernel_conf_variable() {
    CONF_SED_SCRIPT="$CONF_SED_SCRIPT /CONFIG_$1[ =]/d;"
    if test "$2" = "n"
    then
        echo "# CONFIG_$1 is not set" >> ${B}/.config
    else
        echo "CONFIG_$1=$2" >> ${B}/.config
    fi
}

do_configure_prepend() {
    echo "" > ${B}/.config

    oe_runmake -C ${S} O=${B} sunxi_defconfig
    cp .config .config.base

    CONF_SED_SCRIPT=""

    #
    # logo support, if you supply logo_linux_clut224.ppm in SRC_URI, then it's going to be used
    #
    if [ -e ${WORKDIR}/logo_linux_clut224.ppm ]; then
        install -m 0644 ${WORKDIR}/logo_linux_clut224.ppm drivers/video/logo/logo_linux_clut224.ppm
        kernel_conf_variable LOGO y
        kernel_conf_variable LOGO_LINUX_CLUT224 y
    fi

    #
    # oabi / eabi support
    #
    kernel_conf_variable AEABI y
    if [ "${ARM_KEEP_OABI}" = "1" ] ; then
        kernel_conf_variable OABI_COMPAT y
    else
        kernel_conf_variable OABI_COMPAT n
    fi

    # When enabling thumb for userspace we also need thumb support in the kernel
    if [ "${ARM_INSTRUCTION_SET}" = "thumb" ] ; then
        kernel_conf_variable ARM_THUMB y
    fi

    kernel_conf_variable CMDLINE "\"${CMDLINE} ${CMDLINE_DEBUG}\""

    kernel_conf_variable LOCALVERSION "\"${LOCALVERSION}\""
    kernel_conf_variable LOCALVERSION_AUTO n

    kernel_conf_variable SYSFS_DEPRECATED n
    kernel_conf_variable SYSFS_DEPRECATED_V2 n
    kernel_conf_variable HOTPLUG y
    kernel_conf_variable UEVENT_HELPER_PATH \"\"
    kernel_conf_variable UNIX y
    kernel_conf_variable SYSFS y
    kernel_conf_variable PROC_FS y
    kernel_conf_variable TMPFS y
    kernel_conf_variable INOTIFY_USER y
    kernel_conf_variable SIGNALFD y
    kernel_conf_variable TMPFS_POSIX_ACL y
    kernel_conf_variable BLK_DEV_BSG y
    kernel_conf_variable DEVTMPFS y
    kernel_conf_variable DEVTMPFS_MOUNT y

    if [ "${KERNEL_ENABLE_SIMPLEFB}" = "1" ] ; then
        kernel_conf_variable FB y
        kernel_conf_variable FB_SIMPLE y
        kernel_conf_variable FRAMEBUFFER_CONSOLE y
    fi

    if [ "${KERNEL_ENABLE_SYSTEMD}" = "1" ] ; then
	# Kernel Config Options for systemd:
        kernel_conf_variable DEVTMPFS y
        kernel_conf_variable CGROUPS y
        kernel_conf_variable INOTIFY_USER y
        kernel_conf_variable SIGNALFD y
        kernel_conf_variable TIMERFD y
        kernel_conf_variable EPOLL y
        kernel_conf_variable NET y
        kernel_conf_variable SYSFS y
        kernel_conf_variable PROC_FS y
        kernel_conf_variable FHANDLE y

        kernel_conf_variable DMIID y

        kernel_conf_variable BLK_DEV_BSG y

        kernel_conf_variable NET_NS y
        kernel_conf_variable DEVPTS_MULTIPLE_INSTANCES y

        kernel_conf_variable IPV6 y
        kernel_conf_variable AUTOFS4_FS y
        kernel_conf_variable TMPFS_XATTR y
        kernel_conf_variable TMPFS_POSIX_ACL y
        kernel_conf_variable EXT2_POSIX_ACL y
        kernel_conf_variable EXT3_POSIX_ACL y
        kernel_conf_variable EXT4_POSIX_ACL y
        kernel_conf_variable BTRFS_FS_POSIX_ACL y
        kernel_conf_variable SQUASHFS_POSIX_ACL y
        kernel_conf_variable XFS_POSIX_ACL y
        kernel_conf_variable SECCOMP y

        kernel_conf_variable CGROUP_SCHED y
        kernel_conf_variable FAIR_GROUP_SCHED y

        kernel_conf_variable CFS_BANDWIDTH y

        kernel_conf_variable SCHEDSTATS y
        kernel_conf_variable SCHED_DEBUG y

        #Legacy hotplug slows down the system and confuses udev:
        kernel_conf_variable UEVENT_HELPER_PATH ""
        kernel_conf_variable FW_LOADER_USER_HELPER n

	# Udev will fail to work with the legacy layout:
        kernel_conf_variable SYSFS_DEPRECATED n
        kernel_conf_variable AUDIT n
    fi

    # Newer inits like systemd need cgroup support
    if [ "${KERNEL_ENABLE_CGROUPS}" = "1" ] ; then
        kernel_conf_variable CGROUP_SCHED y
        kernel_conf_variable CGROUPS y
        kernel_conf_variable CGROUP_NS y
        kernel_conf_variable CGROUP_FREEZER y
        kernel_conf_variable CGROUP_DEVICE y
        kernel_conf_variable CPUSETS y
        kernel_conf_variable PROC_PID_CPUSET y
        kernel_conf_variable CGROUP_CPUACCT y
        kernel_conf_variable RESOURCE_COUNTERS y
    fi

    # Squashfs support
    if [ "${KERNEL_ENABLE_SQUASHFS}" = "1" ] ; then
        kernel_conf_variable SQUASHFS y
        kernel_conf_variable SQUASHFS_FILE_CACHE y
        kernel_conf_variable SQUASHFS_DECOMP_SINGLE y
        kernel_conf_variable SQUASHFS_XATTR y
        kernel_conf_variable SQUASHFS_ZLIB y
        kernel_conf_variable SQUASHFS_LZO y
        kernel_conf_variable SQUASHFS_XZ y
    fi

    #
    # root-over-nfs-over-usb-eth support. Limited, but should cover some cases.
    # Enable this by setting a proper CMDLINE_NFSROOT_USB.
    #
    if [ ! -z "${CMDLINE_NFSROOT_USB}" ]; then
        bbnote "Configuring the kernel for root-over-nfs-over-usb-eth with CMDLINE ${CMDLINE_NFSROOT_USB}"
        kernel_conf_variable INET y
        kernel_conf_variable IP_PNP y
        kernel_conf_variable USB_GADGET y
        kernel_conf_variable USB_GADGET_SELECTED y
        kernel_conf_variable USB_ETH y
        kernel_conf_variable NFS_FS y
        kernel_conf_variable ROOT_NFS y
        kernel_conf_variable CMDLINE \"${CMDLINE_NFSROOT_USB} ${CMDLINE_DEBUG}\"
    fi

    # Enable/disable extra config flags
    for FLAG in ${KERNEL_EXTRA_ENABLE}; do
        kernel_conf_variable ${FLAG} y
    done
    for FLAG in ${KERNEL_EXTRA_MODULE}; do
        kernel_conf_variable ${FLAG} m
    done
    for FLAG in ${KERNEL_EXTRA_DISABLE}; do
        kernel_conf_variable ${FLAG} n
    done

    sed -e "${CONF_SED_SCRIPT}" \
    < .config.base >>'${B}/.config'

    yes '' | oe_runmake -C ${S} O=${B} oldconfig
}

do_configure_append() {
    if test -e scripts/Makefile.fwinst ; then
        sed -i -e "s:-m0644:-m 0644:g" scripts/Makefile.fwinst
    fi
}

do_install_append() {
    oe_runmake headers_install INSTALL_HDR_PATH=${D}${exec_prefix}/src/linux-${KERNEL_VERSION} ARCH=$ARCH
}

PACKAGES =+ "kernel-headers"
FILES_kernel-headers = "${exec_prefix}/src/linux*"
