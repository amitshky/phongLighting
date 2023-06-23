# phongLighting
A simple Blinn-Phong lighting implementation.

<video src='img/vid.mkv' width=550/>


## Prerequisites
* [CMake](https://cmake.org/download/)
* [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) ([Installation guide](https://vulkan.lunarg.com/doc/sdk/latest/windows/getting_started.html))


## Build and Run
* Configure and build the project,
```
cmake -B build -S .
cmake --build build
```
* Then navigate to the output file and run it (run it from the root directory of the repo). For example,
```
./build/<path_to_executable>
```

OR (in VSCode)

* Start debugging (Press F5) (Currently configured for Clang with Ninja and MSVC for Windows)

OR (using bat scripts from `scripts` folder)

* Run them from the root directory of the repo. For example:
```
./scripts/config-clang-rel.bat
./scripts/build-clang.bat
./scripts/run-clang.bat
```


## Usage
* WASD to move the camera forward, left, back, and right respectively.
* E and Q to move the camera up and down.
* R to reset the camera
* Ctrl+Q to close the window
* Left click and drag the mouse to move the camera


## Screenshots
<img src="img/phongLighting.png" width=550>


## References
* [Vulkan tutorial](https://vulkan-tutorial.com/)
* [learnopengl.com](https://learnopengl.com/)
* [Vulkan Specification](https://registry.khronos.org/vulkan/specs/1.3-extensions/pdf/vkspec.pdf)
* [Sascha Willems examples](https://github.com/SaschaWillems/Vulkan)
* [vkguide.dev](https://vkguide.dev/)