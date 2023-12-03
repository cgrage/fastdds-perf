# Readme

Benchmark for IPC on embedded systems using FAST DDS

## Install

Follow: https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html

- No "Gtest" required
- Use CMake installation
- No "Fast DDS Python bindings" required

Additional:

```bash
sudo apt install sysstat
```

## Compile

### Generate IDL

```bash
cd src/idl
 ~/Fast-DDS-Gen/scripts/fastddsgen -replace *.idl 
```

### Build

```bash
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=~/Fast-DDS/install/ ..
cmake --build .
```

### Run

```bash
export LD_LIBRARY_PATH=~/Fast-DDS/install/lib
./subscriber
./publisher
```
