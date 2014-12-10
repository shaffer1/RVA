project(EclipseASCIIReader)
cmake_minimum_required(VERSION 2.8)

if(NOT ParaView_BINARY_DIR)
    find_package(ParaView REQUIRED)
    include(${PARAVIEW_USE_FILE})
endif(NOT ParaView_BINARY_DIR)

add_paraview_plugin(
    EclipseASCIIReader "Eclipse_ASCII_Reader 0.1"
    SERVER_MANAGER_XML EclipseASCIIReader.xml
    SERVER_MANAGER_SOURCES EclipseASCIIReader.cpp
    GUI_RESOURCE_FILES EclipseASCIIReader.xml
)

target_link_libraries(EclipseASCIIReader)

install(TARGETS EclipseASCIIReader
    RUNTIME DESTINATION . COMPONENT EclipseASCIIReader 
    LIBRARY DESTINATION lib
)
