#C:/RVA/builds/Paraview-3.14.1_msvc_32

PROJECT(ShapefileReader)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(NOT ParaView_BINARY_DIR)
 	FIND_PACKAGE(ParaView REQUIRED)
 	INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)


ADD_PARAVIEW_PLUGIN(

	ShapefileReader "SHP_Reader 0.1"
  	
  	SERVER_MANAGER_XML 
  		ShapefileReader.xml

  	SERVER_MANAGER_SOURCES
  		ShapefileReader.cpp

  	GUI_RESOURCE_FILES 
  		ShapefileReaderGUI.xml

  	SOURCES
  		shapefil.h
  		dbfopen.c
  		safileio.c
  		shpopen.c
  		shptree.c

  		ShapefileReader.h
  		ShapefileReader.cpp
 )

TARGET_LINK_LIBRARIES(ShapefileReader)

install(TARGETS ShapefileReader
    RUNTIME DESTINATION . COMPONENT ShapefileReader
    LIBRARY DESTINATION lib
    )

IF(WIN32)
	TARGET_LINK_LIBRARIES(ShapefileReader Version.lib) 
	# Version.lib: _GetFileVersionInfoSize used in _st_get_exe_version
ENDIF(WIN32)


