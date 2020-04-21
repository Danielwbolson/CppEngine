cmake_minimum_required (VERSION 3.8)

project (CppEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wextra -Wno-implicit-fallthrough -Wshadow -Wno-unused-variable -Wno-unused-function")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2") # look up Ofast
elseif (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Configuration.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Configuration.h)



set(LAPTOP_SDL D:/SDL2-2.0.10/)
set(DESKTOP_SDL E:/SDL2-2.0.10/)

if (UNIX AND NOT APPLE)
 find_package( SDL2 REQUIRED component main)
 set( SDL2_LIBRARIES "SDL2")
 set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread" )
elseif (APPLE)
  find_package( SDL2 REQUIRED component main)
elseif(MSVC)
  link_directories( ${DESKTOP_SDL}lib/x64 )
  set(SDL2_INCLUDE_DIRS ${DESKTOP_SDL}include )
  #link_directories( ${LAPTOP_SDL}lib/x64)
  #set(SDL2_INCLUDE_DIRS ${LAPTOP_SDL}include)
  
  #find_file(SDL2_INCLUDE_DIR NAME SDL.h HINTS SDL2)
  #find_library(SDL2_LIBRARY NAME SDL2)
  
  set( SDL2_LIBRARIES SDL2 SDL2main )
endif()



set(SYSTEM_LIBS "")
if (UNIX AND NOT APPLE)
	set(SYSTEM_LIBS
		dl
		stdc++fs
	)
endif()

set(LIBS
	${SYSTEM_LIBS}
	${SDL2_LIBRARIES}
)



find_package(OpenGL REQUIRED)

file(GLOB glad_src external/glad/*.c)
add_library(glad ${glad_src})

file(GLOB lua_src external/lua-5.3.5/src/*.c)
add_library(lua ${lua_src})

file(GLOB ofbx_src 
	external/OpenFBX/src/*.c
	external/OpenFBX/src/*.cpp)
add_library(ofbx ${ofbx_src})

file(GLOB tinyobj_src external/tinyobjloader/*.cc)
add_library(tinyobj ${tinyobj_src})



set(COMPONENT_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components/Component.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components/ModelRenderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components/Transform.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components/colliders/BoxCollider.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components/colliders/Collider.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components/colliders/SphereCollider.h
)
set(COMPONENT_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/components/Component.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/components/ModelRenderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/components/Transform.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/components/colliders/BoxCollider.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/components/colliders/Collider.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/components/colliders/SphereCollider.cpp
)

set(CORE_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/core/Scene.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/core/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/core/GameObject.h
)
set(CORE_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/core/Scene.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/core/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/core/GameObject.cpp
)

set(LIGHTS_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/AmbientLight.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/DirectionalLight.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/Light.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/PointLight.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/SpotLight.h
)

set(MANAGER_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/managers/AssetManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/managers/MemoryManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/managers/MemoryAllocator.h
)
set(MANAGER_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/managers/AssetManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/managers/MemoryManager.cpp
)

set(MODEL_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/model/Material.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/model/Model.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/model/Mesh.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/model/Bounds.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/model/Texture.h
)
set(MODEL_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/model/Material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/model/Model.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/model/Mesh.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/model/Bounds.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/model/Texture.cpp
)

set(SYSTEMS_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/ColliderSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/RendererSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/PhysicsSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/Systems.h
)
set(SYSTEMS_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/systems/ColliderSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/systems/RendererSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/systems/PhysicsSystem.cpp
)

set(UTILITY_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Globals.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/SDL_Static_Helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Utility.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Configuration.h
)
set(UTILITY_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/utility/SDL_Static_Helper.cpp
)

set(OTHER_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/MainVariables.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/LuaSupport.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/Shader.h
)
set(OTHER_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/LuaSupport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/Shader.cpp
)

set(SHADERS
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredDirectionalLight.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredDirectionalLight.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredLightvolumes.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredLightvolumes.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredToTexture.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredToTexture.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/finalQuad.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/finalQuad.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/forwardPhong.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/forwardPhong.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/shadowMap.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/shadowMap.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/tiledLighting.comp
)

set(INCS
	${COMPONENT_H}
	${CORE_H}
	${LIGHTS_H}
	${MANAGER_H}
	${MODEL_H}
	${SYSTEMS_H}
	${UTILITY_H}
	${OTHER_H}
)
set(SRC
	${COMPONENT_CPP}
	${CORE_CPP}
	${MANAGER_CPP}
	${MODEL_CPP}
	${SYSTEMS_CPP}
	${UTILITY_CPP}
	${OTHER_CPP}
)
set(ALL_FILES
	${SRC}
	${INCS}
	${SHADERS}
)

set(INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/external/OpenFBX/src
	${CMAKE_CURRENT_SOURCE_DIR}/external
	${CMAKE_CURRENT_SOURCE_DIR}/external/sol2/single/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader
	
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/components/colliders
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/core
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/managers
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/model
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility
	
	${SDL2_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIRS}
)



link_directories(${LIB_DIRS})
include_directories(${INCLUDE_DIRS})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CppEngine)

add_executable(CppEngine ${SRC} ${INCS} ${SHADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_FILES})

if (MSVC)
	target_compile_options(CppEngine PRIVATE "/MP")
endif()

target_link_libraries(CppEngine ${LIBS} glad lua ofbx tinyobj)

# Put necessary lib files in our build directory
add_custom_command(TARGET CppEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/libs"
        $<TARGET_FILE_DIR:CppEngine>)
