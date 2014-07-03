PROJECT(UTChem)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

INCLUDE_DIRECTORIES(../common)

IF(PARAVIEW_BUILD_QT_GUI)

  SET_SOURCE_FILES_PROPERTIES(
      UTChemAsciiReader.cxx
  ABSTRACT)

  SET_SOURCE_FILES_PROPERTIES(
      UTChemInputReader.cxx
      UTChemTopReader.cxx
  WRAP_EXCLUDE)


  # Now crete a plugin for the toolbar. Here we pass IFACES and IFACE_SRCS
  # which are filled up by the above macro with relevant entries
  ADD_PARAVIEW_PLUGIN(RVA_UTChemReaders_Plugin "RC_UTChem 1.1"
    SERVER_MANAGER_XML 
      UTChemReader.xml
    SERVER_MANAGER_SOURCES
      UTChemConcReader.cxx 
      UTChemAsciiReader.cxx 
      UTChemWellReader.cxx 
      UTChemFluxReader.cxx
    GUI_RESOURCE_FILES
      UTChemReaderGUI.xml
    SOURCES
      UTChemInputReader.cxx
      UTChemInputReader.h
      UTChemTopReader.cxx
      UTChemTopReader.h
      UTChemAsciiReader.cxx
      UTChemAsciiReader.h
      UTChemFluxReader.h
      UTChemFluxReader.cxx
      UTChemWellReader.h
      UTChemWellReader.cxx
)


INSTALL(TARGETS RVA_UTChemReaders_Plugin
		RUNTIME DESTINATION . COMPONENT UTChem
		# for compiling on Linux
		LIBRARY DESTINATION lib
)

set(CPACK_COMPONENT_PVGREADERS_DISPLAY_NAME "RVA UTChem")  
set(CPACK_COMPONENT_PVGREADERS_DESCRIPTION  "RVA UTChem")

#target_link_libraries(RVA_UTChemReaders_Plugin)

#if(WIN32)
#target_link_libraries(RVA_UTChemReaders_Plugin Version.lib) # Version.lib: _GetFileVersionInfoSize used in #_st_get_exe_version
#endif(WIN32)




ENDIF(PARAVIEW_BUILD_QT_GUI)
