/*Nehme Bilal
nehmebilal@gmail.com
objectivity 
June 2011

This reader can import FLAC3D grid data.
Only blocks are supported currently
*/


#ifndef _vtkFLAC3DReader_h
#define _vtkFLAC3DReader_h

#include <vtksys/ios/sstream>
#include "vtkUnstructuredGridAlgorithm.h"


class VTK_EXPORT vtkFLAC3DReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkFLAC3DReader* New();
  vtkTypeRevisionMacro(vtkFLAC3DReader,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

	vtkSetStringMacro(FileName);  

	vtkSetStringMacro(DisplacementVectorsFile);
	vtkGetStringMacro(DisplacementVectorsFile);

	vtkSetStringMacro(ZonesScalarsFile);
	vtkGetStringMacro(ZonesScalarsFile);

	vtkSetStringMacro(ZonesTensorsFile);
	vtkGetStringMacro(ZonesTensorsFile);

	int CanReadFile( const char* fname );


protected:
  vtkFLAC3DReader();
  ~vtkFLAC3DReader();

  virtual int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);
                  
  virtual int RequestInformation(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);

  void importDisplacements(vtkUnstructuredGrid* output);
  void importScalars(vtkUnstructuredGrid* output);
  void importTensors(vtkUnstructuredGrid *output);

	const char* FileName; 
	char* DisplacementVectorsFile;
	char* ZonesScalarsFile;
	char* ZonesTensorsFile;
};

#endif
