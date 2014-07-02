PROJECT(SQLiteReader)
FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
	
QT4_WRAP_CPP(MOC_SRCS pqSQLiteReader.h) 

ADD_PARAVIEW_OBJECT_PANEL(IFACES IFACE_SRCS CLASS_NAME pqSQLiteReader
                         XML_NAME SQLiteReader XML_GROUP sources)
	
	# create a plugin for this panel
ADD_PARAVIEW_PLUGIN(GUISQLiteReader "1.0" 
                    GUI_INTERFACES ${IFACES} 
                    GUI_RESOURCES pqUI.qrc
                    SOURCES pqSQLiteReader.cpp ${MOC_SRCS} ${IFACE_SRCS})
                    
ADD_PARAVIEW_PLUGIN(SQLiteReaderClientPlugin "1.0"
	GUI_RESOURCES pqSQLiteReader.qrc)

ADD_PARAVIEW_PLUGIN(SQLiteReaderServerManagerPlugin "1.0"
	SERVER_MANAGER_XML SQLiteReader.xml
	SERVER_MANAGER_SOURCES vtkSQLiteReader.cpp)

target_link_libraries(SQLiteReaderServerManagerPlugin)