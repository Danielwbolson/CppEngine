# CppEngine

### A multi-pass Rendering engine using deferred shading. Currently runs with some very basic functions in Lua for the user to spawn lights and gameobjects and interact with the sun.

## Engine Features:  
- Entity-Component-Systems Architecture
- Deferred Shading
- Point Light Volumes
- Dynamic Directional light shadow map
- Transparency (Sorted back-to-front on CPU)
- Post-processing step (currently just gamma correction)
- Material and texturing support


## To clone, build, and run:  
```
git clone --recurse-submodules https://github.com/Danielwbolson/CppEngine.git
```
Make sure you configure `CppEngine/CppEngine/CMakeLists.txt` to the appropriate location of your SDL download:  

`26 | set(LAPTOP_SDL D:/SDL2-2.0.10/)`  
`27 | set(DESKTOP_SDL E:/SDL2-2.0.10/)`  
`...`  
`36 | link_directories( ${DESKTOP_SDL}lib/x64 )`  
`37 | set(SDL2_INCLUDE_DIRS ${DESKTOP_SDL}include )`  
`38 | #link_directories( ${LAPTOP_SDL}lib/x64)`  
`39 | #set(SDL2_INCLUDE_DIRS ${LAPTOP_SDL}include)`  
<br/>
Navigate to the root directory. You should see src/, materials/, etc and perform the following commands:
```
mkdir build
cd build
cmake-gui ..
```
Configure CMake to your system.  
Build CppEngine in your editor and finally you can call the following command from the terminal to test out the engine:
```
./build/Release/CppEngine.exe games/<gameFolder>/
```
On Windows, you can also run from within Visual Studio:  
Add `games/<gameFolder>/` to `CppEngine->Properties->Debugging->Command Arguments`

I recommend running with `games/sponza/` to see a demo of the popular Sponza model featuring hundreds of point light volumes and a  dynamic directional light. That directional light can be dynamically shifted using the `1` and `2` keys.
