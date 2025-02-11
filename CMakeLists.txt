cmake_minimum_required(VERSION 3.30)
project(gaming_rpg)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)
FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
)
FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x
)
FetchContent_Declare(VulkanHeaders
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG v1.4.305
)
FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)

set(SPDLOG_USE_STD_FORMAT ON)
FetchContent_MakeAvailable(glm spdlog VulkanHeaders glfw)

set(IMGUI_SOURCES imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp)

set(GAME_SOURCES src/game/main.cpp
        src/engine/application.cpp
        src/engine/application.hpp
        src/engine/window.cpp
        src/engine/window.hpp
        src/engine/tools.cpp
        src/engine/tools.hpp
        src/game/game.cpp
        src/game/game.hpp
        src/engine/imgui/imgui_backend.cpp
        src/engine/imgui/imgui_backend.hpp
)

add_executable(gaming_rpg ${GAME_SOURCES} ${IMGUI_SOURCES})
target_include_directories(gaming_rpg PRIVATE src/ imgui/ rapidxml/)
target_link_libraries(gaming_rpg PRIVATE glfw glm::glm spdlog::spdlog Vulkan::Headers)

if (WIN32)
    target_link_libraries(gaming_rpg PRIVATE Dwmapi)
endif ()

target_compile_definitions(gaming_rpg PRIVATE GLFW_INCLUDE_NONE GLFW_INCLUDE_VULKAN)