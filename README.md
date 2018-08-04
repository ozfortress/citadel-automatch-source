# Citadel-AutoMatch for the Source Engine

## Dependencies

* [source-sdk-2013](https://github.com/ValveSoftware/source-sdk-2013)
* [CMake](https://cmake.org/)
* A build environment for 32bit supporting C++17 (eg. `g++ 7`)

### Ubuntu Dependency Installation

```bash
sudo apt install build-essential cmake libc6-dev-i386 gcc-multilib g++-7-multilib libcurl4-openssl-dev:i386
```

## Building

To configure CMake, first run:

```bash
cmake . -DSOURCE_SDK_DIR=<your source sdk location>
```

Make sure to point `SOURCE_SDK_DIR` to the `mp/src` location of the full source
sdk.

Once configured, you can now run `make` to build the plugin.

## Installing

Running `make install` will create a directory `install/addons` with all the
files generally needed to install the plugin.

# TF2 Dependencies for 64bit Ubuntu

sudo apt-get install libtinfo5:i386 libncurses5:i386 libcurl3-gnutls:i386
