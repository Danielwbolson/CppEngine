# CppEngine

A multi-pass Rendering engine using deferred shading. Currently runs with some very basic functions in Lua for the user to spawn lights and gameobjects and interact with the sun.
Engine Features:
- Entity-Component-Systems Architecture
- Deferred Shading
- Point Light Volumes
- Dynamic Directional light shadow map
- Transparency (Sorted back-to-front on CPU)
- Post-processing step (currently just gamma correction)
- Material and texturing support

To clone, run:
```
git clone --recurse-submodules https://github.com/Danielwbolson/CppEngine.git
```
<br/>
Make sure you change CMakeLists.txt lines 36-40 to the appropriate location of your SDL download.
<br/>
<br/>
<br/>
To run, navigate to the root directory. You should see src/, materials/, etc.

Run:
```
./build/Release/CppEngine.exe games/<gameFolder>/
```
On Windows, you can also run from within Visual Studio:
Add `games/<gameFolder>/` to CppEngine->Properties->Debugging->Command Arguments

I recommend running with games/sponza/ to see a demo of the popular Sponza model featuring hundreds of point light volumes and a  dynamic directional light.
