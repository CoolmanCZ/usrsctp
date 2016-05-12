#MINGW
This fork is used to build usrsctp library using MINGW compiler.

##build:
```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
make
```

# usrsctp

This is a userland SCTP stack supporting FreeBSD, Linux, Mac OS X and Windows.

See [manual](Manual.md) for more information.

The status of continous integration testing is available from [grid](http://212.201.121.110:18010/grid) and [waterfall](http://212.201.121.110:18010/waterfall).
If you are only interested in a single branch, just append `?branch=BRANCHNAME` to the URL, for example [waterfall](http://212.201.121.110:18010/waterfall?branch=master).
