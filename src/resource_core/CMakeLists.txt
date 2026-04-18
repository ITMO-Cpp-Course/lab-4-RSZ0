add_library(resource_core STATIC
        include/FileHandle.hpp
        include/ResourceError.hpp
        include/ResourceManager.hpp
        src/FileHandle.cpp
        src/ResourceManager.cpp
        src/ResourceError.cpp
)

target_link_libraries(resource_core PRIVATE project_warnings)
if (TARGET project_sanitizers)
        target_link_libraries(resource_core PRIVATE project_sanitizers)
endif ()

target_include_directories(resource_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
