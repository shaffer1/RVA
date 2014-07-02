#ifndef __SHAPEFILEREADER_H_
#define __SHAPEFILEREADER_H_

#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"
#include "vtkCellArray.h"
#include "shapefil.h"

#include <string>
#include <vector>

class ShapefileReader : public vtkPolyDataAlgorithm
{
public:
  static ShapefileReader *New();
  vtkTypeMacro(ShapefileReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetStringMacro(FileName);

protected:
  ShapefileReader();
  ~ShapefileReader();

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector
  );

  virtual int RequestInformation(
    vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector*
  );

  virtual int FillOutputPortInformation(
    int port, 
    vtkInformation* info
  );

  char* FileName;

private:
  ShapefileReader(const ShapefileReader&);  // Not implemented.
  void operator=(const ShapefileReader&);  // Not implemented.
  
  bool illegalFileName();
  bool incompleteSrcFiles();
  void addTriangle(vtkSmartPointer<vtkCellArray> triangles, int a, int b, int c);
};


#endif
