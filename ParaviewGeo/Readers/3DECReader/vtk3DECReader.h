/*Nehme Bilal
nehmebilal@gmail.com
objectivity 
June 2011

This reader can import FLAC3D grid data.
Only blocks are supported currently
*/


#ifndef _vtk3DECReader_h
#define _vtk3DECReader_h

#include <vtksys/ios/sstream>
#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkIdTypeArray.h"


class VTK_EXPORT vtk3DECReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtk3DECReader* New();
  vtkTypeRevisionMacro(vtk3DECReader,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

	vtkSetStringMacro(FileName);  

	vtkSetStringMacro(DisplacementVectorsFile);
	vtkGetStringMacro(DisplacementVectorsFile);

	vtkSetStringMacro(ZonesScalarsFile);
	vtkGetStringMacro(ZonesScalarsFile);

	vtkSetStringMacro(ZonesTensorsFile);
	vtkGetStringMacro(ZonesTensorsFile);
	
	vtkSetStringMacro(PointsFile);
	vtkGetStringMacro(PointsFile);	

	int CanReadFile( const char* fname );


protected:
  vtk3DECReader();
  ~vtk3DECReader();

  virtual int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);
                  
  virtual int RequestInformation(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);
								
	void importPoints(vtkUnstructuredGrid* output);
	void importTetrahedras(vtkUnstructuredGrid* output);
  void importDisplacements(vtkUnstructuredGrid* output);
  void importScalars(vtkUnstructuredGrid* output);
  void importTensors(vtkUnstructuredGrid *output);

	const char* FileName; 
	char* PointsFile;
	char* DisplacementVectorsFile;
	char* ZonesScalarsFile;
	char* ZonesTensorsFile;
	
	vtkIdTypeArray* pidArray;
	vtkIdTypeArray* cidArray;
};

#endif
