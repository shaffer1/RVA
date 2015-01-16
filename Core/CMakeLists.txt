PROJECT(Core)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

INCLUDE_DIRECTORIES(../common)


IF(PARAVIEW_BUILD_QT_GUI)
  
  QT4_WRAP_CPP(MOC_SRCS CoreMenuActions.h GeolocationPanel.h zGlobalScale.h)

  ADD_PARAVIEW_ACTION_GROUP(IFACES IFACE_SRCS
    CLASS_NAME CoreMenuActions
    GROUP_NAME "MenuBar/RVA")
			    
  ADD_PARAVIEW_AUTO_START(IFACES2 IFACE_SRCS2
   CLASS_NAME zGlobalScale
   STARTUP onStartup
   SHUTDOWN onShutdown)

  # Is this sufficient to check for python?
  IF(VTK_WRAP_PYTHON) 
  SET(RVA_PYTHON_MODULES 
      RVAMacros/__init__.py
      RVAMacros/Tubify.py
      RVAMacros/ContextView.py)
  ENDIF(VTK_WRAP_PYTHON)

  # Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
  # which are filled up by the above macro with relevant entries
  ADD_PARAVIEW_PLUGIN(RVA_Core_Plugin "RC 1.0"
    SERVER_MANAGER_XML 
      sources.xml
    SERVER_MANAGER_SOURCES
      CutBetweenWellsFilter.cxx
      SumFilter.cxx
      ConnectedThresholdWithCustomSourceFilter.cxx
      ConnectedThresholdFilter.cxx
      RVAVolumetrics.cxx
    GUI_RESOURCES 
      ../common/RVAQt.qrc
    GUI_RESOURCE_FILES 
      filters.xml
    GUI_INTERFACES 
      ${IFACES}
      ${IFACES2}
    GUI_SOURCES 
      zGlobalScale.cxx 
      ${MOC_SRCS}
    PYTHON_MODULES 
      ${RVA_PYTHON_MODULES}
    SOURCES 
      ${MOC_SRCS} 
      ${IFACE_SRCS}
      ${IFACE_SRCS2}
      CoreMenuActions.cxx
      CoreMenuActions.h
      SumFilter.h
      SumFilter.cxx
      CutBetweenWellsFilter.h
      CutBetweenWellsFilter.cxx
      ConnectedThresholdWithCustomSourceFilter.h
      ConnectedThresholdWithCustomSourceFilter.cxx
      ConnectedThresholdFilter.h
      ConnectedThresholdFilter.cxx
      GeolocationPanel.h
      GeolocationPanel.cxx
      RVAVolumetrics.h
      RVAVolumetrics.cxx
)

IF(WIN32)
TARGET_LINK_LIBRARIES(RVA_Core_Plugin Version.lib) # Version.lib: _GetFileVersionInfoSize used in _st_get_exe_version
ENDIF(WIN32)

INSTALL(TARGETS RVA_Core_Plugin
		RUNTIME DESTINATION . COMPONENT Core
		# For compiling on Linux, works, but might not be best option
		LIBRARY DESTINATION lib
)

set(CPACK_COMPONENT_PVGREADERS_DISPLAY_NAME "RVA Core")  
set(CPACK_COMPONENT_PVGREADERS_DESCRIPTION  "RVA Core")

#target_link_libraries(RVA_Core_Plugin)

ENDIF(PARAVIEW_BUILD_QT_GUI)
