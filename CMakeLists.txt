cmake_minimum_required(VERSION 3.16)

project(sdr_qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Graphs)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appsdr_qt
    src/main.cpp
    src/core/rtl_thread.cpp
    src/core/rtl_thread.h
    src/core/devices/sdr_device.cpp
    src/core/devices/sdr_device.h
)

target_include_directories(appsdr_qt PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/devices
)

target_link_directories(appsdr_qt PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)

qt_add_qml_module(appsdr_qt
    URI sdr_qt
    QML_FILES
        qml/Main.qml
        qml/components/Select.qml
        qml/components/SdrCheckBox.qml
        qml/components/SdrSlider.qml
        qml/components/SdrRadioBox.qml
        qml/components/SdrPlayBtn.qml
    SOURCES
        src/core/sdr_backend.h
        src/core/sdr_backend.cpp
        src/core/devices/rtl_sdr_device.h
        src/core/devices/rtl_sdr_device.cpp
        src/core/sdr_device_manager.h
        src/core/sdr_device_manager.cpp
        src/core/Constants.h
        src/core/singleton.h
    RESOURCES
        fonts/Figtree-VariableFont_wght.ttf
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appsdr_qt PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appsdr_qt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appsdr_qt PRIVATE
    rtlsdr
    Qt6::Quick
    Qt6::Graphs
)

install(TARGETS appsdr_qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
