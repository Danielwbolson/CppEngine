# CppEngine

A Rendering engine using deferred shading. Currently runs with some very basic functions in Lua for the user to spawn lights and gameobjects.
Engine Features:
- Entity-Component-Systems Architecture
- Deferred Shading
- Point Light Volumes
- Material and texturing support
- Transparency (Sorted back-to-front on CPU)

clone using --recurse-submodules to grab submodules
<br/>
Make sure you change CMakeLists.txt lines 36-40 to the appropriate location of your SDL download.
<br/>
<br/>
<br/>
To run, navigate to the root directory. You should see src/, materials/, etc.

Run: `./build/Debug/CppEngine.exe games/<gameFolder>/`

You can run it currently with games/sponza/ to see a demo of the popular Sponza model featuring 1000 lights and transparency.
