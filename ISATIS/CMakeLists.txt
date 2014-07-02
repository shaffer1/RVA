PROJECT(ISATIS)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

INCLUDE_DIRECTORIES(../common)

IF(PARAVIEW_BUILD_QT_GUI)
  # We need to wrap for Qt stuff such as signals/slots etc. to work correctly.
  #QT4_WRAP_CPP(MOC_SRCS ISATISReaderMenuActions.h ISATISGUIPanel.h)
  QT4_WRAP_CPP(MOC_SRCS ISATISGUIPanel.h)

  # This is a macro for adding QActionGroup subclasses automatically as toolbars.
  #ADD_PARAVIEW_ACTION_GROUP(IFACES IFACE_SRCS
  #                          CLASS_NAME ISATISReaderMenuActions
  #                          GROUP_NAME "MenuBar/RVA")

  # Object panel
  ADD_PARAVIEW_OBJECT_PANEL(IFACES2 IFACE_SRCS2
			    CLASS_NAME ISATISGUIPanel
			    XML_NAME ISATISReaderSource XML_GROUP sources)


  # Includes are relative from source directory
  # Linking is relative to build directory		    
  set(GTXSERVER_INCLUDE_DIR "C:/RVA/GTXserver-2011.5/include" CACHE PATH "GTX Server include directory")
  set(GTXSERVER_LIB_DIR "C:/RVA/GTXserver-2011.5/lib" CACHE PATH "GTX Server include directory")

  include_directories(${GTXSERVER_INCLUDE_DIR})

  if(WIN32)
    if(CMAKE_CL_64)
      link_directories(${GTXSERVER_LIB_DIR}/winnt64/static)
    else()
      link_directories(${GTXSERVER_LIB_DIR}/winnt/static)
    endif()
  elseif(APPLE)
    link_directories(${GTXSERVER_LIB_DIR}/darwin/static)
  else()
    link_directories(${GTXSERVER_LIB_DIR}/linux64-gcc41/static)
  endif()

  SET_SOURCE_FILES_PROPERTIES(
      ISATISReaderDelegate.cxx
  ABSTRACT)

  SET_SOURCE_FILES_PROPERTIES(
      ISATISReaderDelegate.cxx 
      ISATISReaderDefault.cxx
      ISATISReaderGrid.cxx
      ISATISReaderLine.cxx
      ISATISReaderPolygon.cxx
      ISATISFaultProcessor.cxx
  WRAP_EXCLUDE)

  # Important for Build information
  SET(RVA_BUILD_MAJOR "\"0\"")
  SET(RVA_BUILD_MINOR "\"0\"")
  SET(RVA_BUILD_RELEASE "\"1\"")
  SET(RVA_BUILD_NUMBER "\"20120710.01\"")
  SET(RVA_BUILD_DATE "\"20120710\"")
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/common/RVA_Build.h.cmake ${CMAKE_SOURCE_DIR}/common/RVA_Build.h)

  # Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
  # which are filled up by the above macro with relevant entries
  ADD_PARAVIEW_PLUGIN(RVA_ISATIS_Plugin "RC 1.0"
    SERVER_MANAGER_XML 
      ISATISSources.xml
    SERVER_MANAGER_SOURCES 
      ISATISReaderSource.cxx 
      ISATISReaderDelegate.cxx 
      ISATISReaderGrid.cxx 
      ISATISReaderLine.cxx 
      ISATISReaderPolygon.cxx 
      ISATISReaderDefault.cxx
    GUI_RESOURCES 
      ../common/RVAQt.qrc
    GUI_INTERFACES 
      ${IFACES} 
      ${IFACES2}
    GUI_SOURCES
      ${MOC_SRCS}
    SOURCES 
      ${MOC_SRCS} 
      ${IFACE_SRCS} 
      ${IFACE_SRCS2}
      #ISATISReaderMenuActions.cxx
      #ISATISReaderMenuActions.h
      ISATISGUIPanel.cxx
      ISATISGUIPanel.h
      ISATISFaultProcessor.h
      ISATISFaultProcessor.cxx
)




target_link_libraries(RVA_ISATIS_Plugin optimized gtxclient debug gtxclient_debug)

if(WIN32)
target_link_libraries(RVA_ISATIS_Plugin Version.lib) # Version.lib: _GetFileVersionInfoSize used in _st_get_exe_version
endif(WIN32)

INSTALL(TARGETS RVA_ISATIS_Plugin
		RUNTIME DESTINATION . COMPONENT ISATIS
		# For compiling on Linux
		LIBRARY DESTINATION lib
)

set(CPACK_COMPONENT_PVGREADERS_DISPLAY_NAME "RVA ISATIS")  
set(CPACK_COMPONENT_PVGREADERS_DESCRIPTION  "RVA ISATIS")




ENDIF(PARAVIEW_BUILD_QT_GUI)
