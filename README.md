# Linux Kernel Development in action

[![Build Status]](https://travis-ci.org/keinohguchi/lkd-in-action)

Our beloved [LKD] in action on the latest linux kernel.

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
air1$ sudo insmod ./ps/ps.ko
[ 1421.464189] ps_init()
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
