PROJECT(Core)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

INCLUDE_DIRECTORIES(../common)

# Object panels
ADD_PARAVIEW_OBJECT_PANEL(IFACES3 IFACE_SRCS3
  CLASS_NAME ConnectedThresholdWithCustomSourceGUIPanel
  XML_NAME ConnectedThresholdWithCustomSource XML_GROUP filters)

ADD_PARAVIEW_OBJECT_PANEL(IFACES4 IFACE_SRCS4
  CLASS_NAME ConnectedThresholdGUIPanel
  XML_NAME ConnectedThreshold XML_GROUP filters)

IF(PARAVIEW_BUILD_QT_GUI)
  
  QT4_WRAP_CPP(MOC_SRCS ConnectedThresholdWithCustomSourceGUIPanel.h ConnectedThresholdGUIPanel.h CoreMenuActions.h GeolocationPanel.h zGlobalScale.h)

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
    GUI_RESOURCES 
      ../common/RVAQt.qrc
    GUI_RESOURCE_FILES 
      filters.xml
    GUI_INTERFACES 
      ${IFACES}
      ${IFACES2}
      ${IFACES3}
      ${IFACES4}
    GUI_SOURCES 
      zGlobalScale.cxx 
      ${MOC_SRCS}
    PYTHON_MODULES 
      ${RVA_PYTHON_MODULES}
    SOURCES 
      ${MOC_SRCS} 
      ${IFACE_SRCS}
      ${IFACE_SRCS2}
      ${IFACE_SRCS3}
      ${IFACE_SRCS4}
      CoreMenuActions.cxx
      CoreMenuActions.h
      SumFilter.h
      SumFilter.cxx
      CutBetweenWellsFilter.h
      CutBetweenWellsFilter.cxx
      ConnectedThresholdWithCustomSourceGUIPanel.h
      ConnectedThresholdWithCustomSourceGUIPanel.cxx
      ConnectedThresholdWithCustomSourceFilter.h
      ConnectedThresholdWithCustomSourceFilter.cxx
      ui_pqConnectedThresholdWithCustomSourcePanel.h
      ConnectedThresholdGUIPanel.h
      ConnectedThresholdGUIPanel.cxx
      ConnectedThresholdFilter.h
      ConnectedThresholdFilter.cxx
      ui_pqConnectedThresholdPanel.h
      GeolocationPanel.h
      GeolocationPanel.cxx
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
