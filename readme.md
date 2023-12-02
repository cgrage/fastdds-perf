# Readme

## Compile

### Generate IDL

'''bash
cd src/idl
 ~/Fast-DDS-Gen/scripts/fastddsgen *.idl 
'''

### Build

'''bash
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=~/Fast-DDS/install/ ..
cmake --build .
'''

### Run

'''bash
export LD_LIBRARY_PATH=~/Fast-DDS/install/lib
./DDSHelloWorldSubscriber
./DDSHelloWorldPublisher
'''