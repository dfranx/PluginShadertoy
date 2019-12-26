# PluginShadertoy
SHADERed plugin that simplifies the process of loading Shadertoy projects.

![Screenshot](./screen.gif)

## How to build
Clone the project:
```bash
git clone https://github.com/dfranx/PluginShadertoy.git
git submodule init
git submodule update
```

### Linux
1. Install OpenSSL (libcrypro & libssl).

2. Build:
```bash
cmake .
make
```

### Windows
1. Install libcrypto & libssl through your favourite package manager (I recommend vcpkg)
2. Run cmake-gui and set CMAKE_TOOLCHAIN_FILE variable
3. Press Configure and then Generate if no errors occured
4. Open the .sln and build the project!

## How to use
This plugin requires SHADERed v1.3 minimum.

Copy the .dll/.so file to plugins/Shadertoy in your SHADERed installation directory

## TODO
- cubemaps
- audio shaders
- texture properties
