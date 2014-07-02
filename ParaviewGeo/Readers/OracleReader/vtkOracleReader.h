#ifndef _vtkOracleReader_h
#define _vtkOracleReader_h


#include "vtkPolyDataAlgorithm.h"



class VTK_EXPORT vtkOracleReader : public vtkPolyDataAlgorithm
{
public:
  static vtkOracleReader* New();
  vtkTypeRevisionMacro(vtkOracleReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

	vtkSetStringMacro(FileName);  

	int CanReadFile( const char* fname );


protected:
  vtkOracleReader();
  ~vtkOracleReader();

  virtual int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);
                  
  virtual int RequestInformation(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);

private:

	const char* FileName; 
};

#endif