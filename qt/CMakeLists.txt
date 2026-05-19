cmake_minimum_required(VERSION 3.16)
project(EquationLearner VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network OpenGL OpenGLWidgets)

add_executable(EquationLearner
    main.cpp
    graphwidget.cpp
    highlighter.cpp
    explanation.cpp
    llmclient.cpp
    mainwindow.cpp
    mainwindow.ui
)

target_include_directories(EquationLearner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EquationLearner PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

if(WIN32)
    set_target_properties(EquationLearner PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(APPLE)
    set_target_properties(EquationLearner PROPERTIES MACOSX_BUNDLE TRUE)
endif()
