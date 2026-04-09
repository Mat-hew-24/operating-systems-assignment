# Character Device Driver - OS Assignment 1

## Description

This project implements a Linux Character Device Driver that enforces ordered execution of operations (read followed by write) within a specified time limit using kernel wait queues.

The module accepts:

* `kernel_version` (major, minor)
* `timer` (in seconds)

---

## Files Included

* `mymodule.c` – Kernel module source code
* `Makefile` – Build file
* `README.txt` – Usage instructions

---

## Requirements

* Linux system
* Kernel headers installed
* GCC
* Root privileges

---

## Build

```bash
make
```

---

## Insert Module

Check kernel version:

```bash
uname -r
```

Insert module:

```bash
sudo insmod mymodule.ko kernel_version=X,Y timer=Z
```

---

## Device File

After insertion:

```
/dev/mymodule
```

---

## Usage

Read from device:

```bash
cat /dev/mymodule
```

Write to device:

```bash
echo "username" | sudo tee /dev/mymodule
```

Remove module:

```bash
sudo rmmod mymodule
```

Check logs:

```bash
sudo dmesg | tail
```

---

## Notes

* Read must be performed before write
* Write blocks if read has not occurred
* Timer is specified during module insertion
* Logs can also be viewed using:

```bash
sudo journalctl -k | tail
```

---

## Clean

```bash
make clean
```

---

## Author

Ans Christy K L

