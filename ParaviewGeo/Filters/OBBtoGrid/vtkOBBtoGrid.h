/*create a structured grid inside an oriented bounding box*/


#ifndef _vtkOBBtoGrid_h
#define _vtkOBBtoGrid_h
#include "vtkStructuredGridAlgorithm.h"

/*
//				Mirarco Mining Innovation
//
// Filter:   Mine24DtoMap3D
// Class:    vtkMine24DtoMap3D
// Author:   Nehme Bilal
// Date:     July 2009
// contact: nehmebilal@gmail.com 

This filter allow creating a grid from an oriented box.
If you just have a set of points, use the FitDataSet filter to create an oriented bounding box
from the set of points.
*/


class VTK_EXPORT vtkOBBtoGrid : public vtkStructuredGridAlgorithm
{
public:
  static vtkOBBtoGrid *New();
  vtkTypeRevisionMacro(vtkOBBtoGrid,vtkStructuredGridAlgorithm);

	vtkSetVector3Macro(GridSize, int);
	vtkGetVector3Macro(GridSize, int);

protected:
  vtkOBBtoGrid();
  ~vtkOBBtoGrid();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
	virtual int FillInputPortInformation(int port, vtkInformation *info);


private:

	int GridSize[3];

  vtkOBBtoGrid(const vtkOBBtoGrid&);
  void operator = (const vtkOBBtoGrid&);
};
#endif
