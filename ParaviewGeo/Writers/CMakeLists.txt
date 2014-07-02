PROJECT(PVGWriters)
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
GocadWriter
SGeMSWriter
SOTWriter
Map3dWriter
)

SET( PVGWriters_SM_SRCS
GocadWriter/vtkGocadCompositeWriter.cxx
GocadWriter/vtkGocadBase.cxx
GocadWriter/vtkGocadWriter.cxx
GocadWriter/vtkGocadVoxetWriter.cxx
SGeMSWriter/vtkSGeMSWriter.cxx
SOTWriter/vtkSOTWriter.cxx
)

ADD_PARAVIEW_PLUGIN(PVGWritersSMPlugin "1.0"
			SERVER_MANAGER_XML WritersSM.xml
			SERVER_MANAGER_SOURCES  ${PVGWriters_SM_SRCS}
			)
			
			
ADD_PARAVIEW_PLUGIN(PVGWritersCPlugin "1.0"
	GUI_RESOURCE_FILES WritersClient.xml) #the name here cannot clash with any other xml files name!
ENDIF(QT4_FOUND)


  
#CPACK CODE TO BUNDLE THE DLLS CORRECTLY
IF(PVGPlugins_STAND_ALONE_BUILD)
	INSTALL(TARGETS PVGWritersSMPlugin PVGWritersCPlugin
		RUNTIME DESTINATION . COMPONENT PVGWriters
	)
ENDIF(PVGPlugins_STAND_ALONE_BUILD)

set(CPACK_COMPONENT_PVGWRITERS_DISPLAY_NAME "ParaViewGeo Writers")  
set(CPACK_COMPONENT_PVGWRITERS_DESCRIPTION 
   "Set of Writers that allow exporting data from ParaView to different file formats")

