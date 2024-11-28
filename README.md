## iRecHS2s uvc

### Introduction
This project aims to provide uvc camera support for [iRecHS2s](https://staff.aist.go.jp/k.matsuda/iRecHS2/index_e.html).

### Requirements
- [libuvc](https://github.com/libuvc/libuvc)
- [libusb](https://github.com/libusb/libusb)
- [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo)

### Build
首先修改 `CMakePresets.json` 文件，将 `<path to store>` 修改为你希望 vcpkg 缓存包的位置, 通常来讲是不需要的, 但是由于 pupil lab 的 cmake 设置的问题, 需要修改到项目根目录之外的地方

```bash
mkdir build
cd build
cmake --preset=vcpkg ..
cmake --build ..
```

依旧是由于 pupillab 的 cmake 设置问题, 需要手动将将生成的 `iRecHS2sUvc.vcxproj` 文件中多余到源文件删掉, 基本上是从行 412 开始, 10 行左右

### License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
