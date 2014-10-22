project(ZMapPlusReader)
cmake_minimum_required(VERSION 2.8)

if(NOT ParaView_BINARY_DIR)
    find_package(ParaView REQUIRED)
    include(${PARAVIEW_USE_FILE})
endif(NOT ParaView_BINARY_DIR)

add_paraview_plugin(
    ZMapPlusReader "ZMapPlus_Reader 0.1"
    SERVER_MANAGER_XML ZMapPlusReader.xml
    SERVER_MANAGER_SOURCES ZMapPlusReader.cpp
    GUI_RESOURCE_FILES ZMapPlusReaderGUI.xml
)

target_link_libraries(ZMapPlusReader)

install(TARGETS ZMapPlusReader
    RUNTIME DESTINATION . COMPONENT ZMapPlusReader
    LIBRARY DESTINATION lib
)
