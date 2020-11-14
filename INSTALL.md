<!-- SPDX-FileCopyrightText: 2008-2010 Volker Lanz <vl@fidra.de>
     SPDX-FileCopyrightText: 2016-2019 Andrius Štikonas <andrius@stikonas.eu>
     SPDX-License-Identifier: GFDL-1.2-or-later
-->

Building and installing KDE Partition Manager from source
=========================================================


1. Dependencies

KDE Frameworks: The minimum required version is 5.31.

KPMcore: https://cgit.kde.org/kpmcore.git/


2. Configure

KDE Partition Manager is built with cmake. It is
recommended to build out of tree: After unpacking the source, create a separate
build directory and run cmake there:

```
$ tar xf partitionmanager-x.y.z.tar.xz
$ cd partitionmanager-x.y.z
$ mkdir build
$ cd build
$ cmake ..
```

If all dependencies are met, cmake configures the build directory.


3. Build and install

Just run make and make install in the build directory. The default install path
is `/usr/local`, so installing will need write privileges there. You can
configure a different install path by passing
`-DCMAKE_INSTALL_PREFIX=<your_path>` to cmake when configuring. To change the
install path after configuring and building, run

```
$ ccmake .
```

in the build directory and modify `CMAKE_INSTALL_PREFIX` there.


4. Running

KDE Partition Manager should be run as root. Running it as an unprivileged user
does no harm, but the default settings will not allow you to apply any
operations (i.e., you can click through the UI, but cannot modify your disks).


5. Troubleshooting

If you are getting an error like this during the build:

```
index.docbook:71: parser error : Entity 'partman' not defined
```

there is a problem with a documentation file. This is easily fixed by
commenting out the offending language in `doc/CMakeLists.txt` -- the error
message should indicate which language causes the error. As an alternative, you
can disable building all documentation by commenting out the
`macro_optional_add_subdirectory(doc)` line in the toplevel `CMakeLists.txt` file.

