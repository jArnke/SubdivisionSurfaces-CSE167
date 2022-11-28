# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("CSE_167_Final_SubDiv_Surfaces")

# Add sources
file(
	GLOB SOURCES_MODEL_VIEWER
	src/*.cpp
)

# Add executables
add_executable(ModelViewer ${SOURCES_MODEL_VIEWER})

# Include headers
set(
	INCLUDE_DIRECTORIES
	${PROJECT_SOURCE_DIR}/include
)
target_include_directories(ModelViewer PRIVATE ${INCLUDE_DIRECTORIES})

# Link libraries
set(
	LINK_DIRECTORIES
	${PROJECT_SOURCE_DIR}/lib
)
target_link_directories(ModelViewer PRIVATE ${LINK_DIRECTORIES})
target_link_libraries(ModelViewer glew32 freeglut FreeImage)

# Ensure .dll is with .exe
file(COPY "${LINK_DIRECTORIES}/glew32.dll" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY "${LINK_DIRECTORIES}/freeglut.dll" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY "${LINK_DIRECTORIES}/FreeImage.dll" DESTINATION "${CMAKE_BINARY_DIR}")

# Move assets to .exe
add_custom_target(CopyShaders ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/shaders"
	"${CMAKE_BINARY_DIR}/shaders"
)
add_custom_target(CopyModels ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/models"
	"${CMAKE_BINARY_DIR}/models"
)
add_dependencies(ModelViewer CopyShaders)
add_dependencies(ModelViewer CopyModels)