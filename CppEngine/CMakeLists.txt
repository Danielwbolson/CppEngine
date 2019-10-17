cmake_minimum_required (VERSION 3.8)

project (CppEngine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wno-implicit-fallthrough -Wshadow -Wno-unused-variable -Wno-unused-function")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2") # look up Ofast
elseif (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Configuration.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/Configuration.h)



set(LAPTOP_SDL D:/SDL2-2.0.8/)
set(DESKTOP_SDL E:/SDL2-2.0.10/)

if (UNIX AND NOT APPLE)
 find_package( SDL2 REQUIRED component main)
 set( SDL2_LIBRARIES "SDL2")
 set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread" )
elseif (APPLE)
  find_package( SDL2 REQUIRED component main)
elseif(MSVC)
  link_directories( ${DESKTOP_SDL}lib/x64 )
  set( SDL2_INCLUDE_DIRS ${DESKTOP_SDL}include )
  #link_directories( ${LAPTOP_SDL}lib/x64)
  #set(SDL2_INCLUDE_DIRS ${LAPTOP_SDL}include)
  
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




set(COMPONENT_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/Component.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/Door.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/Grab.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/InteractableObject.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/Key.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/MeshRenderer.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/PlayerMovement.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/Transform.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/colliders/BoxCollider.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/colliders/Collider.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/colliders/SphereCollider.h
)
set(COMPONENT_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/Component.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/Door.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/Grab.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/InteractableObject.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/Key.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/MeshRenderer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/PlayerMovement.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/Transform.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/colliders/BoxCollider.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/colliders/Collider.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/component/colliders/SphereCollider.cpp
)

set(FILETYPES_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/fileTypes/Map.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/fileTypes/Scene.h
)
set(FILETYPES_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/fileTypes/Map.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/fileTypes/Scene.cpp
)

set(LIGHTS_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/AmbientLight.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/DirectionalLight.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/Light.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/PointLight.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights/SpotLight.h
)

set(OBJECTDATA_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/objectData/GameObject.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/objectData/Material.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/objectData/Mesh.h
)
set(OBJECTDATA_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/objectData/GameObject.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/objectData/Material.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/objectData/Mesh.cpp
)

set(SYSTEMS_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/ColliderSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/MeshRendererSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/PhysicsSystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/systems/Systems.h
)
set(SYSTEMS_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/systems/ColliderSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/systems/MeshRendererSystem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/systems/PhysicsSystem.cpp
)

set(UTILITY_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Globals.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Parse.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/SDL_Static_Helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Utility.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Vec2.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/utility/Vec3.h
)
set(UTILITY_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/utility/SDL_Static_Helper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/utility/Vec2.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/utility/Vec3.cpp
)

set(OTHER_H
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/Camera.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/MainVariables.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/Configuration.h
)
set(OTHER_CPP
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/Camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/source/main.cpp
)

set(SHADERS
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredLighting.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredLighting.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredLightvolumes.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredLightvolumes.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredToTexture.frag
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/deferredToTexture.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/Phong.vert
	${CMAKE_CURRENT_SOURCE_DIR}/shaders/Phong.frag
)

set(INCS
	${COMPONENT_H}
	${FILETYPES_H}
	${LIGHTS_H}
	${OBJECTDATA_H}
	${SYSTEMS_H}
	${UTILITY_H}
	${OTHER_H}
)
set(SRC
	${COMPONENT_CPP}
	${FILETYPES_CPP}
	${OBJECTDATA_CPP}
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
	${CMAKE_CURRENT_SOURCE_DIR}/external
	
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/component/colliders
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/fileTypes
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/lights
	${CMAKE_CURRENT_SOURCE_DIR}/src/headers/objectData
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

target_link_libraries(CppEngine ${LIBS} glad)

# Put necessary lib files in our build directory
add_custom_command(TARGET CppEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/libs"
        $<TARGET_FILE_DIR:CppEngine>)
