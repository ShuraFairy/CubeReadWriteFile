cmake_minimum_required(VERSION 3.16)
project(CubeReadWriteFile VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(HEADERS
    src/mainwindow.h
    src/fileworker.h
    src/glwidget.h
)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/fileworker.cpp
    src/glwidget.cpp
)

# qt6_add_resources(PROJECT_SOURCES shaders.qrc)
qt6_add_resources(SOURCES shaders.qrc)

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION
    ${HEADERS}
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Core
    Qt6::Widgets
    Qt6::Core5Compat
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
