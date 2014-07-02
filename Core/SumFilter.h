/*=========================================================================

Program:   RVA
Module:    Sum

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Li, D McWherter, R Reizner

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __SumFilter_h
#define __SumFilter_h

#include "vtkDataSetAlgorithm.h"
#include "vtkRectilinearGrid.h"
#include "vtkStdString.h"
#include <vector>
#include <utility>

class vtkPolyData;

class SumFilter : public vtkDataSetAlgorithm
{
public:

  virtual int RequestUpdateExtent (
    vtkInformation *,
    vtkInformationVector **,
    vtkInformationVector *);
  virtual int RequestInformation(vtkInformation*, 
    vtkInformationVector**, 
    vtkInformationVector*);
  virtual int RequestData(vtkInformation*, 
    vtkInformationVector**, 
    vtkInformationVector*);

  static SumFilter *New();
  vtkTypeMacro(SumFilter,vtkDataSetAlgorithm);

protected: 
  SumFilter();
  virtual ~SumFilter();

private:
  SumFilter(const SumFilter&);  // Not implemented.
  void operator=(const SumFilter&);  // Not implemented.

  vtkDataSet * Output;

  virtual void calculateSums(vtkFieldData* data, vtkDataSet * output);
	virtual void GetExtent(vtkDataSet* image, int extent[]);
	virtual void SetExtent(vtkDataSet* image, int extent[]);
};


#endif


