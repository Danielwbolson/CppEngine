# CppEngine

A Deferred Rendering "engine". Currently runs with some very basic functions in Lua for the user to spawn lights and gameobjects.
Engine Features:
- Entity-Component-Systems Architecture
- Deferred Lighting
- Point Light Volumes
- Material and texturing support (Not normal maps or displacement maps yet)

clone using --recursive to grab submodules
<br/>
Make sure you change CMakeLists.txt lines 36-40 to the appropriate location of your SDL download.
<br/>
<br/>
<br/>
To run, navigate to the root directory. You should see src/, materials/, etc.

Run: `./build/Debug/CppEngine.exe games/<gameFolder>/`

You can run it currently with games/demo/ to see a complex, basically animated, textured set of models with many light volumes.
