cmake_minimum_required(VERSION 3.16)

project(MaktStark VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    PrintSupport
)

set(SOURCES
    main.cpp
    mainwindow.cpp
    loginwindow.cpp
    database.cpp
    productdialog.cpp
    exportmanager.cpp
)

set(HEADERS
    mainwindow.h
    loginwindow.h
    database.h
    product.h
    productdialog.h
    exportmanager.h
)

add_executable(MaktStark
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(MaktStark PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

set_target_properties(MaktStark PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
