cmake_minimum_required(VERSION 3.5)

project(SOSForWindows VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)


set(PROJECT_SOURCES
        main.cpp
        sosforwindows.cpp
        PredictThread.cpp
        PredictThread.h
        sosforwindows.h
        sosforwindows.ui
        audiostream.h
        audiostream.cpp
        capturethread.cpp
        capturethread.h
        BufferUtil.h
        BufferUtil.cpp
        base64.cpp
        base64.h
        mainwindow.ui
        mainwindow.cpp
        mainwindow.h
        mydb.cpp
        mydb.h
        mymd5.cpp
        mymd5.h
        MySock.cpp
        MySock.h
        rsa_a.cpp
        rsa_a.h
        SetUp.cpp
        SetUp.h
        Setup.ui
        SOSForWindows.qrc
        SOSForWindows.rc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SOSForWindows
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SOSForWindows APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(SOSForWindows SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SOSForWindows
           WIN32
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SOSForWindows PUBLIC Qt${QT_VERSION_MAJOR}::Widgets wenet_api decoder frontend)
target_compile_definitions(SOSForWindows PRIVATE WIN32_LEAN_AND_MEAN)

set_target_properties(SOSForWindows PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER www.xayytech.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SOSForWindows)
endif()
