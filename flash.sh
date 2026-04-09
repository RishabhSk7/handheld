#!/bin/bash
set -e

DEV=$1
IMAGES=buildroot/output/images

if [ -z "$DEV" ]; then
    echo "Usage: $0 /dev/sdX"
    exit 1
fi


echo "Flashing to $DEV..."
umount ${DEV}* 2>/dev/null || true

# Write full image (creates partition table with all 3 partitions)
echo "Writing full image..."
dd if=${IMAGES}/sdcard.img of=${DEV} bs=4M status=progress conv=fsync

# Re-inform kernel of partition changes
partprobe ${DEV}

# Format p3 only if not ext4
if ! blkid ${DEV}3 | grep -q ext4; then
    echo "Formatting data partition..."
    mkfs.ext4 ${DEV}3 -L data
else
    echo "Data partition exists, skipping format"
fi

echo "Done"
