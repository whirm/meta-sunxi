setenv bootargs console=${console} console=tty1 root=/dev/mmcblk0p2 rootwait panic=10 ${extra}
load mmc 0:1 ${fdt_addr_r} ${fdtfile} || load mmc 0:1 ${fdt_addr_r} boot/${fdtfile}
load mmc 0:1 ${kernel_addr_r} zImage || load mmc 0:1 ${kernel_addr_r} boot/zImage || load mmc 0:1 ${kernel_addr_r} uImage || load mmc 0:1 ${kernel_addr_r} boot/uImage
bootz ${kernel_addr_r} - ${fdt_addr_r} || bootm ${kernel_addr_r} - ${fdt_addr_r}

#for location in zImage uImage boot/zImage boot/uImage; do;
#    echo "LOCATION" ${location};
#    if test -e mmc 0:1 ${location}; then;
#       echo "EXISTS";
#       load mmc 0:1 ${fdt_addr_r} ${prefix}${fdtfile};
#       load mmc 0:1 ${kernel_addr_r} ${prefix}${kimage};
#       if test ${kimage} = "zImage"; then;
#          echo "ZIMAGE";
#          bootz ${kernel_addr_r} - ${fdt_addr_r};
#       else;
#          echo "UIMAGE";
#          bootm ${kernel_addr_r} - ${fdt_addr_r};
#       fi;
#     else;
#       echo "NOT EXISTS";
#    fi;
#done;
