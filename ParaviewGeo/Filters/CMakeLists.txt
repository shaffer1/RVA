PROJECT(PVGFilters)
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
ApplySchedule
BoundingBox
CleanCells
ConvertPointsToBlocks
ExtractDataType
FitDataset
InsideClosedSurface
IntersectWireframe
MapAttributes
Mine24DtoMap3D
#PlotPropertiesOverTime
OBBtoGrid
PolyLineAddPoints
PolyToStruct
ProximityThreshold
ProximityToLines
RadialDistance
RegexThreshold
RelativeTransform
SequenceArrows
SharedCode
SumBillboard
SurfaceReconstructor
TensorGlyph
TimeBillboard
TimeThreshold
TopOfCell
VTKBultIn
Ruler
StringArrayToIndex
../common
)

SET( PVGFilters_SM_SRCS
ApplySchedule/vtkApplySchedule.cxx
BoundingBox/vtkBBox.cxx
CleanCells/vtkCleanCells.cxx
ConvertPointsToBlocks/vtkConvertPointsToBlocks.cxx
ExtractDataType/vtkExtractPolyData.cxx
ExtractDataType/vtkExtractUnstrucGrid.cxx
FitDataset/vtkFitDataset.cpp
InsideClosedSurface/vtkInsideClosedSurface.cxx
IntersectWireframe/vtkIntersectFilter.cxx 
IntersectWireframe/vtkIntersectWireframe.cxx
IntersectWireframe/vtkThresholdProcessId.cxx
MapAttributes/vtkMapAttributesToGrid.cxx
MapAttributes/vtkMapAttributesFromGrid.cxx
MapAttributes/vtkMapAttributes.cxx 
Mine24DtoMap3D/vtkMine24DtoMap3D.cpp
#PlotPropertiesOverTime/vtkPlotPropertiesOverTime.cxx
OBBtoGrid/vtkOBBtoGrid.cpp
PolyLineAddPoints/vtkPolyLineAddPoints.cxx
PolyToStruct/vtkPolyStruct.cxx
ProximityThreshold/vtkProximityThreshold.cxx
ProximityToLines/vtkProximityToLines.cpp
RadialDistance/vtkRadialDistance.cxx
RegexThreshold/vtkRegexThreshold.cxx
RelativeTransform/vtkRelativeTransform.cxx
SequenceArrows/vtkSequenceArrows.cxx
SumBillboard/vtkSumBillboard.cxx
SurfaceReconstructor/vtkSurfaceReconstructor.cxx
TensorGlyph/vtkTensorGlyphFilter.cxx
TimeBillboard/vtkTimeBillboard.cxx
TimeThreshold/vtkTimeThreshold.cxx
TopOfCell/vtkTopOfCell.cxx
Ruler/vtkRuler.cxx
StringArrayToIndex/vtkStringArrayToIndex.cpp
)

SET( PVGFilters_EXTRA_SRCS
SharedCode/TimeConverter.cxx
../common/vtkCoordinateSystemMapper.cpp
FitDataset/vtkConvexHull.cpp
FitDataset/vtkLeastSquareFit.cpp
Mine24DtoMap3D/Face.cpp
Mine24DtoMap3D/Segments.cpp
)

SET( PVGFilters_GUI_SRCS
ApplySchedule/pqApplySchedule.cxx
BoundingBox/pqBoundingBox.cxx 
ConvertPointsToBlocks/pqConvertPointsToBlocks.cxx
RegexThreshold/pqRegexThreshold.cxx
SumBillboard/pqSumBillboardPanel.cxx
TimeThreshold/pqTimeThreshold.cxx
#PlotPropertiesOverTime/pqPlotPropertiesOverTime.cxx
#PlotPropertiesOverTime/pqPlotPropertiesOverTimePanel.cxx
#PlotPropertiesOverTime/pqLabelFormatter.cxx
RadialDistance/pqRadialDistanceWidget.cxx
SharedCode/TimeConverter.cxx
Ruler/pqRuler.cxx
Mine24DtoMap3D/pqMine24DtoMap3D.cxx
)

SET(MOC_LIST
ApplySchedule/pqApplySchedule.h
BoundingBox/pqBoundingBox.h
ConvertPointsToBlocks/pqConvertPointsToBlocks.h
RegexThreshold/pqRegexThreshold.h
SumBillboard/pqSumBillboardPanel.h
TimeThreshold/pqTimeThreshold.h
RadialDistance/pqRadialDistanceWidget.h
Ruler/pqRuler.h
Mine24DtoMap3D/pqMine24DtoMap3D.h
)

QT4_WRAP_CPP(MOC_SRCS ${MOC_LIST})

ADD_PARAVIEW_OBJECT_PANEL(IFACES IFACE_SRCS CLASS_NAME pqApplySchedule
                        XML_NAME "Apply Schedule" XML_GROUP filters)

ADD_PARAVIEW_OBJECT_PANEL(IFACES2 IFACE_SRCS2 CLASS_NAME pqBoundingBox
                        XML_NAME "Bounding Box" XML_GROUP filters)
                                     
ADD_PARAVIEW_OBJECT_PANEL(IFACES3 IFACE_SRCS3 CLASS_NAME pqRegexThreshold
                        XML_NAME RegexThreshold XML_GROUP filters)

ADD_PARAVIEW_OBJECT_PANEL(IFACES4 IFACE_SRCS4 CLASS_NAME pqSumBillboardPanel
                        XML_NAME SumBillboard XML_GROUP filters)  
                                                   
ADD_PARAVIEW_OBJECT_PANEL(IFACES5 IFACE_SRCS5 CLASS_NAME pqTimeThreshold
                        XML_NAME TimeThreshold XML_GROUP filters)
                                                 
ADD_PARAVIEW_OBJECT_PANEL(IFACES6 IFACE_SRCS6 CLASS_NAME pqRadialDistanceWidget
                        XML_NAME "Radial Distance" XML_GROUP filters)

ADD_PARAVIEW_OBJECT_PANEL(IFACES7 IFACE_SRCS7 CLASS_NAME pqConvertPointsToBlocks
                        XML_NAME ConvertPointsToBlocks XML_GROUP filters)

ADD_PARAVIEW_OBJECT_PANEL(IFACES8 IFACE_SRCS8 CLASS_NAME 	pqRuler
						XML_NAME "Ruler" XML_GROUP filters)    

ADD_PARAVIEW_OBJECT_PANEL(IFACES9 IFACE_SRCS9 CLASS_NAME pqMine24DtoMap3D
                        XML_NAME "Mine24D to Map3D" XML_GROUP filters)  

LIST(APPEND IFACES ${IFACES2})
LIST(APPEND IFACES ${IFACES3})
LIST(APPEND IFACES ${IFACES4})
LIST(APPEND IFACES ${IFACES5})
LIST(APPEND IFACES ${IFACES6})
LIST(APPEND IFACES ${IFACES7})
LIST(APPEND IFACES ${IFACES8})
LIST(APPEND IFACES ${IFACES9})

LIST(APPEND IFACE_SRCS ${IFACE_SRCS2})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS3})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS4})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS5})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS6})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS7})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS8})
LIST(APPEND IFACE_SRCS ${IFACE_SRCS9})

ADD_PARAVIEW_PLUGIN(PVGFiltersSMPlugin "1.0"
			SERVER_MANAGER_XML FiltersSM.xml
			SERVER_MANAGER_SOURCES  ${PVGFilters_SM_SRCS}
			SERVER_SOURCES  ${PVGFilters_EXTRA_SRCS}
			)

ADD_PARAVIEW_PLUGIN(PVGFiltersGUIPlugin "1.0"
             GUI_INTERFACES ${IFACES}
             GUI_RESOURCES GUI.qrc
             GUI_SOURCES ${PVGFilters_GUI_SRCS} ${MOC_SRCS} ${IFACE_SRCS}
			)

ADD_PARAVIEW_PLUGIN(PVGFiltersCPlugin  "1.0" 
			GUI_RESOURCE_FILES FiltersCategory.xml)			

ENDIF(QT4_FOUND)

#CPACK CODE TO BUNDLE THE DLLS CORRECTLY
IF(PVGPlugins_STAND_ALONE_BUILD)
	INSTALL(TARGETS PVGFiltersGUIPlugin PVGFiltersSMPlugin PVGFiltersCPlugin
		RUNTIME DESTINATION . COMPONENT PVGFilters
		)
ENDIF(PVGPlugins_STAND_ALONE_BUILD)

set(CPACK_COMPONENT_PVGFILTERS_DISPLAY_NAME "ParaViewGeo Filters")
set(CPACK_COMPONENT_PVGFILTERS_DESCRIPTION 
   "Filters that allow manipulating data in ParaView")