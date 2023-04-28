cmake_minimum_required(VERSION 3.16)

project(WordsCounter VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

set(HEADERS
    Controller.h
    Model.h
    Reader.h
)

set(SOURCES
    Controller.cpp
    Model.cpp
    Reader.cpp
)

qt_add_executable(WordsCounter
    main.cpp
    ${SOURCES}
    ${HEADERS}
)

qt_add_qml_module(WordsCounter
    URI WordsCounter
    VERSION 1.0
    QML_FILES main.qml 
)

set_target_properties(WordsCounter PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(WordsCounter
    PRIVATE Qt6::Quick)
