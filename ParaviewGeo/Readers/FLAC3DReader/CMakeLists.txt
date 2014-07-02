PROJECT(FLAC3DReader)
FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
	
QT4_WRAP_CPP(MOC_SRCS pqFLAC3DReader.h) 

ADD_PARAVIEW_OBJECT_PANEL(IFACES IFACE_SRCS CLASS_NAME pqFLAC3DReader
                         XML_NAME FLAC3DReader XML_GROUP sources)
	
	# create a plugin for this panel
ADD_PARAVIEW_PLUGIN(GUIFLAC3DReader "1.0" 
                    GUI_INTERFACES ${IFACES} 
                    GUI_RESOURCES pqUI.qrc
                    SOURCES pqFLAC3DReader.cpp ${MOC_SRCS} ${IFACE_SRCS}
					
					SERVER_SOURCES ../../common/StringUtilities.cpp
				)

INCLUDE_DIRECTORIES(../../common)				
                    
ADD_PARAVIEW_PLUGIN(FLAC3DReaderClientPlugin "1.0"
	GUI_RESOURCES pqFLAC3DReader.qrc)

ADD_PARAVIEW_PLUGIN(FLAC3DReaderServerManagerPlugin "1.0"
	SERVER_MANAGER_XML FLAC3DReader.xml
	SERVER_MANAGER_SOURCES vtkFLAC3DReader.cpp)

target_link_libraries(FLAC3DReaderServerManagerPlugin)