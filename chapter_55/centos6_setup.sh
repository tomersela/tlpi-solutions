#!/bin/bash

# update yum to use vault repositories (CentOS 6 is EOL)
sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-Base.repo
sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-Base.repo

yum clean all
yum install -y gcc

# check if gcc installed successfully
if command -v gcc &> /dev/null; then
    echo "gcc installed"
    gcc --version
else
    echo "gcc installation failed"
    exit 1
fi

# compile the test program if it exists
if [ -f "mandatory_deadlock_test.c" ]; then
    gcc -o mandatory_deadlock_test mandatory_deadlock_test.c
    if [ $? -eq 0 ]; then
        echo "compilation complete"
    else
        echo "compilation failed"
    fi
else
    echo "mandatory_deadlock_test.c not found in current directory"
fi

# enable mandatory locking on root filesystem
mount -o remount,mand /

# verify mandatory locking is enabled
if mount | grep -q mand; then
    echo "mandatory locking enabled"
    mount | grep mand
else
    echo "creating test filesystem..."

    # create a test file and loop mount it
    dd if=/dev/zero of=/tmp/testfs.img bs=1M count=10 2>/dev/null
    if [ $? -eq 0 ]; then
        mkfs.ext2 /tmp/testfs.img >/dev/null 2>&1
        mkdir -p /mnt/test
        mount -o loop,mand /tmp/testfs.img /mnt/test
        if mount | grep -q mand; then
            echo "SUCCESS: test filesystem created at /mnt/test"
            echo "change to /mnt/test directory before running the test"
        fi
    fi
fi

echo "done!"