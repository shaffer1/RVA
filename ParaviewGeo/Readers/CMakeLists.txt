PROJECT(PVGReaders)
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
ArcMapReader
BlockModelReader
DrillHoleReader
DXFReader
Ellipsoid
#EPSReader
FDEM
FLAC3DReader
3DECReader
GocadReader
GXFReader
Map3dBlockReader
PointSetReader
SGeMSReader
ShapeFileReader
SOTResult
TriReader
LineNetworkReader

../common

)

SET( PVGReaders_SM_SRCS
ArcMapReader/vtkArcMapReader.cxx
BlockModelReader/vtkBlockModelReader.cxx
DrillHoleReader/vtkDrillHoleReader.cxx
DXFReader/vtkDXFReader.cxx
Ellipsoid/vtkEllipsoid.cxx 
Ellipsoid/vtkElliptic.cxx
#EPSReader/vtkEPSReader.cxx
FDEM/vtkFDEMReader.cxx
FLAC3DReader/vtkFLAC3DReader.cpp
3DECReader/vtk3DECReader.cpp
GocadReader/vtkGocadReader.cxx 
GocadReader/vtkGocadAtomic.cxx 
GocadReader/vtkGocadTSurf.cxx 
GocadReader/vtkGocadPLine.cxx 
GocadReader/vtkGocadVSet.cxx 
GocadReader/vtkGocadTSolid.cxx 
GocadReader/vtkGocadVoxet.cxx
GocadReader/vtkGocadSGrid.cxx
GXFReader/vtkGXFReader.cxx
Map3dBlockReader/vtkMap3dBlockReader.cxx
PointSetReader/vtkPointSetReader.cpp
SGeMSReader/vtkSGeMSReader.cxx
SGeMSReader/vtkSGeMSCgridReader.cxx
ShapeFileReader/vtkShapeFileReader.cxx
SOTResult/vtkSOTResult.cxx
TriReader/vtkTriReader.cxx
LineNetworkReader/vtkLineNetworkReader.cpp
)

SET( PVGReaders_EXTRA_SRCS
DXFReader/vtkDXFObject.cxx 
DXFReader/vtkDXFLayer.cxx 
DXFReader/vtkDXFParser.cxx 
DXFReader/vtkDXFBlock.cxx
DXFReader/vtkDXFObjectMap.cxx 
FDEM/FDEMStorage.cxx 
FDEM/FDEMFill.cxx 
FDEM/FDEMPoint.cxx 
FDEM/FDEMCrackJoint.cxx 
FDEM/TriangleProperties.cxx 
GocadReader/GocadColorMap.cxx
ShapeFileReader/shpread.c
ShapeFileReader/dimension.cxx
ShapeFileReader/hgrid.cxx
ShapeFileReader/hpolygon.cxx
ShapeFileReader/mgconst.cxx
ShapeFileReader/mgexcept.cxx

../common/StringUtilities.cpp
)

SET(PVGReaders_GUI_SRCS
DrillHoleReader/pqDrillHoleReader.cxx
#EPSReader/pqEPSReader.cxx
FLAC3DReader/pqFLAC3DReader.cpp
3DECReader/pq3DECReader.cpp
GocadReader/pqGocadReader.cxx
PointSetReader/pqPointSetReader.cxx
SOTResult/pqSOTResult.cxx
LineNetworkReader/pqLineNetworkReader.cxx
)

SET(MOC_LIST
DrillHoleReader/pqDrillHoleReader.h
#EPSReader/pqEPSReader.h
FLAC3DReader/pqFLAC3DReader.h
3DECReader/pq3DECReader.h
GocadReader/pqGocadReader.h
PointSetReader/pqPointSetReader.h
SOTResult/pqSOTResult.h
LineNetworkReader/pqLineNetworkReader.h
)

#wrap the moc for the cross platform stuff
QT4_WRAP_CPP(MOC_SRCS ${MOC_LIST})
                         
ADD_PARAVIEW_OBJECT_PANEL(IFACES IFACE_SRCS CLASS_NAME pqSOTResult
                         XML_NAME SOTResult XML_GROUP sources)
                         
ADD_PARAVIEW_OBJECT_PANEL(IFACES1 IFACE_SRCS1 CLASS_NAME pqDrillHoleReader
                         XML_NAME DrillHoleReader XML_GROUP sources)
						 
ADD_PARAVIEW_OBJECT_PANEL(IFACES2 IFACE_SRCS2 CLASS_NAME pqPointSetReader
                         XML_NAME PointSetReader XML_GROUP sources)	

ADD_PARAVIEW_OBJECT_PANEL(IFACES3 IFACE_SRCS3 CLASS_NAME pqGocadReader
                         XML_NAME GocadReader XML_GROUP sources)	

ADD_PARAVIEW_OBJECT_PANEL(IFACES4 IFACE_SRCS4 CLASS_NAME pqLineNetworkReader
                         XML_NAME LineNetworkReader XML_GROUP sources)		
		
ADD_PARAVIEW_OBJECT_PANEL(IFACES5 IFACE_SRCS5 CLASS_NAME pqFLAC3DReader
                         XML_NAME FLAC3DReader XML_GROUP sources)				

ADD_PARAVIEW_OBJECT_PANEL(IFACES6 IFACE_SRCS6 CLASS_NAME pq3DECReader
                         XML_NAME 3DECReader XML_GROUP sources)																 

# Append DrillHoldReader
LIST(APPEND IFACES ${IFACES1})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS1})
# Append PointSetReader
LIST(APPEND IFACES ${IFACES2})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS2})
# Append GocadReader
LIST(APPEND IFACES ${IFACES3})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS3})
# Append LineNetworkReader
LIST(APPEND IFACES ${IFACES4})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS4})
#Append FLAC3DReader
LIST(APPEND IFACES ${IFACES5})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS5})
#Append 3DECReader
LIST(APPEND IFACES ${IFACES6})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS6})

ADD_PARAVIEW_PLUGIN(PVGReadersSMPlugin "1.0"
			SERVER_MANAGER_XML ReadersSM.xml
			SERVER_MANAGER_SOURCES  ${PVGReaders_SM_SRCS}
			SERVER_SOURCES  ${PVGReaders_EXTRA_SRCS}
			)
			
ADD_PARAVIEW_PLUGIN(PVGReadersGUIPlugin "1.0"
             GUI_INTERFACES ${IFACES} 
             GUI_RESOURCES GUI.qrc
             SOURCES ${PVGReaders_GUI_SRCS} ${MOC_SRCS} ${IFACE_SRCS}
			)

ADD_PARAVIEW_PLUGIN(PVGReadersCPlugin "1.0"
	GUI_RESOURCE_FILES ReadersClient.xml)
	
##############################################################
# Oracle Reader
#OPTION(PVG_BUILD_ORACLE "Build Oracle Reader Plugin" OFF)
#IF (PVG_BUILD_ORACLE)
	#set(ORACLE_LIB "C:/"
	#CACHE PATH "Select the ORACLE lib")
	#target_link_libraries (PVGReadersSMPlugin ${ORACLE_LIB} )	
	
	#set(ORACLE_INCLUDE_DIR "C:/"
	#CACHE PATH "Select the ORACLE include directory")
	#INCLUDE_DIRECTORIES(${ORACLE_INCLUDE_DIR})
	
	#INCLUDE_DIRECTORIES(OracleReader)
	#ADD_SUBDIRECTORY(OracleReader)
	
#ENDIF(PVG_BUILD_ORACLE)
##############################################################	

##############################################################
# ODBC Reader
#OPTION(PVG_BUILD_ODBC "Build ODBC Reader Plugin" OFF)
#IF (PVG_BUILD_ODBC)
	#ADD_SUBDIRECTORY(ODBCReader)
#ENDIF(PVG_BUILD_ODBC)
##############################################################	

#CPACK CODE TO BUNDLE THE DLLS CORRECTLY
IF(PVGPlugins_STAND_ALONE_BUILD)
	INSTALL(TARGETS PVGReadersGUIPlugin PVGReadersSMPlugin PVGReadersCPlugin
		RUNTIME DESTINATION . COMPONENT PVGReaders
		)
ENDIF(PVGPlugins_STAND_ALONE_BUILD)
  
set(CPACK_COMPONENT_PVGREADERS_DISPLAY_NAME "ParaViewGeo Readers")  
set(CPACK_COMPONENT_PVGREADERS_DESCRIPTION 
   "Readers for importing mining data into ParaView")

ENDIF(QT4_FOUND)