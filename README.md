# Linux Kernel Development in action

[![Build Status]](https://travis-ci.org/keinohguchi/kernel-in-action)

Our beloved [LKD] and [LDD] in action, with the latest kernel.

- [Build](#build)
- [Load](#load)
  - [Process status](#process-status)
  - [Hello world](#hello-world)
  - [Scull](#scull)
  - [Sleepy](#sleepy)
  - [Scullp](#scullp)
  - [Sculld](#sculld)
- [Test](#test)
- [Unload](#unload)
- [Cleanup](#cleanup)
- [References](#references)

[Build Status]: https://travis-ci.org/keinohguchi/kernel-in-action.svg

## Build

Top level `make` will do the work:

```sh
air1$ pwd
/home/kei/src/linux-4.16.5/kernel-in-action
air1$ make
make -C /lib/modules/4.16.5.d/build M=/home/kei/src/linux-4.16.5/kernel-in-action modules
make[1]: Entering directory '/home/kei/src/linux-4.16.5'
  CC [M]  /home/kei/src/linux-4.16.5/kernel-in-action/hello/main.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/hello/hello.o
  CC [M]  /home/kei/src/linux-4.16.5/kernel-in-action/ps/main.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/ps/ps.o
  CC [M]  /home/kei/src/linux-4.16.5/kernel-in-action/scull/main.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/scull/scull.o
  CC [M]  /home/kei/src/linux-4.16.5/kernel-in-action/scullp/main.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/scullp/scullp.o
  CC [M]  /home/kei/src/linux-4.16.5/kernel-in-action/sleepy/main.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/sleepy/sleepy.o
  Building modules, stage 2.
  MODPOST 5 modules
  CC      /home/kei/src/linux-4.16.5/kernel-in-action/hello/hello.mod.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/hello/hello.ko
  CC      /home/kei/src/linux-4.16.5/kernel-in-action/ps/ps.mod.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/ps/ps.ko
  CC      /home/kei/src/linux-4.16.5/kernel-in-action/scull/scull.mod.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/scull/scull.ko
  CC      /home/kei/src/linux-4.16.5/kernel-in-action/scullp/scullp.mod.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/scullp/scullp.ko
  CC      /home/kei/src/linux-4.16.5/kernel-in-action/sleepy/sleepy.mod.o
  LD [M]  /home/kei/src/linux-4.16.5/kernel-in-action/sleepy/sleepy.ko
make[1]: Leaving directory '/home/kei/src/linux-4.16.5'
air1$
```

and also, `sudo make modules_install` also works and all the modules will be
installed in the right location, which will allow `modprobe` to find those
modules.

```sh
air1$ pwd
/home/kei/src/linux-4.16.5/kernel-in-action
air1$ sudo make modules_install
make -C /lib/modules/4.16.5.d/build M=/home/kei/src/linux-4.16.5/kernel-in-action modules_install
make[1]: Entering directory '/home/kei/src/linux-4.16.5'
  INSTALL /home/kei/src/linux-4.16.5/kernel-in-action/hello/hello.ko
  INSTALL /home/kei/src/linux-4.16.5/kernel-in-action/ps/ps.ko
  INSTALL /home/kei/src/linux-4.16.5/kernel-in-action/scull/scull.ko
  INSTALL /home/kei/src/linux-4.16.5/kernel-in-action/scullp/scullp.ko
  INSTALL /home/kei/src/linux-4.16.5/kernel-in-action/sleepy/sleepy.ko
  DEPMOD  4.16.5.d
make[1]: Leaving directory '/home/kei/src/linux-4.16.5'
air1$
```

## Load

`sudo make load` will install and load those modules.

```sh
air1$ sudo make load
make -C /lib/modules/4.16.6.3/build M=/home/kei/src/linux-4.16.6/kernel-in-action modules_install
make[1]: Entering directory '/home/kei/src/linux-4.16.6'
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/hello/hello.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/ps/ps.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/scull/scull.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/sculld/sculld.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/scullp/scullp.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/sleepy/sleepy.ko
  DEPMOD  4.16.6.3
make[1]: Leaving directory '/home/kei/src/linux-4.16.6'
for MODULE in ps hello scull sleepy scullp sculld; do modprobe $MODULE; done
[ 1480.768916] systemd[1]
[ 1480.769404] kthreadd[2]
[ 1480.770009] kworker/0:0H[4]
[ 1480.770627] kworker/u4:0[5]
...
air1$
```

### Process status

[ps.ko] module is a classic ps command inside kernel:

[ps.ko]: ps/main.c

```sh
air1$ sudo modprobe ps
[ 3467.251298] systemd[1]
[ 3467.252030] kthreadd[2]
[ 3467.252509] kworker/0:0H[4]
[ 3467.253987] mm_percpu_wq[6]
[ 3467.254910] ksoftirqd/0[7]
[ 3467.255553] rcu_preempt[8]
[ 3467.256449] rcu_sched[9]
[ 3467.257212] rcu_bh[10]
[ 3467.258171] migration/0[11]
[ 3467.258715] watchdog/0[12]
[ 3467.259185] cpuhp/0[13]
[ 3467.259891] kdevtmpfs[14]
[ 3467.260326] netns[15]
[ 3467.260679] rcu_tasks_kthre[16]
[ 3467.261167] kauditd[17]
[ 3467.261485] khungtaskd[19]
[ 3467.261924] oom_reaper[20]
[ 3467.262328] writeback[21]
[ 3467.263040] kcompactd0[22]
[ 3467.263509] ksmd[23]
[ 3467.263840] khugepaged[24]
[ 3467.264256] crypto[25]
[ 3467.264624] kintegrityd[26]
[ 3467.265063] kblockd[27]
[ 3467.265438] edac-poller[28]
[ 3467.265877] devfreq_wq[29]
[ 3467.266472] watchdogd[30]
[ 3467.266853] kswapd0[31]
[ 3467.267415] kthrotld[45]
[ 3467.267881] ipv6_addrconf[46]
[ 3467.268383] kworker/u2:1[47]
[ 3467.268830] kstrp[56]
[ 3467.269186] ata_sff[99]
[ 3467.270787] scsi_eh_0[100]
[ 3467.271302] scsi_tmf_0[101]
[ 3467.271778] scsi_eh_1[102]
[ 3467.272224] scsi_tmf_1[103]
[ 3467.272943] kworker/0:1H[105]
[ 3467.273440] jbd2/vda1-8[123]
[ 3467.273907] ext4-rsv-conver[124]
[ 3467.274457] systemd-journal[156]
[ 3467.274864] lvmetad[159]
[ 3467.275353] systemd-udevd[164]
[ 3467.276487] systemd-network[165]
[ 3467.277057] systemd-resolve[182]
[ 3467.277607] systemd-logind[184]
[ 3467.278143] dbus-daemon[185]
[ 3467.278612] sshd[199]
[ 3467.279147] agetty[206]
[ 3467.279792] ttm_swap[234]
[ 3467.280254] sshd[273]
[ 3467.280557] systemd[275]
[ 3467.280876] (sd-pam)[276]
[ 3467.281207] sshd[281]
[ 3467.281494] bash[282]
[ 3467.281817] sshd[1014]
[ 3467.282219] sshd[1016]
[ 3467.282621] bash[1017]
[ 3467.283226] journalctl[1020]
[ 3467.283793] kworker/0:2[5855]
[ 3467.284442] login[6779]
[ 3467.284863] cscope[7516]
[ 3467.285315] vim[7517]
[ 3467.285705] cscope[7520]
[ 3467.286332] kworker/u2:0[7521]
[ 3467.286877] kworker/0:1[8357]
[ 3467.287387] bash[8358]
[ 3467.287791] sudo[9765]
[ 3467.288230] insmod[9766]
air1$
```

### Hello world

[hello.ko] module is a classic hello world kernel thread module:

[hello.ko]: hello/main.c

```sh
air1$ sudo modprobe hello
[ 9151.462539] hello_init
[ 9151.464501] Hello world!
[ 9152.478305] Hello world!
[ 9153.491636] Hello world!
[ 9154.504930] Hello world!
[ 9155.518379] Hello world!
[ 9156.531786] Hello world!
[ 9157.545049] Hello world!
air1$ sudo rmmod hello
[ 9158.342860] hello_exit
```

### Scull

[scull.ko] is a [LDD]'s simple character device, explained in [LDD chapter 3].

[scull.ko]: scull/main.c

```sh
air1$ sudo modprobe scull
[ 2470.008335] scull0[246:0]: added
[ 2470.008894] scull1[246:1]: added
[ 2470.009638] scull2[246:2]: added
[ 2470.010373] scull3[246:3]: added
air1$ ls -l /dev/scull*
crw------- 1 root root 246, 0 Apr 25 21:19 /dev/scull0
crw------- 1 root root 246, 1 Apr 25 21:19 /dev/scull1
crw------- 1 root root 246, 2 Apr 25 21:19 /dev/scull2
crw------- 1 root root 246, 3 Apr 25 21:19 /dev/scull3
```
```sh
air1$ sudo bash -c 'ls -l > /dev/scull3'
[ 2488.790331] scull_open
[ 2488.790822] trim_qset
[ 2488.791913] scull_write
[ 2488.792400] scull_release
```
```sh
air1$ sudo cat /dev/scull3
[ 2497.280772] scull_open
[ 2497.281594] scull_read
total 28
-rw-r--r-- 1 kei wheel  220 Apr 25 20:40 Makefile
-rw-r--r-- 1 kei wheel    0 Apr 25 20:44 Module.symvers
-rw-r--r-- 1 kei wheel 4327 Apr 25 21:18 README.md
drwxr-xr-x 2 kei wheel 4096 Apr 25 20:44 hello
-rw-r--r-- 1 kei wheel  187 Apr 25 21:06 modules.order
drwxr-xr-x 2 kei wheel 4096 Apr 25 20:44 ps
drwxr-xr-x 2 kei wheel 4096 Apr 25 21:06 scull
[ 2497.287428] scull_read
[ 2497.288004] scull_release
air1$
```

### Sleepy

[sleepy.ko] is a simple module to demonstrate the wait queue, as explained
in [LDD chapter 5].

[sleepy.ko]: sleepy/main.c

```sh
air1$ sudo modprobe sleepy
[ 1700.779823] sleepy_init
[ 1700.780192] sleep0[246:0]: added
[ 1700.780701] sleep1[246:1]: added
[ 1700.781214] sleep2[246:2]: added
[ 1700.781727] sleep3[246:3]: added
air1$ ls -l /dev/sleep*
crw------- 1 root root 246, 0 Apr 27 14:05 /dev/sleep0
crw------- 1 root root 246, 1 Apr 27 14:05 /dev/sleep1
crw------- 1 root root 246, 2 Apr 27 14:05 /dev/sleep2
crw------- 1 root root 246, 3 Apr 27 14:05 /dev/sleep3
```

Let's read something from the sleepy device, e.g. `sleep3`:

```sh
air1$ sudo cat /dev/sleep3
[ 1715.179215] sleepy_open(sleep3)
[ 1715.183207] sleepy_read(sleep3)
[ 1715.183859] process 6153 (cat) going to sleep
```
The `cat` process blocked, e.g. slept, inside `sleepy_read`.

Now, from another teaminal, just write something to `/dev/sleep3`,
e.g. `sudo bash -c date > /dev/sleep3`:

```sh
[ 1727.499938] sleepy_open(sleep3)
[ 1727.503422] sleepy_write(sleep3)
[ 1727.503845] process 6671 (date) awakening the reader...
[ 1727.504461] awoken 6153 (cat)
[ 1727.504828] sleepy_release(sleep3)
[ 1727.506273] sleepy_release(sleep3)
air1$
```

Process 6153, `cat` process, has been awoken by process 6671, through
the sleepy_write() method call.

### Scullp

[scullp.ko] is a pipe version of the scull, which blocks both in read/write
while there is no enough data to read or enough space to write, as explained
in [LDD chapter 5].  This is a great example to show case kernel's
[wait queue].

[scullp.ko]: scullp/main.c
[wait queue]: https://github.com/torvalds/linux/blob/master/include/linux/wait.h

First load the module:

```sh
air1$ sudo modprobe scullp
[  586.546778] scullp[scullp_init]:
[  586.547248] scullp[scullp_init]: added scullp0[247:0]
[  586.547944] scullp[scullp_init]: added scullp1[247:1]
[  586.548669] scullp[scullp_init]: added scullp2[247:2]
[  586.549427] scullp[scullp_init]: added scullp3[247:3]
air1$
```

and read it with `cat` command:

```sh
air1$ sudo cat /dev/scullp2
[  625.119341] scullp[scullp_open]: opening scullp2
[  625.120004] scullp[scullp_read]: reading from scullp2

```

From a different terminal, write some data, e.g.
`sudo bash -c 'date > /dev/scullp2'` to fill a data to the pipe:

```
[  647.724218] scullp[scullp_open]: opening scullp2
[  647.725966] scullp[scullp_write]: writing on scullp2
Sat Apr 28 10:32:36 PDT 2018
[  647.727783] scullp[scullp_read]: reading from scullp2
[  647.728700] scullp[scullp_release]: releasing scullp2

^C
air1$
```

As above, you can see the `date` output on the console.

### Sculld

[sculld] is a registerable scull driver, explained in [LDD chapter 14]:

[sculld]: sculld/main.c

```sh
air1$ sudo make install
make -C /lib/modules/4.16.6.9/build M=/home/kei/src/linux-4.16.6/kernel-in-action modules_install
make[1]: Entering directory '/home/kei/src/linux-4.16.6'
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/hello/hello.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/ps/ps.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/scull/scull.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/sculld/sculld.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/scullp/scullp.ko
  INSTALL /home/kei/src/linux-4.16.6/kernel-in-action/sleepy/sleepy.ko
  DEPMOD  4.16.6.9
make[1]: Leaving directory '/home/kei/src/linux-4.16.6'
air1$ sudo modprobe sculld
air1$ ls -l /sys/bus/ldd
total 0
drwxr-xr-x 2 root root    0 May  1 18:56 devices
drwxr-xr-x 2 root root    0 May  1 18:56 drivers
-rw-r--r-- 1 root root 4096 May  1 18:56 drivers_autoprobe
--w------- 1 root root 4096 May  1 18:56 drivers_probe
--w------- 1 root root 4096 May  1 18:55 uevent
air1$
```

## Test

It has a kernel's selftest based test cases.

Make sure you load the target module(s):

```sh
air1$ sudo lsmod | grep scull
scullp                 20480  0
scull                  20480  0
```

then, run `make run_tests` with the root priviledge:

```
air1$ sudo make run_tests
make[1]: Entering directory '/home/kei/src/linux-4.16.6/kernel-in-action/scull/tests'
TAP version 13
selftests: ioctl_test
========================================
1. ioctl(SCULL_IOC?QSET)
2. ioctl(SCULL_IOC?QUANTUM)
ok 1..1 selftests: ioctl_test [PASS]
make[1]: Leaving directory '/home/kei/src/linux-4.16.6/kernel-in-action/scull/tests'
make[1]: Entering directory '/home/kei/src/linux-4.16.6/kernel-in-action/scullp/tests'
TAP version 13
selftests: open_test
========================================
 1) read only       : PASS
 2) write only      : PASS
Pass 2 Fail 0 Xfail 0 Xpass 0 Skip 0 Error 0
1..2
ok 1..1 selftests: open_test [PASS]
selftests: select_test
========================================
 1) write ready on write only fd                    : PASS
 2) write ready on read-write fd                    : PASS
 3) write 1 byte on write only fd                   : PASS
 4) write 1 byte on read-write fd                   : PASS
 5) write 1024 bytes on write only fd               : PASS
 6) write 1024 bytes on read-write fd               : PASS
 7) read 1 byte of data                             : PASS
 8) read 1024 bytes of data                         : PASS
Pass 8 Fail 0 Xfail 0 Xpass 0 Skip 0 Error 0
1..8
ok 1..2 selftests: select_test [PASS]
make[1]: Leaving directory '/home/kei/src/linux-4.16.6/kernel-in-action/scullp/tests'
air1$
```

## Unload

`sudo make unload` will unload all the drivers:

```sh
air1$ sudo make unload
for MODULE in ps hello scull sleepy scullp sculld; do rmmod $MODULE; done
[ 1662.731301] ps_exit()
[ 1662.740866] hello_exit
[ 1662.751236] scull_exit
[ 1662.752461] scull0[247:0]: deleted
[ 1662.753870] __trim_qset
[ 1662.754777] scull1[247:1]: deleted
[ 1662.755887] __trim_qset
[ 1662.756979] scull2[247:2]: deleted
[ 1662.758133] __trim_qset
[ 1662.758993] scull3[247:3]: deleted
[ 1662.760260] __trim_qset
[ 1662.774189] sleepy_exit
[ 1662.775146] sleep0[246:0]: deleted
[ 1662.776793] sleep1[246:1]: deleted
[ 1662.778171] sleep2[246:2]: deleted
[ 1662.779517] sleep3[246:3]: deleted
[ 1662.794207] scullp: scullp_exit(): exiting
[ 1662.795160] scullp: scullp_terminate(): deleting scullp0[245:0]
[ 1662.796574] scullp: scullp_terminate(): deleting scullp1[245:1]
[ 1662.797903] scullp: scullp_terminate(): deleting scullp2[245:2]
[ 1662.799456] scullp: scullp_terminate(): deleting scullp3[245:3]
[ 1662.814432] sculld_exit
air1$
```

## Cleanup

Just `make clean`, of course:

```sh
air1$ make clean
make -C /lib/modules/4.16.2.1/build M=/home/kei/git/kernel-in-action clean
make[1]: Entering directory '/home/kei/src/linux-4.16.2'
  CLEAN   /home/kei/git/kernel-in-action/.tmp_versions
  CLEAN   /home/kei/git/kernel-in-action/Module.symvers
make[1]: Leaving directory '/home/kei/src/linux-4.16.2'
air1$ ls -l ./ps/
total 8
-rw-r--r-- 1 kei wheel 204 Apr 23 15:28 Makefile
-rw-r--r-- 1 kei wheel   0 Apr 23 15:29 Module.symvers
-rw-r--r-- 1 kei wheel 395 Apr 23 15:29 main.c
```

# References

- [LKD]: Linux Kernel Development, by [Robert Love]
- [LDD]: Linux Device Driver, by [Jonathan Corbet], [Alessandro Rubini]
                                 & [Greg Kroah-Hartman]

[LKD]: https://www.amazon.com/Linux-Kernel-Development-Robert-Love/dp/0672329468/ref=as_li_ss_tl?ie=UTF8&tag=roblov-20
[LDD]: https://lwn.net/Kernel/LDD3/
[LDD chapter 3]:  https://lwn.net/images/pdf/LDD3/ch03.pdf
[LDD chapter 5]:  https://lwn.net/images/pdf/LDD3/ch05.pdf
[LDD chapter 14]: https://lwn.net/images/pdf/LDD3/ch14.pdf
[Robert Love]: https://rlove.org/
[Jonathan Corbet]: http://www.oreilly.com/pub/au/592
[Alessandro Rubini]: http://www.linux.it/~rubini/
[Greg Kroah-Hartman]: http://www.kroah.com/

Happy Hackin!
