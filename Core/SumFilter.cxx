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
#include "SumFilter.h"

#include <cassert>

#include "vtkDoubleArray.h"
#include "vtkCellData.h"
#include "vtkImageData.h"
#include "vtkStructuredGrid.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(SumFilter);


//----------------------------------------------------------------------------
SumFilter::SumFilter() :
Output(NULL)
{
  this->SetDebug(1);    
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1); 
  //this->isImageData = true;
}

//----------------------------------------------------------------------------
SumFilter::~SumFilter()
{
}

int SumFilter::RequestUpdateExtent (
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

int SumFilter::RequestInformation(vtkInformation*  request ,
                                                 vtkInformationVector** inputVector,
                                                 vtkInformationVector* outputVector)
{
  return 1;
};


int SumFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                          vtkInformationVector **inputVector,
                                          vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  //vtkInformation* inInfo2 = inputVector[1]->GetInformationObject(0);

  // Get the input and ouptut
  vtkDataSet * input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkDataSet * output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

	
	assert(output);
	if(output==NULL)
		return 0;

	output->ShallowCopy(input);
  this->Output = output;

	vtkFieldData* cellData = input->GetAttributesAsFieldData(vtkDataObject::CELL);
	vtkFieldData* pointData = input->GetAttributesAsFieldData(vtkDataObject::POINT);
	calculateSums(cellData, output);
	calculateSums(pointData, output);

	int extent[6];
  GetExtent(input, extent);
  // Without these lines, the output will appear real but will not work as the input to any other filters
  outInfo->Set( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6 );
  SetExtent(output, extent);
  output->SetUpdateExtent(extent);
  output->SetWholeExtent(extent);

  return 1;
}

void SumFilter::calculateSums(vtkFieldData * data, vtkDataSet * output)
{
	if (!data || !output)
		return;
	int numArrays = data->GetNumberOfArrays();
	for (int i=0; i<numArrays; i++)
	{
		vtkDataArray* arr = data->GetArray(i);
		const char* arrName = data->GetArrayName(i);
		int numTuples = arr->GetNumberOfTuples();
		int numComponents = arr->GetNumberOfComponents();
		vtkDoubleArray* sumArr  = vtkDoubleArray::New();
		sumArr->SetNumberOfComponents(numComponents);
		sumArr->SetNumberOfTuples(1);
		for (int j=0; j<numComponents; j++)
		{
			double sum = 0;
			for (int k=0; k<numTuples; k++)
			{
				sum += arr->GetComponent(k, j);
			}
			sumArr->SetComponent(0, j, sum);
		}
		vtkStdString name = arrName;
		name+= " Sum";
		sumArr->SetName(name);
		output->GetFieldData()->AddArray(sumArr);
		sumArr->Delete();
	}
}


void SumFilter::SetExtent(vtkDataSet* image, int extent[])
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
void SumFilter::GetExtent(vtkDataSet* image, int extent[])
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