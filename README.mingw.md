# MINGW
MinGW was tested on Windows 10 using [MSYS2](http://msys2.github.io/) - 32bit and 64bit build. Cross compiling was tested on Ubuntu 14.04 LTS.

We recommend to don't use in-source build. So create a build directory first and execute build commands in this directory.

#### Generate Makefiles for MSVC project on Windows
````
mkdir build.msvc
cd build.msvc
cmake.exe ..
```

#### MINGW 32bit or 64bit on Windows
```
mkdir build.mingw32
cd build.mingw32
cmake.exe -G "MinGW Makefiles" ..
mingw32-make
```

#### Cross compile 32bit or 64bit on Linux
```
mkdir build.cross32
cd build.cross32
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
make
```

#### 32bit toolchain example for Ubuntu
```
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32 )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

#### 64bit toolchain example for Ubuntu
```
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32 )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

