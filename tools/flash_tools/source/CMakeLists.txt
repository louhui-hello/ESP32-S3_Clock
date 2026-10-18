cmake_minimum_required(VERSION 3.21)
project(esp32flasher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 SerialPort)

qt_standard_project_setup()

qt_add_executable(esp32flasher
    main.cpp
    src/esptoolrunner.cpp
    src/esptoolrunner.h
    src/serialmonitor.cpp
    src/serialmonitor.h
    src/portlister.cpp
    src/portlister.h
)

qt_add_qml_module(esp32flasher
    URI EspFlasher
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(esp32flasher PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
)

# Windows 下不弹控制台窗口
if(WIN32)
    set_target_properties(esp32flasher PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
