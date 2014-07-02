/*=========================================================================

Program:   RVA
Module:    CutBetweenWellsFilter

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Li

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __CutBetweenWellsFilter_h
#define __CutBetweenWellsFilter_h


#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSmartPointer.h"

#include <vector>
#include <string>

class vtkTimeStamp;
class vtkIdList;
class vtkDataSet;


class VTK_EXPORT CutBetweenWellsFilter : public vtkUnstructuredGridAlgorithm {


public:

  static CutBetweenWellsFilter* New();
  vtkTypeRevisionMacro(CutBetweenWellsFilter, vtkUnstructuredGridAlgorithm);

protected:

  CutBetweenWellsFilter();
  virtual ~CutBetweenWellsFilter();

  virtual int FillInputPortInformation(int, vtkInformation*);
  virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

private:
  //BTX
  int lineSegsCount;
  bool cached;
  bool * cache;// ptr to array of bools (one for each cell)
  double ** lineSegs; //For each line segment there are 6 doubles (x,y,z,x,y,z) that represent the start and end points of the line seg
  double * cutBounds; // a bounding box by the min and max x y coordinate of the two wells

  vtkTimeStamp buildTime; // ensure that our cache is cleared if the input changes

  bool checkCellInclusion(vtkIdType numIds, vtkDataSet* input, vtkSmartPointer<vtkIdList> ptIds, double* bounds);
  void constructLineSegs(vtkInformationVector **inputVectors);
  void calculateCutBounds();

  /*
  void getProjection(double*, double*, double*);
  double getMagnitude(double*, double*);
  double getScalar(double*, double*, double*, double);
  */

  CutBetweenWellsFilter(const CutBetweenWellsFilter&); // Not implemented.
  void operator=(const CutBetweenWellsFilter&); // Not implemented.
  //ETX
};

#endif // __CutBetweenWellsFilter_h
