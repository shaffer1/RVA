/*=========================================================================

Program:   Visualization Toolkit
Module:    $ vtkConvertPointsToBlocks$
Author:    Arolde VIDJINNAGNI
MIRARCO,   Laurentian University
Date:      May 7 2009
Version:   0.1
=========================================================================*/
/*
Used to convert a point set to a set of blocks.
Often block model are represented by a set of points where each point is 
the center of a block. It is somtimes useful to go back to the original blocks
representation for better visualization, which is the purpose of this filter.

You can manually specify the blocks size or choose a property from the input.
Using a property from the input will provide you with the capacity of using
variable blocks size. 
*/

#ifndef _vtkConvertPointsToBlocks_h
#define _vtkConvertPointsToBlocks_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPolyData.h"

class VTK_EXPORT vtkConvertPointsToBlocks : public vtkUnstructuredGridAlgorithm
{
public:
	static vtkConvertPointsToBlocks *New();
	vtkTypeRevisionMacro(vtkConvertPointsToBlocks,vtkUnstructuredGridAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent);

	//dimension 
	vtkSetStringMacro(XINC);
	vtkGetStringMacro(XINC);

	vtkSetStringMacro(YINC);
	vtkGetStringMacro(YINC);

	vtkSetStringMacro(ZINC);
	vtkGetStringMacro(ZINC);

	vtkSetMacro(SizeCX, double);
	vtkGetMacro(SizeCX, double);

	vtkSetMacro(SizeCY, double);
	vtkGetMacro(SizeCY, double);

	vtkSetMacro(SizeCZ, double);
	vtkGetMacro(SizeCZ, double);

	vtkSetStringMacro(XEntry);
	vtkSetStringMacro(YEntry);
	vtkSetStringMacro(ZEntry);


protected:
	vtkConvertPointsToBlocks();
	~vtkConvertPointsToBlocks();
	int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
	int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
	int FillInputPortInformation(int port, vtkInformation *info);
	int FillOutputPortInformation(int port, vtkInformation *info);

	vtkUnstructuredGrid* ConvertsToDefaultGeometry(vtkDataSet* polys);


private:
	vtkConvertPointsToBlocks(const vtkConvertPointsToBlocks&);
	void operator = (const vtkConvertPointsToBlocks&);

	//
	char* XINC;
	char* YINC;
	char* ZINC;
	
	double SizeCX;
	double SizeCY;
	double SizeCZ; 

	char* XEntry;
	char* YEntry;
	char* ZEntry;
};
#endif
