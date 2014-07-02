PROJECT(PVGNonBSDPlugins)
INCLUDE(${PARAVIEW_USE_FILE})

FIND_PACKAGE(Qt4)
IF(QT4_FOUND)

INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
	${VTK_INCLUDE_DIR}
	${PARAVIEW_INCLUDE_DIRS}
	${ParaView_SOURCE_DIR}/VTK/GUISupport/Qt
	${pqComponents_SOURCE_DIR}
	${pqComponents_BINARY_DIR}
	${pqCore_SOURCE_DIR}
	${pqCore_BINARY_DIR}
	${QtWidgets_SOURCE_DIR}
	${QtWidgets_BINARY_DIR}
	${QtChart_SOURCE_DIR}
	${QtChart_BINARY_DIR}
	${QtTesting_SOURCE_DIR}
	${QtTesting_BINARY_DIR}
	${QtPython_SOURCE_DIR}
	${QtPython_BINARY_DIR}
	## Filters
	AnnotateVolume
	ComputeVolumes
	tetgen
	## Readers
	DataMineReader
	../common
	)

SET(PVGPlugins_SM_SRCS_NBSD
	## Filters
	AnnotateVolume/vtkAnnotateVolume.cxx
	ComputeVolumes/vtkComputeVolumes.cxx
	tetgen/vtkTetgen.cxx
	## Readers
	DataMineReader/vtkDataMineDummyReader.cxx
	DataMineReader/vtkDataMineWireFrameReader.cxx
	DataMineReader/vtkDataMinePerimeterReader.cxx
	DataMineReader/vtkDataMinePointReader.cxx
	DataMineReader/vtkDataMineBlockReader.cxx
	DataMineReader/vtkDataMineDrillHoleReader.cxx
	DataMineReader/vtkDataMineReader.cxx
	)
	
SET(PVGPlugins_EXTRA_SRCS_NBSD
	## Filters
	tetgen/tetgen.cxx
	tetgen/predicates.cxx
	## Readers
	DataMineReader/dmfile.cxx
	DataMineReader/PropertyStorage.cxx
	DataMineReader/PointMap.cxx
	)

SET(PVGPlugins_GUI_SRCS_NBSD
	## Filters
	ComputeVolumes/pqComputeVolumes.cxx
	tetgen/pqTetgen.cxx
	## Readers
	DataMineReader/pqDataMineWireFrameReader.cxx
	)
	
SET(MOC_LIST_NBSD
	## Filters
	ComputeVolumes/pqComputeVolumes.h
	tetgen/pqTetgen.h
	## Readers
	DataMineReader/pqDataMineWireFrameReader.h
	)
	
#wrap the moc for the cross platform stuff
QT4_WRAP_CPP(MOC_SRCS_NBSD ${MOC_LIST_NBSD})

ADD_PARAVIEW_OBJECT_PANEL(IFACES_NBSD IFACE_SRCS_NBSD CLASS_NAME pqDataMineWireFrameReader
	XML_NAME DataMineWireFrameReader XML_GROUP sources)
	
ADD_PARAVIEW_OBJECT_PANEL(IFACES1 IFACE_SRCS1 CLASS_NAME pqTetgen
	XML_NAME "Volumique Tetrahedralization" XML_GROUP filters)
						 
LIST(APPEND IFACES_NBSD ${IFACES1})
LIST(APPEND IFACE_SRCS_NBSD ${IFACE_SRCS1})

ADD_PARAVIEW_PLUGIN(PVGNonBSDSMPlugin "1.0"
	SERVER_MANAGER_XML NonBSDPluginsSM.xml
	SERVER_MANAGER_SOURCES  ${PVGPlugins_SM_SRCS_NBSD}
	SERVER_SOURCES  ${PVGPlugins_EXTRA_SRCS_NBSD}
	)
		
ADD_PARAVIEW_PLUGIN(PVGNonBSDGUIPlugin "1.0"
	GUI_INTERFACES ${IFACES_NBSD} 
	GUI_RESOURCES GUI.qrc
	SOURCES ${PVGPlugins_GUI_SRCS_NBSD} ${MOC_SRCS_NBSD} ${IFACE_SRCS_NBSD}
	)
	
ADD_PARAVIEW_PLUGIN(PVGNonBSDCPlugin "1.0"
	GUI_RESOURCE_FILES NonBSDPluginsClient.xml NonBSDPluginsCategory.xml)
	
#CPACK CODE TO BUNDLE THE DLLS CORRECTLY
IF(PVGPlugins_STAND_ALONE_BUILD)
	INSTALL(TARGETS PVGNonBSDGUIPlugin PVGNonBSDSMPlugin PVGNonBSDCPlugin
		RUNTIME DESTINATION . COMPONENT PVGNonBSDPlugins
		)
ENDIF(PVGPlugins_STAND_ALONE_BUILD)		
  
set(CPACK_COMPONENT_PVGPLUGINS_DISPLAY_NAME "ParaViewGeo NBSD Plugins")  
set(CPACK_COMPONENT_PVGPLUGINS_DESCRIPTION 
   "Readers for importing mining data into ParaView")
	
# Windows32 platform
IF(PV_WIN32)
	INCLUDE_DIRECTORIES(
	GeoReferencedImage
	GeoReferencedImage/geotiff/include
	GeoReferencedImage/geotiff/lib
	)
	
	SET( WIN_SM_SRCS_NBSD
		GeoReferencedImage/vtkGeoReferencedImageReader.cxx
		)
		
	FILE(GLOB GEOTIFF_EXTRAS1 "${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/include/*.c")
	FILE(GLOB GEOTIFF_EXTRAS2 "${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/include/*.h")
	FILE(GLOB GEOTIFF_EXTRAS3 "${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/lib/*.lib")
	
	SET( WIN_SM_EXTRA_SRCS_NBSD
		${GEOTIFF_EXTRAS1}
		${GEOTIFF_EXTRAS2}
		${GEOTIFF_EXTRAS3}
		)

	SET( WIN_GUI_SRCS_NBSD
		GeoReferencedImage/pqGeoReferencedImageReader.cxx
		)

	SET( WIN_MOC_LIST_NBSD
		GeoReferencedImage/pqGeoReferencedImageReader.h
		)
		
	QT4_WRAP_CPP(WIN_MOC_SRCS_NBSD ${WIN_MOC_LIST_NBSD})

	ADD_PARAVIEW_OBJECT_PANEL(WIN_IFACES_NBSD WIN_IFACE_SRCS_NBSD CLASS_NAME pqGeoReferencedImageReader
		XML_NAME GeoReferencedImageReader XML_GROUP sources)
					  
	ADD_PARAVIEW_PLUGIN(WindowsReadersSMPluginNBSD "1.0"
		SERVER_MANAGER_XML NonBSDWindowsPluginsSM.xml
		SERVER_MANAGER_SOURCES  ${WIN_SM_SRCS_NBSD}
		SERVER_SOURCES ${WIN_SM_EXTRA_SRCS_NBSD}
		)
		
	ADD_PARAVIEW_PLUGIN(WindowsReadersCPluginNBSD "1.0"
		GUI_RESOURCE_FILES NonBSDWindowsPluginsClient.xml
		)

	ADD_PARAVIEW_PLUGIN(WindowsReadersGUIPluginNBSD "1.0"
		GUI_INTERFACES ${WIN_IFACES_NBSD}
		GUI_RESOURCES WinGUI.qrc
		SOURCES ${WIN_GUI_SRCS_NBSD} ${WIN_MOC_SRCS_NBSD} ${WIN_IFACE_SRCS_NBSD}
		)
		
	#CPACK CODE TO BUNDLE THE DLLS CORRECTLY
	IF(PVGPlugins_STAND_ALONE_BUILD)
		INSTALL(TARGETS WindowsReadersSMPluginNBSD WindowsReadersCPluginNBSD WindowsReadersGUIPluginNBSD
			RUNTIME DESTINATION . COMPONENT WindowsReadersSMPluginNBSD
			)
	ENDIF(PVGPlugins_STAND_ALONE_BUILD)

	LINK_DIRECTORIES ( ${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/lib )

	# LINK GEOTIFF LIBRARIES
	target_link_libraries (WindowsReadersSMPlugin ${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/lib/geotiff_i.lib )
	target_link_libraries (WindowsReadersSMPlugin ${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/lib/libtiff_i.lib )
	  
	#SPECIAL CODE JUST FOR GEOTIFF
	#TO COPY THE EXTRA DLLS/INCLUDES OVER	
	FILE(GLOB CPACK_GEOTIFF_LIB "${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/lib/*.lib")
	FILE(GLOB CPACK_GEOTIFF_DLL "${CMAKE_CURRENT_SOURCE_DIR}/GeoReferencedImage/geotiff/bin/*.dll")
	LIST(APPEND CPACK_GEOTIFF_EXTRAS ${CPACK_GEOTIFF_LIB} ${CPACK_GEOTIFF_DLL}) 
	
	INSTALL(FILES ${CPACK_GEOTIFF_EXTRAS}
		DESTINATION bin
		COMPONENT Runtime
		)
ENDIF(PV_WIN32)
ENDIF(QT4_FOUND)