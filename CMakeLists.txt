cmake_minimum_required(VERSION 3.21)
project(Sandbox2D LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(SFML 3 COMPONENTS Graphics Window System CONFIG REQUIRED)
add_executable(Sandbox2D
    main.cpp
    GameTypes.hpp
    Inventory.cpp
    World.cpp
    Player.cpp
    Mining.cpp
    Rendering.cpp
    Tooltips.cpp
    Menu.cpp
    SaveSystem.cpp
    SaveSystem.hpp
    Assets.cpp
)
target_link_libraries(Sandbox2D PRIVATE SFML::Graphics SFML::Window SFML::System)
set_target_properties(Sandbox2D PROPERTIES WIN32_EXECUTABLE FALSE)
add_custom_command(TARGET Sandbox2D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:Sandbox2D>/assets"
    COMMENT "Copying block textures")

if(MSVC)
    target_compile_options(Sandbox2D PRIVATE /utf-8)
endif()
