# PluginShadertoy
SHADERed plugin that simplifies the process of porting your Shadertoy projects.

![Screenshot](./screen.gif)

## How to build
Clone the project:
```bash
git clone https://github.com/dfranx/PluginShadertoy.git
git submodule update --init
```

### Linux
1. Install OpenSSL (libcrypto & libssl).

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
This plugin requires at least SHADERed v1.3.5.

Copy the .dll/.so file to `plugins/Shadertoy` folder in your SHADERed's installation directory

## TODO
- cubemaps
- audio shaders
