PROJECT(FitDataset)
FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(FitDatasetSMPlugin "1.0"
	SERVER_MANAGER_XML FitDataset.xml
	SERVER_MANAGER_SOURCES vtkFitDataset.cpp
	SERVER_SOURCES 	vtkConvexHull.cpp
					vtkLeastSquareFit.cpp
					Inc3dCHull_ConvexHull.cpp
					Inc3dCHull_Face.cpp
					Inc3dCHull_Edge.cpp
					Inc3dCHull_Vertex.cpp
				)

ADD_PARAVIEW_PLUGIN(FitDatasetCPlugin "1.0"
	GUI_RESOURCE_FILES FitDatasetCategory.xml)


