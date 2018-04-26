# Linux Kernel Development in action

[![Build Status]](https://travis-ci.org/keinohguchi/kernel-in-action)

Our beloved [LKD] and [LDD] in action on the latest kernel.

- [Build](#build)
- [Load](#load)
  - [Process status](#process-status)
  - [Hello world](#hello-world)
  - [Scull](#scull)
- [Unload](#unload)
- [Cleanup](#cleanup)
- [References](#references)

[Build Status]: https://travis-ci.org/keinohguchi/kernel-in-action.svg

## Build

Top level `make` will do the work:

```sh
air1$ make
make -C /lib/modules/4.16.2.1/build M=/home/kei/git/kernel-in-action modules
make[1]: Entering directory '/home/kei/src/linux-4.16.2'
  CC [M]  /home/kei/git/kernel-in-action/ps/main.o
  LD [M]  /home/kei/git/kernel-in-action/ps/ps.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/kei/git/kernel-in-action/ps/ps.mod.o
  LD [M]  /home/kei/git/kernel-in-action/ps/ps.ko
make[1]: Leaving directory '/home/kei/src/linux-4.16.2'
air1$
```

## Load

### Process status

[ps.ko] module is a classic ps command inside kernel:

[ps.ko]: ps/main.c

```sh
air1$ sudo insmod ps/ps.ko
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

[khellod.ko] module is a classic hello world kernel thread:

[khellod.ko]: khellod/main.c

```sh
air1$ sudo insmod khellod/khellod.ko
[ 9151.462539] khellod_init
[ 9151.464501] Hello world!
[ 9152.478305] Hello world!
[ 9153.491636] Hello world!
[ 9154.504930] Hello world!
[ 9155.518379] Hello world!
[ 9156.531786] Hello world!
[ 9157.545049] Hello world!
air1$ sudo rmmod khellod
[ 9158.342860] khellod_exit
```

### Scull

[scull.ko] is a [LDD]'s simple character device, explained in [LDD chapter 3].

[scull.ko]: scull/main.c

```sh
air1$ pwd
/home/kei/src/linux-4.16.4/kernel-in-action
air1$ sudo insmod ./scull/scull.ko
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
drwxr-xr-x 2 kei wheel 4096 Apr 25 20:44 khellod
-rw-r--r-- 1 kei wheel  187 Apr 25 21:06 modules.order
drwxr-xr-x 2 kei wheel 4096 Apr 25 20:44 ps
drwxr-xr-x 2 kei wheel 4096 Apr 25 21:06 scull
[ 2497.287428] scull_read
[ 2497.288004] scull_release
air1$
```

## Unload

```sh
air1$ sudo rmmod ps
[ 1134.851008] ps_exit()
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
[LDD chapter 3]: https://lwn.net/images/pdf/LDD3/ch03.pdf
[Robert Love]: https://rlove.org/
[Jonathan Corbet]: http://www.oreilly.com/pub/au/592
[Alessandro Rubini]: http://www.linux.it/~rubini/
[Greg Kroah-Hartman]: http://www.kroah.com/

Happy Hackin!
