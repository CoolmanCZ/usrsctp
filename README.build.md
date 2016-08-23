# MINGW
MinGW was tested on Windows 10 using [MSYS2](http://msys2.github.io/) - 32bit and 64bit build. Cross compiling was tested on Ubuntu 14.04 LTS. 32bit or 64bit build depends on the selected MSYS2 terminal.

We recommend to do not use in-source build. So create a build directory first and execute build commands in this directory.

#### Generate CMakeLists and build project using GCC on Linux
````
mkdir build.gcc
cd build.gcc
cmake ..
make
````

#### Generate CMakeLists and build project using CLANG
```
mkdir build.clang
cd build.clang
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-clang.cmake ..
make
```

#### CLANG toolchain example for Ubuntu
```
# which compilers to use for CLANG and CLANG++
SET(_CMAKE_TOOLCHAIN_PREFIX "llvm-")
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)
```

#### Generate CMakeLists and build project using MSVC 64bit on Windows
```
mkdir build.msvc
cd build.msvc
cmake.exe -G "Visual Studio 10 Win64" ..
cmake --build . --target ALL_BUILD --config Release
```

#### Generate CMakeLists and build project using MINGW 32bit on Windows
```
mkdir build.mingw32
cd build.mingw32
cmake.exe -G "MinGW Makefiles" ..
mingw32-make
```

#### Generate CMakeLists and build project using cross compilation (32bit or 64bit) on Linux
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

