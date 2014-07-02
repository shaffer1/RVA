PROJECT(3DECReader)
FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
	
QT4_WRAP_CPP(MOC_SRCS pq3DECReader.h) 

ADD_PARAVIEW_OBJECT_PANEL(IFACES IFACE_SRCS CLASS_NAME pq3DECReader
                         XML_NAME 3DECReader XML_GROUP sources)
	
	# create a plugin for this panel
ADD_PARAVIEW_PLUGIN(GUI3DECReader "1.0" 
                    GUI_INTERFACES ${IFACES} 
                    GUI_RESOURCES pqUI.qrc
                    SOURCES pq3DECReader.cpp ${MOC_SRCS} ${IFACE_SRCS}
					
					SERVER_SOURCES ../../common/StringUtilities.cpp
				)

INCLUDE_DIRECTORIES(../../common)				
                    
ADD_PARAVIEW_PLUGIN(3DECReaderClientPlugin "1.0"
	GUI_RESOURCES pq3DECReader.qrc)

ADD_PARAVIEW_PLUGIN(3DECReaderServerManagerPlugin "1.0"
	SERVER_MANAGER_XML 3DECReader.xml
	SERVER_MANAGER_SOURCES vtk3DECReader.cpp)

target_link_libraries(3DECReaderServerManagerPlugin)