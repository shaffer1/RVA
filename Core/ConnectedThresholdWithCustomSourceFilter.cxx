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
#include "ConnectedThresholdWithCustomSourceFilter.h"

#include <cassert>

#include "vtkCellData.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkCellLocator.h"
vtkStandardNewMacro(ConnectedThresholdWithCustomSourceFilter);

// TODO Replace with vtk enum
#define IMAGE (1)
#define SGRID (2)
#define RGRID (3)

//----------------------------------------------------------------------------
ConnectedThresholdWithCustomSourceFilter::ConnectedThresholdWithCustomSourceFilter() :
RVAArrayName(""), ResultArrayName("Connectivity"),cellLocator(NULL)//, Output(NULL)
{
  this->SetDebug(1);    
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1); 
  this->isImageData = true;
}

//----------------------------------------------------------------------------
ConnectedThresholdWithCustomSourceFilter::~ConnectedThresholdWithCustomSourceFilter()
{
	if(cellLocator)
		cellLocator->Delete();
	cellLocator = NULL;
}

int ConnectedThresholdWithCustomSourceFilter::RequestUpdateExtent (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *vtkNotUsed( outputVector ))
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  // always request the whole extent
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),6);

  return 1;
}

int ConnectedThresholdWithCustomSourceFilter::RequestInformation(vtkInformation*  request ,
                                                 vtkInformationVector** inputVector,
                                                 vtkInformationVector* outputVector)
{
  return 1;
};

// Criterion is cells whose scalars are between lower and upper thresholds.
void ConnectedThresholdWithCustomSourceFilter::ThresholdBetween(double lower, double upper)
{
  if ( this->LowerThreshold != lower || this->UpperThreshold != upper ||
    this->ThresholdFunction != &ConnectedThresholdWithCustomSourceFilter::Between)
  {
    this->LowerThreshold = lower; 
    this->UpperThreshold = upper;
    this->ThresholdFunction = &ConnectedThresholdWithCustomSourceFilter::Between;
    this->Modified();
  }
}

void ConnectedThresholdWithCustomSourceFilter::ThresholdBetween2(double lower, double upper)
{
  if ( this->LowerThreshold2 != lower || this->UpperThreshold2 != upper ||
    this->ThresholdFunction != &ConnectedThresholdWithCustomSourceFilter::Between2)
  {
    this->LowerThreshold2 = lower; 
    this->UpperThreshold2 = upper;
    this->ThresholdFunction = &ConnectedThresholdWithCustomSourceFilter::Between2;
    this->Modified();
  }
}

int ConnectedThresholdWithCustomSourceFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                          vtkInformationVector **inputVector,
                                          vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
	
	vtkInformation* inInfo2 = NULL;
	vtkPolyData* input2 = NULL;

	if (this->GetNumberOfInputPorts()==2)
	{
		inInfo2 = inputVector[1]->GetInformationObject(0);
		input2 = vtkPolyData::SafeDownCast(
			inInfo2->Get(vtkDataObject::DATA_OBJECT()));
	}

  // Get the input and ouptut
  vtkDataSet * genericInput = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDataSet * output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  assert(genericInput);
	assert(output);

  //this->Output = output;

  output->ShallowCopy(genericInput);


  vtkDataArray* inScalars  = genericInput->GetCellData()->GetArray(this->RVAArrayName);
  vtkDataArray* inScalars2 = genericInput->GetCellData()->GetArray(this->RVAArrayName2);

  assert(inScalars && inScalars2);
  if(!inScalars || !inScalars2)
    return 1;

  if(!doThreshold(outInfo, genericInput, output, input2, inScalars, inScalars2))
    return 0;

  return 1;
}

int ConnectedThresholdWithCustomSourceFilter::doThreshold(vtkInformation* outInfo, vtkDataSet* input, vtkDataSet* output, vtkPolyData* input2, vtkDataArray* inScalars, vtkDataArray* inScalars2)
{
  assert(input && output && inScalars && inScalars2 &&! this->ResultArrayName.empty());

  GetCellDimensions(input);
  // we need cell dimensions not point dimensions...
  this->cellDimensions[0]--;
  this->cellDimensions[1]--;
  this->cellDimensions[2]--;

  if(!inScalars) return 0;

  vtkIdType numTupes1  = inScalars->GetNumberOfTuples();
  vtkIdType numTupes2  = inScalars2->GetNumberOfTuples();

  vtkIdType requiredNumTupes = cellDimensions[0]*cellDimensions[1] * cellDimensions[2];

  if( numTupes1 != requiredNumTupes || numTupes2 != requiredNumTupes ) {
    vtkErrorMacro(<<"Invalid array size");
    return 0;
  }

  vtkIntArray* arr  = vtkIntArray::New();

  arr->SetNumberOfValues(requiredNumTupes);
  
  for(vtkIdType i =0;i <requiredNumTupes;i++) {
    arr->SetValue(i,0);
  }

  arr->SetName(this->ResultArrayName);
  output->GetCellData()->AddArray(arr);
  int* rawConnectivityArray = arr->GetPointer(0);
  assert(rawConnectivityArray);
  if(!rawConnectivityArray) 
    return 0;

 
  GetExtent(input, extent);

	if(input2!=NULL)
		iterateOverStartingPoints(rawConnectivityArray,inScalars,inScalars2,input, input2,false /*always paint with 1 */);
	else
		iterateOverStartingPoints(rawConnectivityArray,inScalars,inScalars2,input, input, true);

  // MVM: with VTK 5.6 pipeline, not sure this is necessary
  /*
  // Without these lines, the output will appear real but will not work as the input to any other filters
  outInfo->Set( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6 );
  SetExtent(output, extent);
  output->SetUpdateExtent(extent);
  output->SetWholeExtent(extent);
  */
  arr->Delete();

  return 1;
}
void ConnectedThresholdWithCustomSourceFilter::iterateOverStartingPoints(int*rawConnectivityArray,vtkDataArray*inScalars,vtkDataArray*inScalars2,  vtkDataSet*input,vtkDataSet*dataset, int autoIncrement) {
	  //iterate over all of source points 
  int ijk[3];
  double pcoords[3];
  double point[3];
		
  int numPoints2 = dataset->GetNumberOfPoints();
	int paint=1;

	if( cellLocator )
		cellLocator->BuildLocatorIfNeeded();

	for (int i=0; i<numPoints2; i++){
    dataset->GetPoint(i, point);
    if (ComputeStructuredCoordinates(input, point, ijk, pcoords,extent))
    {
      vtkIdType result = executeConnectivity(inScalars, inScalars2, rawConnectivityArray, ijk[0],ijk[1],ijk[2],paint);      
      if(autoIncrement && result != 0) paint ++;
		}
  }
}

void ConnectedThresholdWithCustomSourceFilter::SetRVAArrayName(int a, int b, int c, int d, vtkStdString array)
{
  RVAArrayName=array;
}

void ConnectedThresholdWithCustomSourceFilter::SetRVAArrayName2(int a, int b, int c, int d, vtkStdString array)
{
  RVAArrayName2=array;
}

vtkIdType ConnectedThresholdWithCustomSourceFilter::executeConnectivity(vtkDataArray* data, vtkDataArray* data2, int* connectivity, int i, int j, int k, int numb) {
	if(i<0 || j<0 ||k<0 || i>= cellDimensions[0] || j>= cellDimensions[1] || k>=cellDimensions[2])
    return 0;
  vtkIdType cellId = i+j*this->cellDimensions[0] + k*cellDimensions[0]*cellDimensions[1];
  double val  = data->GetComponent(cellId,0);
  double val2 = data2->GetComponent(cellId,0);

	int result=0;
  bool dataCheck  = ((Between(val) && !InsideOut) || (!Between(val) && InsideOut)) && (connectivity[cellId]==0);
  bool data2Check = ((Between2(val2) && !InsideOut2) || (!Between2(val2) && InsideOut2)) && (connectivity[cellId]==0);
  bool connected  = (Mode == 0) ? dataCheck && data2Check : (Mode == 1) ? dataCheck || data2Check : (Mode == 2) ? dataCheck : data2Check;
  if(connected) {
    connectivity[cellId]=numb;
		result = 1;
    result += executeConnectivity(data, data2, connectivity, i+1, j, k, numb);
    result += executeConnectivity(data, data2, connectivity, i-1, j, k, numb);
    result += executeConnectivity(data, data2, connectivity, i, j+1, k, numb);
    result += executeConnectivity(data, data2, connectivity, i, j-1, k, numb);
    result += executeConnectivity(data, data2, connectivity, i, j, k+1, numb);
    result += executeConnectivity(data, data2, connectivity, i, j, k-1, numb);
  }
  return result;
}

void ConnectedThresholdWithCustomSourceFilter::GetCellDimensions(vtkDataSet* image)
{
  vtkImageData* imd = vtkImageData::SafeDownCast(image);
  vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast(image);
  vtkStructuredGrid* sgrid = vtkStructuredGrid::SafeDownCast(image);

	if(imd != NULL)
  {
    imd->GetDimensions(this->cellDimensions);
    this->DataType = IMAGE;
  }
  else if(rgrid != NULL)
  {
    rgrid->GetDimensions(this->cellDimensions);
    this->DataType = RGRID;
  }
	else if(sgrid != NULL)
  {
    sgrid->GetDimensions(this->cellDimensions);
    this->DataType = SGRID;
  }
}

void ConnectedThresholdWithCustomSourceFilter::GetExtent(vtkDataSet* image, int extent[])
{
  vtkImageData* imd = vtkImageData::SafeDownCast(image);
  vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast(image);
  vtkStructuredGrid* sgrid = vtkStructuredGrid::SafeDownCast(image);
 if(imd != NULL)
    imd->GetExtent(extent);
  else if(rgrid != NULL)
    rgrid->GetExtent(extent);
 else if(sgrid != NULL)
    sgrid->GetExtent(extent);
}

void ConnectedThresholdWithCustomSourceFilter::SetResultArrayName(vtkStdString name)
{
 /* if(this->Output != NULL) {
    vtkAbstractArray* arr = this->Output->GetCellData()->GetAbstractArray(name.c_str());
    if(arr != NULL)
      arr->SetName(name.c_str());
  }*/

  vtkStdString oldName(this->ResultArrayName);
  if(name.empty())
    this->ResultArrayName = "Connectivity";
  else
    this->ResultArrayName = name;
  if(oldName.compare(this->ResultArrayName) != 0) {
    this->Modified();
  }
}

int ConnectedThresholdWithCustomSourceFilter::ComputeStructuredCoordinates(vtkDataSet* image, double point[], int ijk[], double pcoords[], int extent[])
{
  vtkImageData* imd = vtkImageData::SafeDownCast(image);
  vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast(image);
	vtkStructuredGrid* sgrid = vtkStructuredGrid::SafeDownCast(image);
  if(imd != NULL)
	{
		int result = imd->ComputeStructuredCoordinates(point, ijk, pcoords);
		 ijk[0]-=extent[0];
      ijk[1]-=extent[2];
      ijk[2]-=extent[4];
			return result;
	}
	else if(rgrid != NULL) {
    int result = rgrid->ComputeStructuredCoordinates(point, ijk, pcoords);
		 ijk[0]-=extent[0];
      ijk[1]-=extent[2];
      ijk[2]-=extent[4];
			return result;
	}
	
	else if(sgrid != NULL) {
		if(cellLocator == NULL) {
		  this->cellLocator = vtkCellLocator::New();
			cellLocator->SetDataSet(sgrid);
			cellLocator->BuildLocator();
		}
			vtkIdType cellId = cellLocator->FindCell(point);
			if(cellId<0) return 0; // outside

			int nx = extent[1]-extent[0];
			int ny = extent[3]-extent[2];
			int nz = extent[5]-extent[4];

			ijk[0] = cellId % nx;
			ijk[1] = (cellId/nx) % ny;
			ijk[2] = cellId/nx / ny;

			return 1;
	}
  return 0;
}

void ConnectedThresholdWithCustomSourceFilter::SetExtent(vtkDataSet* image, int extent[])
{
  vtkImageData* imd = vtkImageData::SafeDownCast(image);
  vtkRectilinearGrid* rgrid = vtkRectilinearGrid::SafeDownCast(image);
	vtkStructuredGrid* sgrid = vtkStructuredGrid::SafeDownCast(image);
  if(imd)
    imd->SetExtent(extent);
  else if(rgrid != NULL)
    rgrid->SetExtent(extent);
	else if(sgrid != NULL)
		sgrid->SetExtent(extent);
}
