/*=========================================================================

Program:   RVA
Module:    ConnectedThresholdWithCustomSource

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Li, D McWherter, R Reizner

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __ConnectedThresholdWithCustomSourceFilter_h
#define __ConnectedThresholdWithCustomSourceFilter_h

#include "vtkDataSetAlgorithm.h"
#include "vtkRectilinearGrid.h"
#include "vtkStdString.h"
#include <vector>
#include <utility>

class vtkPolyData;
class vtkCellLocator;

class ConnectedThresholdWithCustomSourceFilter : public vtkDataSetAlgorithm
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

  static ConnectedThresholdWithCustomSourceFilter *New();
  vtkTypeMacro(ConnectedThresholdWithCustomSourceFilter,vtkDataSetAlgorithm);

  // Description:
  // Criterion is cells whose scalars are between lower and upper thresholds
  // (inclusive of the end values).
  void ThresholdBetween(double lower, double upper);
  void ThresholdBetween2(double lower, double upper);

  // Description:
  // Get the Upper and Lower thresholds.
  vtkGetMacro(UpperThreshold, double);
  vtkGetMacro(LowerThreshold, double);

  vtkSetMacro(UpperThreshold, double);
  vtkSetMacro(LowerThreshold, double);

  vtkGetMacro(UpperThreshold2, double);
  vtkGetMacro(LowerThreshold2, double);

  vtkSetMacro(UpperThreshold2, double);
  vtkSetMacro(LowerThreshold2, double);

  vtkSetMacro(InsideOut, bool);
  vtkGetMacro(InsideOut, bool);
  vtkSetMacro(InsideOut2, bool);
  vtkGetMacro(InsideOut2, bool);

// First, Second or both  (And/OR) setting
  vtkSetMacro(Mode, int);

  //char* RVAArrayName;
  //vtkSetStringMacro(RVAArrayName);
  //vtkGetStringMacro(RVAArrayName);
  // Paraview sends us the string name of the array.
  // We do not use the other arguments
  void SetRVAArrayName (int , int, int, int, vtkStdString);
  void SetRVAArrayName2 (int , int, int, int, vtkStdString);
  void SetResultArrayName(vtkStdString);

  vtkGetMacro(RVAArrayName, vtkStdString);

protected: 
  ConnectedThresholdWithCustomSourceFilter();
  virtual ~ConnectedThresholdWithCustomSourceFilter();

private:
  ConnectedThresholdWithCustomSourceFilter(const ConnectedThresholdWithCustomSourceFilter&);  // Not implemented.
  void operator=(const ConnectedThresholdWithCustomSourceFilter&);  // Not implemented.

  vtkStdString RVAArrayName;
  vtkStdString RVAArrayName2;

  //vtkDataSet * Output;
  vtkStdString ResultArrayName;

  int Mode; // see executeConnectivity source for enumerated values
  int DataType;
	vtkCellLocator* cellLocator;
	int extent[6];

  virtual void iterateOverStartingPoints(int*rawConnectivityArray,vtkDataArray*inScalars,vtkDataArray*inScalars2, vtkDataSet*input,vtkDataSet*dataset, int autoIncrement);

  virtual void GetCellDimensions(vtkDataSet* image);
  virtual void GetExtent(vtkDataSet* image, int extent[]);
  virtual int ComputeStructuredCoordinates(vtkDataSet* imageOrRectilinear, double point[], int ijkOUT[], double pcoordsOUT[], int extentIN[]);
  virtual void SetExtent(vtkDataSet* imageOrRectilinear, int extent[]);
  virtual int doThreshold(vtkInformation* outInfo, vtkDataSet* imageOrRectilinear, vtkDataSet* output, vtkPolyData* input2, vtkDataArray* inScalars, vtkDataArray* inScalars2);

  // recursive method
  vtkIdType executeConnectivity(vtkDataArray* data, vtkDataArray* data2, int* connectivity, int i, int j, int k, int paint);

  int WholeExtent[6];
  int cellDimensions[3];
  double LowerThreshold;
  double UpperThreshold;
  double LowerThreshold2;
  double UpperThreshold2;
  int    AttributeMode;
  bool isImageData;
  bool InsideOut;
  bool InsideOut2;

  //BTX
  int (ConnectedThresholdWithCustomSourceFilter::*ThresholdFunction)(double s);
  //ETX

  int Between(double s) {return ( s >= this->LowerThreshold ?
    ( s <= this->UpperThreshold ? 1 : 0 ) : 0 );}
  int Between2(double s) {return ( s >= this->LowerThreshold2 ?
    ( s <= this->UpperThreshold2 ? 1 : 0 ) : 0 );}
};


#endif


