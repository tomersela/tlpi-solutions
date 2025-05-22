# Further Analysis

Welcome to the endless journey of checking why things work the way they are. And on this episode, we'll try to understand why the SUID clearing behavior changes per file system.

I'm going to check ext4 vs. 9p.
And I'll test append (`$ echo x >> mysleep`) and trunacte (`$ echo x > mysleep`) on both file systems.

As a start, I used [strace](https://man7.org/linux/man-pages/man1/strace.1.html) for checking the system calls made by each modification method:


We can see that when we use the shell for appending mysleep, there's an `openat` system call for the `mysleep` file with the O_APPEND flag:
```
$ strace -e openat,chmod bash -c 'echo hi >> mysleep'
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libtinfo.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/dev/tty", O_RDWR|O_NONBLOCK) = 3
openat(AT_FDCWD, "/usr/lib/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/usr/lib/aarch64-linux-gnu/gconv/gconv-modules.cache", O_RDONLY) = 3
openat(AT_FDCWD, "mysleep", O_WRONLY|O_CREAT|O_APPEND, 0666) = 3
+++ exited with 0 +++
```

After running this command on a 9p fs, the set-user-ID permission bit is still set:
```
$ ls -l --block-size=1 mysleep 
-rwsr-xr-x 1 debian debian 68435 May  6 23:20 mysleep
```
On ext4 this bit gets cleared.

The file size changed from 68442 to 68435, which confirms the modiciation took place.

When changing the existing content of the file, there's an `openat` system call for the `mysleep` file with the `O_TRUNC` flag:
```
$ strace -e openat,chmod bash -c 'echo hi > mysleep' 
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libtinfo.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/dev/tty", O_RDWR|O_NONBLOCK) = 3
openat(AT_FDCWD, "/usr/lib/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/usr/lib/aarch64-linux-gnu/gconv/gconv-modules.cache", O_RDONLY) = 3
openat(AT_FDCWD, "mysleep", O_WRONLY|O_CREAT|O_TRUNC, 0666) = 3
+++ exited with 0 +++
```

For that case set-user-ID permission bit is cleared on a 9p fs:
```
$ ls -l --block-size=1 mysleep 
-rwxr-xr-x 1 debian debian 3 May  6 23:13 mysleep
```
On ext4 this bit gets cleared.

So the SUID bit modification is different, but the system calls made are identical (as expected).


| File system | open flag | SUID cleared? |
| ----------- | --------  | ------------- |
| **ext4**    | O_APPEND  | YES           |
| **ext4**    | O_TRUNC   | YES           |
| **9p**      | O_APPEND  | NO            |
| **9p**      | O_TRUNC   | YES           |

The issue must be somewhere between the kernel to the 9p server on the host system.

## Is it macos fault?
Before going into crazy stuff, I wanted to check if it's macos that affects this behavior.
Maybe that's how [APFS](https://en.wikipedia.org/wiki/Apple_File_System) works? And since we're mounting a folder on macos into the Linux VM, it might explain why the behavior is different.

I did the same experiment on macos:
```
$ cp /bin/sleep ./mysleep
$ chmod u+sx ./mysleep
$ ls -l ./mysleep
.rws------@ 101k tomer.sela 11 May 21:19  ./mysleep
$ echo x >> mysleep
$ ls -l ./mysleep
.rwx------@ 101k tomer.sela 11 May 21:27  ./mysleep
```

mac play by the rules. so it's not it.

But then, another question popped out - If macos clears the SUID bit, how come the bit remains when appending a file from the linux machine?

I then realized, macos and the linux VM keeps the file attributes in different places!
Runing the following on the Linux VM gives:
```
$ cp /bin/sleep mysleep
$ chmod u+s mysleep
$ ls -l mysleep
-rwsr-xr-x 1 debian debian 68432 May 11 21:32 mysleep
```
When listing the file from macos, on the same folder that's mounted in the VM:
```
$ ls -l mysleep
.rw-------@ 68k tomer.sela 11 May 21:32  ./mysleep
```

Linux maintains its own value of file attributes of the 9p mount somewhere else.

And with that, we ruled out macos as responsible for the SUID behavior. It is somewhere in the Kernel or the QEMU 9p server.

## mount config
Running the mount command shows another interesting detail - We're using [9P VirtIO transport](https://www.linux-kvm.org/page/9p_virtio)

```
$ mount | grep 9p

share on /mnt/macos type 9p (rw,relatime,sync,dirsync,access=client,trans=virtio,_netdev)
```

While searching around about "Virtio" I found this page - https://wiki.qemu.org/Documentation/9p <br/>
It confirms this behavior and shed more light of the implementation:
> Without these assumed permissions, it would nearly be impossible to run any useful service on guest side ontop of a 9pfs filesystem. The QEMU binary on the host system however is usually not running as privileged user for security reasons, so the 9pfs server can actually not do all those things on the file system it has access to on host side.

> For that reason the "local" driver supports remapping of file permissions and owners. So when the "remap" driver option of the "local" driver is used (like it's usually the case on a production system), then the "local" driver pretends to the guest system it could do all those things, but in reality it just maps things like permissions and owning users and groups as additional data on the filesystem, either as some hidden files, or as extended attributes (the latter being recommended) which are not directly exposed to the guest OS. With remapping enabled, you can actually run an entire guest OS

That explains it :) Checking the extended attributes of the file on the macos host turn in the truth:

```
$ xattr -l mysleep
com.apple.provenance:
com.apple.quarantine: 0082;6820ed56;QEMUHelper;
user.virtfs.gid: �
user.virtfs.mode: �
user.virtfs.uid: �
```
Virtfs remaps the file attributes to extended attributes.


## Diving into 9p
As I couldn't find any documentation explaining the behavior, I wanted to check for myself where it really happens - the Kernel.

But first, we need to check the Kernel version, since a different one might have different results.
```
$ uname -r
6.1.55
```

Under the Linux Kernel code, there's a folder called /fs/9p where the 9p FS is defined.

I happen to know that the VFS is the abstraction layer for accessing different file systems by the Kernel, so /fs/9p/vfs_file.c seem to be a good bet.

There's a struct down there for the implementation of each operation:
```C
const struct file_operations v9fs_file_operations_dotl = {
	.llseek = generic_file_llseek,
	.read_iter = v9fs_file_read_iter,
	.write_iter = v9fs_file_write_iter,
	.open = v9fs_file_open,
	.release = v9fs_dir_release,
	.lock = v9fs_file_lock_dotl,
    ...
};
```

We're particularly interested in the file open action, implemented by the `v9fs_file_open` function.

There's a debug statement under this function:
```C
	p9_debug(P9_DEBUG_VFS, "inode: %p file: %p\n", inode, file);
```

There's also a call to the function `p9_client_open(fid, omode)` which seem to be interesting.
And under `p9_client_open` there's another debug statement showing the file mode (attributes):
```C
	p9_debug(P9_DEBUG_9P, ">>> %s fid %d mode %d\n",
		 p9_is_proto_dotl(clnt) ? "TLOPEN" : "TOPEN", fid->fid, mode);
```

This one can be useful to debug what mode is passed when we open the file. maybe something is lost in translation.


### 9p logs
dmesg or journalctl can be used to observe system logs.
```
$ dmesg | grep 9p
```
 or
```
journalctl -k | grep 9p
```

Looking at the system logs didn't show much. None of the debug logs where shown.

I wish turning on debug mode for 9p was as easy as remounting the drive with debug config set.
But having debug working requires rebuilding the Kernel with the `CONFIG_NET_9P_DEBUG=y` config option.

Verifying if the Kernel was compiled with that option is easy:
```
$ sudo cat /sys/kernel/debug/dynamic_debug/control | grep fs/9p
```
If that command prints nothing, than 9p debug isn't enabled.

#### Rebuild 9p Kernel Modules with Debugging

Finding out how to rebuild the kernel and modifying the config took longer than shown here. I'm including here the flow that eventually worked.

1. Install required packages
```
sudo apt update
sudo apt install build-essential bc libncurses-dev flex bison libssl-dev \
  libelf-dev dwarves git fakeroot kernel-wedge wget
```

2. Check Kernel version
```
uname -r
```
(For me it was 6.1.55)

3. Download matching Kernel version source
```
cd /usr/src
sudo mkdir linux-6.1.55
sudo chown $(whoami) linux-6.1.55
cd linux-6.1.55

wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.55.tar.xz
tar -xf linux-6.1.55.tar.xz
cd linux-6.1.55

sudo chown -R $(whoami):$(whoami) .
```

4. copy the current Kernel config
```
cp /boot/config-$(uname -r) .config
```

5. Modify the config file and regenerate dependencies
```
echo 'CONFIG_NET_9P_DEBUG=y' >> .config
make olddefconfig
```

6. Build and install the new kernel
```
make -j$(nproc) Image
sudo make modules_install
sudo make install
```

7. Reboot to the new Kernel
```
sudo reboot
```

8. Build the 9p modules
```
cd /usr/src/linux-6.1.55/linux-6.1.55
sudo make -j$(nproc) M=fs/9p
sudo make -j$(nproc) M=net/9p
```

9. Unmount the shared drive
```
sudo umount /mnt/macos
```

If it fails with "umount: /mnt/macos: target is busy.", try finding which process is using it with:
```
sudo fuser -vm /mnt/macos
```

10. uninstall the modules
```
sudo modprobe -r 9pnet_virtio 9pnet 9p
```

11. Install custom modules
```
sudo insmod net/9p/9pnet.ko
sudo insmod fs/9p/9p.ko
sudo insmod net/9p/9pnet_virtio.ko
```

If it doesn't work, build the full kernel and boot into it:
```
make -j$(nproc)
sudo make modules_install
sudo make install
sudo reboot
```

For long term use, we can run:
```
sudo cp fs/9p/9p.ko /lib/modules/$(uname -r)/extra/
sudo cp net/9p/9pnet.ko /lib/modules/$(uname -r)/extra/
sudo cp net/9p/9pnet_virtio.ko /lib/modules/$(uname -r)/extra/
sudo depmod
sudo modprobe 9pnet_virtio
sudo reboot
```

12. Remount with debug enabled
```
sudo bash -c 'echo "file fs/9p/* +p" > /sys/kernel/debug/dynamic_debug/control'
sudo mount -t 9p -o trans=virtio,debug=15 macos /mnt/macos
```

13. check logs
```
sudo dmesg --follow | grep 9p
```

We have logs!

#### Analyzing 9p logs
We now have logs, let's analyze what's happening on the 9pFS per file modification case:
##### Checking Append (echo x >> mysleep)
```
[ 1659.513498] 9pnet: -- v9fs_vfs_lookup (2635): dir: 00000000143dec54 dentry: (mysleep) 000000000d802866 flags: 0
[ 1659.513576] 9pnet: -- v9fs_fid_find (2635):  dentry: chapter_38 (000000006c916e7f) uid 1000 any 0
...
[ 1659.516514] 9pnet: -- v9fs_file_open (2635): inode: 00000000cdaec914 file: 0000000040a984a0
[ 1659.516519] 9pnet: -- v9fs_fid_find (2635):  dentry: mysleep (000000000d802866) uid 1000 any 0
[ 1659.516522] 9pnet: (00002635) >>> TWALK fids 7,8 nwname 0d wname[0] (null)
[ 1659.516527] 9pnet: (00002635) >>> size=17 type: 110 tag: 0
[ 1659.517904] 9pnet: (00002635) <<< size=9 type: 111 tag: 0
[ 1659.517911] 9pnet: (00002635) <<< RWALK nwqid 0:
[ 1659.517913] 9pnet: (00002635) >>> TLOPEN fid 8 mode 34113             <-- CHECK THIS OUT!
[ 1659.517917] 9pnet: (00002635) >>> size=15 type: 12 tag: 0
[ 1659.519944] 9pnet: (00002635) <<< size=24 type: 13 tag: 0
[ 1659.519948] 9pnet: (00002635) <<< RLOPEN qid 0.60b7978.6914f3c5 iounit 1f000
[ 1659.519977] 9pnet: (00002635) >>> TWRITE fid 8 offset 68432 count 2
[ 1659.519982] 9pnet: (00002635) >>> size=25 type: 118 tag: 0
[ 1659.520899] 9pnet: (00002635) <<< size=11 type: 119 tag: 0
[ 1659.520906] 9pnet: (00002635) <<< RWRITE count 2
...
```
Pay attention to `TLOPEN fid 8 mode 34113`

34113 (decimal) = 0x8541 (hex) = O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND | O_LARGEFILE

O_APPEND is there!

##### Checking Truncate (echo x > mysleep)
```
[ 1722.730260] 9pnet: -- v9fs_write_inode_dotl (79): v9fs_write_inode_dotl: inode 00000000c9ad58e6, writeback_fid 0000000000000000
[ 1737.787446] 9pnet: -- v9fs_vfs_lookup (2635): dir: 00000000143dec54 dentry: (mysleep) 000000005fb938b7 flags: 0
[ 1737.787480] 9pnet: -- v9fs_fid_find (2635):  dentry: chapter_38 (000000006c916e7f) uid 1000 any 0
[ 1737.787534] 9pnet: (00002635) >>> TWALK fids 4,7 nwname 1d wname[0] mysleep
...
[ 1737.791913] 9pnet: -- v9fs_file_open (2635): inode: 000000003e8acc03 file: 0000000018739f12
[ 1737.791922] 9pnet: -- v9fs_fid_find (2635):  dentry: mysleep (000000005fb938b7) uid 1000 any 0
[ 1737.791926] 9pnet: (00002635) >>> TWALK fids 7,8 nwname 0d wname[0] (null)
[ 1737.791933] 9pnet: (00002635) >>> size=17 type: 110 tag: 0
[ 1737.792602] 9pnet: (00002635) <<< size=9 type: 111 tag: 0
[ 1737.792608] 9pnet: (00002635) <<< RWALK nwqid 0:
[ 1737.792610] 9pnet: (00002635) >>> TLOPEN fid 8 mode 33089             <-- CHECK THIS OUT!
[ 1737.792613] 9pnet: (00002635) >>> size=15 type: 12 tag: 0
[ 1737.793831] 9pnet: (00002635) <<< size=24 type: 13 tag: 0
[ 1737.793834] 9pnet: (00002635) <<< RLOPEN qid 0.60b7978.6914f628 iounit 1f000
[ 1737.793845] 9pnet: -- v9fs_xattr_get (2635): name = security.capability value_len = 0
[ 1737.793850] 9pnet: -- v9fs_fid_find (2635):  dentry: mysleep (000000005fb938b7) uid 1000 any 0
[ 1737.793854] 9pnet: (00002635) >>> TXATTRWALK file_fid 7, attr_fid 9 name security.capability
[ 1737.793858] 9pnet: (00002635) >>> size=36 type: 30 tag: 0
[ 1737.794283] 9pnet: (00002635) <<< size=11 type: 7 tag: 0
[ 1737.794287] 9pnet: (00002635) <<< RLERROR (-95)
[ 1737.794290] 9pnet: -- v9fs_fid_xattr_get (2635): p9_client_attrwalk failed -95
[ 1737.794295] 9pnet: -- v9fs_vfs_setattr_dotl (2635): 
[ 1737.794297] 9pnet: (00002635) >>> TSETATTR fid 8
[ 1737.794299] 9pnet: (00002635)     valid=69 mode=81ed uid=-1 gid=-1 size=0
[ 1737.794302] 9pnet: (00002635)     atime_sec=0 atime_nsec=0
[ 1737.794304] 9pnet: (00002635)     mtime_sec=0 mtime_nsec=0
[ 1737.794327] 9pnet: (00002635) >>> size=67 type: 26 tag: 0
[ 1737.798202] 9pnet: (00002635) <<< size=7 type: 27 tag: 0
[ 1737.798205] 9pnet: (00002635) <<< RSETATTR fid 8
[ 1737.798229] 9pnet: (00002635) >>> TWRITE fid 8 offset 0 count 2
[ 1737.798233] 9pnet: (00002635) >>> size=25 type: 118 tag: 0
[ 1737.798422] 9pnet: (00002635) <<< size=11 type: 119 tag: 0
[ 1737.798427] 9pnet: (00002635) <<< RWRITE count 2
...
```

This one is interesting

TLOPEN fid 8 mode 33089

33089 = 0x8141 = O_WRONLY | O_CREAT | O_NOCTTY | O_LARGEFILE

No O_TRUNC! looks like that was handled somewhere in the kernal. perhaps the in the VFS layer.

And still, it doesn't explain why the behavior is different for O_APPEND.

Also shown in the log above is the `v9fs_vfs_setattr_dotl` function logs, specifically this one:
```
[ 1737.794299] 9pnet: (00002635)     valid=69 mode=81ed uid=-1 gid=-1 size=0
```

here "mode" represents the file attributes:


**0x81ed =**
| Portion             | Hex      | Octal     | Meaning                      |
| ------------------- | -------- | --------- | ---------------------------- |
| **File-type bits**  | `0x8000` | `0100000` | `S_IFREG` – a *regular file* |
| **Permission bits** | `0x01ed` | `0000755` | `rwxr-xr-x`                  |

The permission bits show that the SUID bit is cleared.

#### Observations
* The logs show that we're using the dotl (9P2000.L) version of the 9p protocol. this can be useful later when we look at the FS code.
* for the truncate case, we see openat(..., O_TRUNC) in strace. But the resulting TLOPEN mode does not include 0x200 (O_TRUNC) The kernel still clears the SUID bit, because that behavior is handled before the 9p request is constructed
* There's a call to `v9fs_vfs_setattr_dotl` on the O_TRUNC case but not in the O_APPEND case.

This narrow down our search for where the issue is.

Since we see the 9p protocol sets the security attributes for O_TRUNC, and not for O_APPEND, it means the issue isn't with the 9p server QEMU.

It's either VFS or 9p.


## Back to the Kernel

We have more focus now. we narrowed down the places we should look.

Both ext4 and 9p filesystems are integrated with the VFS in Linux - ext4 as a standard local kernel filesystem, and 9p as a remote filesystem implementation

https://www.kernel.org/doc/html/latest/filesystems/vfs.html
> The Virtual File System (also known as the Virtual Filesystem Switch) is the software layer in the kernel that provides the filesystem interface to userspace programs. It also provides an abstraction within the kernel which allows different filesystem implementations to coexist.

At this stage, I started explore the calls that goes through the Kernel.

There's a cool tool called `perf` (and `trace-cmd`) that can be used to profile the Kernel.

Running the following will record system calls and deeper function calls within the kernel for the open syscall:
```
sudo trace-cmd record                      \      
     -p function_graph                     \
     -g __arm64_sys_openat -g __arm64_sys_openat2 \
     --user debian                          \
     -- /bin/sh -c 'echo x > /mnt/macos/tlpi-solutions/chapter_38/mysleep'
```

Then we can write this recording into a file:
```
sudo trace-cmd report > trace_report_9p_open_trunc.txt
```


 [`do_sys_openat2` (static)](https://elixir.bootlin.com/linux/v6.1.55/source/fs/open.c#L1302) > [`do_filp_open`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/namei.c#L3733) > [`path_openat`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/namei.c#L3694) > [calls](https://elixir.bootlin.com/linux/v6.1.55/source/fs/namei.c#L3714) [`do_open` (static)](https://elixir.bootlin.com/linux/v6.1.55/source/fs/namei.c#L3513) :
```C
/*
 * Handle the last step of open()
 */
static int do_open(struct nameidata *nd,
		   struct file *file, const struct open_flags *op)
{
	struct user_namespace *mnt_userns;
	int open_flag = op->open_flag;
	bool do_truncate;
	int acc_mode;
	int error;

	if (!(file->f_mode & (FMODE_OPENED | FMODE_CREATED))) {
		error = complete_walk(nd);
		if (error)
			return error;
	}
	if (!(file->f_mode & FMODE_CREATED))
		audit_inode(nd->name, nd->path.dentry, 0);
	mnt_userns = mnt_user_ns(nd->path.mnt);
	if (open_flag & O_CREAT) {
		if ((open_flag & O_EXCL) && !(file->f_mode & FMODE_CREATED))
			return -EEXIST;
		if (d_is_dir(nd->path.dentry))
			return -EISDIR;
		error = may_create_in_sticky(mnt_userns, nd,
					     d_backing_inode(nd->path.dentry));
		if (unlikely(error))
			return error;
	}
	if ((nd->flags & LOOKUP_DIRECTORY) && !d_can_lookup(nd->path.dentry))
		return -ENOTDIR;

	do_truncate = false;
	acc_mode = op->acc_mode;
	if (file->f_mode & FMODE_CREATED) {
		/* Don't check for write permission, don't truncate */
		open_flag &= ~O_TRUNC;
		acc_mode = 0;
	} else if (d_is_reg(nd->path.dentry) && open_flag & O_TRUNC) {
		error = mnt_want_write(nd->path.mnt);
		if (error)
			return error;
		do_truncate = true;
	}
	error = may_open(mnt_userns, &nd->path, acc_mode, open_flag);
	if (!error && !(file->f_mode & FMODE_OPENED))
		error = vfs_open(&nd->path, file);
	if (!error)
		error = ima_file_check(file, op->acc_mode);
	if (!error && do_truncate)
		error = handle_truncate(mnt_userns, file);
	if (unlikely(error > 0)) {
		WARN_ON(1);
		error = -EINVAL;
	}
	if (do_truncate)
		mnt_drop_write(nd->path.mnt);
	return error;
}
```
Here we can see logic that will cause `handle_truncate` to run only if the `O_TRUNC` mode was set.

[handle_truncate (static)](https://elixir.bootlin.com/linux/v6.1.55/source/fs/namei.c#L3206) calls [`do_truncate`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/namei.c#L3206) :
```C
int do_truncate(struct mnt_idmap *idmap, struct dentry *dentry,
		loff_t length, unsigned int time_attrs, struct file *filp)
{
	int ret;
	struct iattr newattrs;

	/* Not pretty: "inode->i_size" shouldn't really be signed. But it is. */
	if (length < 0)
		return -EINVAL;

	newattrs.ia_size = length;
	newattrs.ia_valid = ATTR_SIZE | time_attrs;
	if (filp) {
		newattrs.ia_file = filp;
		newattrs.ia_valid |= ATTR_FILE;
	}

	/* Remove suid, sgid, and file capabilities on truncate too */
	ret = dentry_needs_remove_privs(idmap, dentry);
	if (ret < 0)
		return ret;
	if (ret)
		newattrs.ia_valid |= ret | ATTR_FORCE;

	inode_lock(dentry->d_inode);
	/* Note any delegations or leases have already been broken: */
	ret = notify_change(idmap, dentry, &newattrs, NULL);
	inode_unlock(dentry->d_inode);
	return ret;
}
```
There's a [call](https://elixir.bootlin.com/linux/v6.14.6/source/fs/open.c#L57) to `dentry_needs_remove_privs`.

[`dentry_needs_remove_privs`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L1998) either return a negative number for error, or flags for security attributes to be removed.
Within [`dentry_needs_remove_privs`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L1998) there's a call to [`setattr_should_drop_suidgid`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/attr.c#L67) which always clears SUID:
```C
int setattr_should_drop_suidgid(struct user_namespace *mnt_userns,
				struct inode *inode)
{
	umode_t mode = inode->i_mode;
	int kill = 0;

	/* suid always must be killed */
	if (unlikely(mode & S_ISUID))
		kill = ATTR_KILL_SUID;

	kill |= setattr_should_drop_sgid(mnt_userns, inode);

	if (unlikely(kill && !capable(CAP_FSETID) && S_ISREG(mode)))
		return kill;

	return 0;
}
```

Later, `do_truncate` calls [notify_change](https://elixir.bootlin.com/linux/v6.1.55/source/fs/open.c#L57). `notify_change` executes the actual attibute update (will later call `v9fs_vfs_setattr_dotl` down the stack:

- `notify_change` [dispatches](https://elixir.bootlin.com/linux/v6.1.55/source/fs/attr.c#L499) the [`setattr` property of  `struct inode_operations v9fs_dir_inode_operations_dotl`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/9p/vfs_inode_dotl.c#L984), thuse calling [`v9fs_vfs_setattr_dotl`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/9p/vfs_inode_dotl.c#L539)

The main thing here - VFS does the work for us. Whenver we open a file with `O_TRUNC`, VFS will call `handle_truncate` and that will clear the SUID attribute and ask the underlying filesystem to modify the permission bits.

This is both true to ext4, and 9p.

As said, it only happen in the case of the `O_TRUNC` flag.

So what clear the SUID bit in the case of `O_APPEND` in ext4? and why isn't the same happens in the case of `O_APPEND` in 9p?

### What happen when we append a file in ext4?
I ran `trace-cmd` with that open syscall, but nothing related to "uid" or setting the attributes showed up.

But the operation we're running also do some `write`, so I added write calls to the functions to be traced:
```
sudo trace-cmd record \        
     -p function_graph \
     -g __arm64_sys_openat -g __arm64_sys_openat2 -g __arm64_sys_write -g __arm64_sys_pwrite64 \
     --user debian  \
     -- /bin/sh -c 'echo x >> /home/debian/dev/mysleep'
```

modifying SUID is done by the ext4 implementation and not VFS:
[`__arm64_sys_write`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/read_write.c#L646) <br/>
 -> calls [`ksys_write`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/read_write.c#L626) <br/>
 -> calls [`vfs_write`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/read_write.c#L564) <br/>
 -> [dispatches](https://elixir.bootlin.com/linux/v6.1.55/source/fs/read_write.c#L583) `write_iter` of [`struct file_operations ext4_file_operations`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/ext4/file.c#L934) which points [`ext4_file_write_iter`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/ext4/file.c#L686) <br/>
 -> [calls](https://elixir.bootlin.com/linux/v6.1.55/source/fs/ext4/file.c#L700) [`ext4_buffered_write_iter`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/ext4/file.c#L270) <br/>
 -> [calls](https://elixir.bootlin.com/linux/v6.1.55/source/fs/ext4/file.c#L280) [`ext4_write_checks`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/ext4/file.c#L256) (static) <br/>
 -> [calls](https://elixir.bootlin.com/linux/v6.1.55/source/fs/ext4/file.c#L264) [`file_modified`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L2189) <br/>
 -> [calls](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L2191) [`file_modified_flags`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L2152):

 ```C
 static int file_modified_flags(struct file *file, int flags)
{
	int ret;
	struct inode *inode = file_inode(file);
	struct timespec64 now = current_time(inode);

	/*
	 * Clear the security bits if the process is not being run by root.
	 * This keeps people from modifying setuid and setgid binaries.
	 */
	ret = __file_remove_privs(file, flags);
	if (ret)
		return ret;

	if (unlikely(file->f_mode & FMODE_NOCMTIME))
		return 0;

	ret = inode_needs_update_time(inode, &now);
	if (ret <= 0)
		return ret;
	if (flags & IOCB_NOWAIT)
		return -EAGAIN;

	return __file_update_time(file, &now, ret);
}
```
 -> calls [`__file_remove_privs`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L2030) (static):

 ```C
 static int __file_remove_privs(struct file *file, unsigned int flags)
{
	struct dentry *dentry = file_dentry(file);
	struct inode *inode = file_inode(file);
	int error = 0;
	int kill;

	if (IS_NOSEC(inode) || !S_ISREG(inode->i_mode))
		return 0;

	kill = dentry_needs_remove_privs(file_mnt_user_ns(file), dentry);
	if (kill < 0)
		return kill;

	if (kill) {
		if (flags & IOCB_NOWAIT)
			return -EAGAIN;

		error = __remove_privs(file_mnt_user_ns(file), dentry, kill);
	}

	if (!error)
		inode_has_no_xattr(inode);
	return error;
}
 ```
we already covered `dentry_needs_remove_privs` in the open syscall flow. It tells us which attributes should be removed (e.g. SUID, SGID)
[`__remove_privs`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L2017) [calls](https://elixir.bootlin.com/linux/v6.1.55/source/fs/inode.c#L2027) [`notify_change`](https://elixir.bootlin.com/linux/v6.1.55/source/fs/attr.c#L381) which fires the actual attributes update with the underlying filesystem.


### Observation
The main conclusion from this - It's ext4 implementation that clears the SUID bit in the append flow.

When running the same analysis for the 9p append scenario, there's no call to `notify_change` in the write syscall stack. That probably means that clearing SUID wasn't implemented for 9p.

### Can we patch this?
But what if we wanted 9p to clear the SUID bit in the case of append?

I tried and modified `v9fs_file_write_iter` with a call to `file_modified` to trigger the SUID (and SGID) clearing logic.

`fs/9p/vfs_file.c`
```diff
@@ -400,6 +400,8 @@
 	if (retval <= 0)
 		return retval;
 
+	file_modified(file);
+
 	origin = iocb->ki_pos;
 	retval = p9_client_write(file->private_data, iocb->ki_pos, from, &err);
 	if (retval > 0) {
```

Now let's test this.

#### Rebuild 9p modules
1. Unmount 9p drive
```
sudo umount /mnt/macos
```
If it's in use, you can find out what's using it with:
```
sudo fuser -vm /mnt/macos
```
2. Uninstall current 9p module
```
sudo modprobe -r 9p
```

3. From the linux kernel source directory, rebuild the 9p module
```
make M=net/9p modules
make M=fs/9p modules
```

4. Load dependent modules and re-install
```
sudo modprobe fscache
sudo modprobe netfs
sudo insmod net/9p/9pnet.ko
sudo insmod fs/9p/9p.ko
sudo insmod net/9p/9pnet_virtio.ko
```

5. Mount the drive back
```
sudo mount -t 9p -o trans=virtio,debug=15 share /mnt/macos
```

#### Testing our patch
```
$ cp /bin/sleep mysleep && chmod u+s,+x mysleep
$ ls -l mysleep 
-rwsr-xr-x 1 debian debian 68432 May 21 02:51 mysleep
$ echo x >> ./mysleep
$ ls -l mysleep      
-rwxr-xr-x 1 debian debian 68434 May 21 02:53 mysleep
```

It worked!

We can see the file is modified (size changed) and the SUID bit is cleared.
