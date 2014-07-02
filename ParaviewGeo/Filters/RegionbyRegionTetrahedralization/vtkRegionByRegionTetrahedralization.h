/*
// Author:   Nehme Bilal
// Date:     MARCH 2009
// contact: nehmebilal@gmail.com 

The "Volumique Tetrahedralization" filter cannot tetrahedralize an object 
containing intersecting closed surfaces. To do so, this filter will apply
"Volumique Tetrahedralization" filter on each of the closed surfaces separatley
and append the results.
*/


#ifndef __vtkRegionByRegionTetrahedralization_h
#define __vtkRegionByRegionTetrahedralization_h

#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkDoubleArray.h"



struct Internal;

class VTK_EXPORT vtkRegionByRegionTetrahedralization : public vtkUnstructuredGridAlgorithm
{
public:
	static vtkRegionByRegionTetrahedralization *New();
  vtkTypeRevisionMacro(vtkRegionByRegionTetrahedralization,vtkUnstructuredGridAlgorithm);

	void copyPoint(double *src, double *dest);
	double tetrahedronVolume(double *p1, double *p2, double *p3, double *p4);

	vtkSetStringMacro(RegionArray);
	vtkGetStringMacro(RegionArray);

	char* GetVolumesArray()
	{
		return VolumesArray;
	}

protected:
  vtkRegionByRegionTetrahedralization();
  ~vtkRegionByRegionTetrahedralization();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

	char* RegionArray;
	char *VolumesArray;

private:
  vtkRegionByRegionTetrahedralization(const vtkRegionByRegionTetrahedralization&);  // Not implemented.
  void operator=(const vtkRegionByRegionTetrahedralization&);  // Not implemented.

	Internal *internals;

};

#endif
