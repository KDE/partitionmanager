# KDE Partition Manager

<img src="https://invent.kde.org/kde/partitionmanager/raw/master/icons/sc-apps-partitionmanager.svg" align="right"
     title="KDE Partition Manager logo" width="96" height="96">

KDE Partition Manager is an application to help you manage the disk devices,
partitions and file systems on your computer. It allows you to easily create,
copy, move, delete, resize without losing data, backup and restore partitions.

A large number of file systems are also supported:
- ext2/3/4, Btrfs, Linux swap
- Reiserfs
- F2FS
- NTFS
- FAT
- exFAT
- LUKS encryption support
- and more....

:zap: **Back up your data before using this software**. KDE Partition Manager is
a potentially dangerous program for your data. It has been tested carefully and
there are currently no known bugs that could lead to data loss, but nevertheless
there is always a chance for an error to occur and you might lose your data.

<img src="https://docs.kde.org/trunk5/en/extragear-sysadmin/partitionmanager/resize_howto_4.png" align="center"
     title="KDE Partition Manager" width="800">

## Installation

KDE Partition Manager is a KF5 application, so you will need the
[KDE frameworks](https://www.kde.org/products/frameworks/) libraries to run it
on your computer. Most modern OSs will install them as dependencies
for you when you use their default package manager as the installation method.

It also makes use of external programs to get its job done, so
you might have to install additional software (preferably packages from your
distribution) to make use of all features and get full support for all file
systems.

The methods listed below for each major OS are based on executing the
installation commands on a terminal window. Alternatively, you can use
your OS' package management app. 

### Ubuntu

```bash
sudo apt install partitionmanager
```

### Debian

As the `root` user:

```bash
apt install partitionmanager
```

### CentOS, Fedora, RHEL

```bash
sudo yum install kde-partitionmanager
```

### OpenSUSE
```bash
sudo zypper install partitionmanager
```

### ArchLinux

1. Enable the community repository on `/etc/pacman.conf`:
    ```ini
    [community]
    Include = /etc/pacman.d/mirrorlist
    ```
1. Install the `partitionmanager` xz package:
    ```
    # pacman -Sy partitionmanager
    ```

### Gentoo
```bash
sudo emerge partitionmanager
```

### From source

See [INSTALL](INSTALL.md).

## Changelog

For a list of changes since the previous release see [CHANGES](CHANGES).
