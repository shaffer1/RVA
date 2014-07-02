/*Nehme Bilal
nehmebilal@gmail.com
objectivity 
June 2011

This reader can import data from an ODBC database.
Currently only point sets are supported but this reader
was designed in a way that allow adding the ability to
import different type of data. The goal is to provide the 
capacity of having all data related to some project in one
database.
*/


#ifndef _vtkODBCReader_h
#define _vtkODBCReader_h

#include <vtksys/ios/sstream>
#include "vtkPolyDataAlgorithm.h"
#include <vtkStdString.h>

struct Internal;


class VTK_EXPORT vtkODBCReader : public vtkPolyDataAlgorithm
{
public:
  static vtkODBCReader* New();
  vtkTypeRevisionMacro(vtkODBCReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

	vtkSetStringMacro(FileName);  

	//vtkSetStringMacro(Headers);
	//vtkGetStringMacro(Headers);

	//vtkSetStringMacro(ActiveTable);
	//vtkGetStringMacro(ActiveTable);

	//vtkSetStringMacro(ActiveTableType);
	//vtkGetStringMacro(ActiveTableType);

	//vtkSetStringMacro(Px);
	//vtkGetStringMacro(Px);

	//vtkSetStringMacro(Py);
	//vtkGetStringMacro(Py);

	//vtkSetStringMacro(Pz);
	//vtkGetStringMacro(Pz);

	int CanReadFile( const char* fname );

	// see xml file.
	// this function is called automatically when the StringVectorProperty
	// "Properties" is modified
	void SetProperties(const char*name, const char* type, int status);

protected:
  vtkODBCReader();
  ~vtkODBCReader();

  virtual int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);
                  
  virtual int RequestInformation(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);

private:

	const char* FileName; 
	char* Headers;

	char* ActiveTable;
	char* ActiveTableType;

	char* Px;
	char* Py;
	char* Pz;

	bool guiLoaded;

	Internal *internals;

	vtkStdString dataSourceName;
	vtkStdString username;
	vtkStdString password;
};

#endif
