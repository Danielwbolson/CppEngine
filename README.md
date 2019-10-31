# CppEngine

A Deferred Rendering "engine". Currently runs with some very basic functions in Lua for the user to spawn lights and gameobjects.
Engine Features:
- Engine-Component-Systems Architecture
- Deferred Lighting
- Point Light Volumes
- Material and texturing support (Not normal maps or displacement maps yet)

Was started as part of a final project for CSCI5607 in the Fall of 2018 at the University of Minnesota.
  
  
Make sure you change CMakeLists.txt lines 36-40 to the appropriate location of your SDL download.
  
  
To run, navigate to the root directory. You should see src/, materials/, etc.

Run: ./build/Debug/CppEngine.exe games/<gameFolder>/

You can run it currently with games/demo/ to see a complex, basically animated, textured set of models with many light volumes
