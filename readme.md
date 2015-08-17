AwayExtensions-FBXPlugin
========================


requirement
-----------

Cmake

```sh
$ brew install cmake
```

awd-sdk

https://github.com/fyoudine/awd-sdk/tree/develop


build
-----

be sure to checkout submodules

```
$ git submodule init
$ git submodule update
```

on branch develop

```sh
$ mkdir build && cd build
$ cmake ../ -DFBX_VERSION=2016.1
$ make
```
