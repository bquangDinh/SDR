cmake_minimum_required(VERSION 3.16)

project(sdr_qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Graphs Multimedia)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appsdr_qt
    src/main.cpp


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
        qml/components/SdrFrequencyBox.qml
    SOURCES
        src/core/sdr_backend.h
        src/core/sdr_backend.cpp
        src/core/devices/rtl_sdr_device.h
        src/core/devices/rtl_sdr_device.cpp
        src/core/sdr_device_manager.h
        src/core/sdr_device_manager.cpp
        src/core/Constants.h
        src/core/singleton.h
        src/core/waterfall.h
        src/core/waterfall.cpp
        src/core/rtl_audio_thread.h
        src/core/rtl_audio_thread.cpp
        src/core/fast_decimator.h
        src/core/fast_decimator.cpp
        src/core/chunkqueue.h
        src/core/chunkqueue.cpp
        src/core/latestchunk.h
        src/core/latestchunk.cpp
        src/core/rtl_reader_thread.h
        src/core/rtl_reader_thread.cpp
        src/core/rtl_fft_thread.h
        src/core/rtl_fft_thread.cpp
        src/core/spectrum.h
        src/core/spectrum.cpp
        src/core/Signal.h
        src/core/Signal.cpp
    RESOURCES
        fonts/Figtree-VariableFont_wght.ttf
        SOURCES
        SOURCES src/core/mixer.h src/core/mixer.cpp
        SOURCES src/core/audiooutput.h src/core/audiooutput.cpp
        SOURCES src/core/demodulators/demodulator.h src/core/demodulators/demodulator.cpp
        SOURCES src/core/demodulators/nfmdemod.h src/core/demodulators/nfmdemod.cpp
        SOURCES src/core/fm_discriminator.h src/core/fm_discriminator.cpp
        SOURCES src/core/demodulators/amdemod.h src/core/demodulators/amdemod.cpp
        SOURCES src/core/am_discriminator.h src/core/am_discriminator.cpp
        SOURCES src/core/demodulators/wfmdemod.h src/core/demodulators/wfmdemod.cpp
        SOURCES src/core/demodulators/cwdemod.h src/core/demodulators/cwdemod.cpp
        SOURCES src/core/demodulators/usbdemod.h src/core/demodulators/usbdemod.cpp
        SOURCES src/core/demodulators/lsbdemod.h src/core/demodulators/lsbdemod.cpp
        SOURCES src/core/demodulators/dsbdemod.h src/core/demodulators/dsbdemod.cpp
        SOURCES src/core/demodulators/rawdemod.h src/core/demodulators/rawdemod.cpp
        SOURCES src/core/fps_counter.h
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

add_library(fftw3f SHARED IMPORTED)
set_target_properties(fftw3f PROPERTIES
    IMPORTED_IMPLIB               ${CMAKE_SOURCE_DIR}/lib/libfftw3f-3.a      # for the linker
    IMPORTED_LOCATION             ${CMAKE_SOURCE_DIR}/lib/libfftw3f-3.dll    # for runtime
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include                # for fftw3.h
)

target_link_libraries(appsdr_qt PRIVATE
    rtlsdr
    fftw3f-3
    Qt6::Quick
    Qt6::Graphs
    Qt6::Multimedia
)

install(TARGETS appsdr_qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_command(TARGET appsdr_qt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:fftw3f>
            $<TARGET_FILE_DIR:appsdr_qt>
    COMMENT "Copying libfftw3f-3.dll next to the executable")
