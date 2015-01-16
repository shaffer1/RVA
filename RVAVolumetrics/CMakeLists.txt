project(RVAVolumetrics)
cmake_minimum_required(VERSION 2.8)

if(NOT ParaView_BINARY_DIR)
    find_package(ParaView REQUIRED)
    include(${PARAVIEW_USE_FILE})
endif(NOT ParaView_BINARY_DIR)

add_paraview_plugin(
    RVAVolumetrics "RVA_Volumetrics 0.1"
    SERVER_MANAGER_XML RVAVolumetrics.xml
    SERVER_MANAGER_SOURCES RVAVolumetrics.cxx
    SOURCES RVAVolumetrics.h RVAVolumetrics.cxx
    )

target_link_libraries(RVAVolumetrics)

install(TARGETS RVAVolumetrics
    RUNTIME DESTINATION . COMPONENT RVAVolumetrics
    LIBRARY DESTINATION lib
    )

if(WIN32)
    target_link_libraries(RVAVolumetrics Version.lib)
endif(WIN32)
