# Linux Kernel Development in action

[![Build Status]](https://travis-ci.org/keinohguchi/lkd-in-action)

Our beloved [LKD] in action on the latest kernel.

[Build Status]: https://travis-ci.org/keinohguchi/lkd-in-action.svg
[LKD]: https://www.amazon.com/Linux-Kernel-Development-Robert-Love/dp/0672329468/ref=as_li_ss_tl?ie=UTF8&tag=roblov-20

## Build

Top level `make` will do the work:

```sh
air1$ make
make -C /lib/modules/4.16.2.1/build M=/home/kei/git/lkd-in-action modules
make[1]: Entering directory '/home/kei/src/linux-4.16.2'
  CC [M]  /home/kei/git/lkd-in-action/ps/main.o
  LD [M]  /home/kei/git/lkd-in-action/ps/ps.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/kei/git/lkd-in-action/ps/ps.mod.o
  LD [M]  /home/kei/git/lkd-in-action/ps/ps.ko
make[1]: Leaving directory '/home/kei/src/linux-4.16.2'
air1$
```

## Load

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
make -C /lib/modules/4.16.2.1/build M=/home/kei/git/lkd-in-action clean
make[1]: Entering directory '/home/kei/src/linux-4.16.2'
  CLEAN   /home/kei/git/lkd-in-action/.tmp_versions
  CLEAN   /home/kei/git/lkd-in-action/Module.symvers
make[1]: Leaving directory '/home/kei/src/linux-4.16.2'
air1$ ls -l ./ps/
total 8
-rw-r--r-- 1 kei wheel 204 Apr 23 15:28 Makefile
-rw-r--r-- 1 kei wheel   0 Apr 23 15:29 Module.symvers
-rw-r--r-- 1 kei wheel 395 Apr 23 15:29 main.c
```

Happy Hackin!
