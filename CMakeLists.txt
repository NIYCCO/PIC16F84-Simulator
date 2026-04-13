cmake_minimum_required(VERSION 3.20)

project(pic16F84_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/glfw)

find_package(OpenGL REQUIRED)

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    external
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

set(SOURCE_FILES
    src/simulator/SimulationInterface.h
    src/simulator/PIC16F84.h
    src/simulator/components/instructions.h
    src/simulator/components/CPU.h
    src/simulator/components/ProgramMemory.h
    src/simulator/components/DataMemory.h
    src/ui/editor.h

    src/main.cpp
    src/simulator/SimulationInterface.cpp
    src/simulator/PIC16F84.cpp
    src/simulator/components/CPU.cpp
    src/simulator/components/ProgramMemory.cpp
    src/simulator/components/DataMemory.cpp
    src/ui/editor.cpp
    external/TextEditor.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE external)

target_link_libraries(pic16F84_sim PRIVATE imgui)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"

    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini"
)