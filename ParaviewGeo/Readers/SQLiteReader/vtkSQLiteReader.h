/*Nehme Bilal
nehmebilal@gmail.com
objectivity 
June 2011

This reader can import data from an SQLite database.
Currently only point sets are supported but this reader
was designed in a way that allow adding the ability to
import different type of data. The goal is to provide the 
capacity of having all data related to some project in one
database.
*/


#ifndef _vtkSQLiteReader_h
#define _vtkSQLiteReader_h

#include <vtksys/ios/sstream>
#include "vtkPolyDataAlgorithm.h"

struct Internal;


class VTK_EXPORT vtkSQLiteReader : public vtkPolyDataAlgorithm
{
public:
  static vtkSQLiteReader* New();
  vtkTypeRevisionMacro(vtkSQLiteReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

	vtkSetStringMacro(FileName);  

	vtkSetStringMacro(Headers);
	vtkGetStringMacro(Headers);

	vtkSetStringMacro(ActiveTable);
	vtkGetStringMacro(ActiveTable);

	vtkSetStringMacro(ActiveTableType);
	vtkGetStringMacro(ActiveTableType);

	vtkSetStringMacro(Px);
	vtkGetStringMacro(Px);

	vtkSetStringMacro(Py);
	vtkGetStringMacro(Py);

	vtkSetStringMacro(Pz);
	vtkGetStringMacro(Pz);

	int CanReadFile( const char* fname );

	// see xml file.
	// this function is called automatically when the StringVectorProperty
	// "Properties" is modified
	void SetProperties(const char*name, const char* type, int status);

protected:
  vtkSQLiteReader();
  ~vtkSQLiteReader();

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
};

#endif
