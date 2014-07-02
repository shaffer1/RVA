/*=========================================================================

Program:   RVA
Module:    ISATISReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Duggirala, D McWherter, U Yadav

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "ISATISReaderDefault.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkImageData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"

#include "ISATISReaderSource.h"

vtkStandardNewMacro(ISATISReaderDefault)

int ISATISReaderDefault::CanRead(
  ISATISReaderSource* source,
  GTXClient* client,
  GTXFileInfo* fileInfo)
{
  return 1; // 1= Can Read Anything
}


int ISATISReaderDefault::FillOutputPortInformation(int port, vtkInformation* info){
  if (port == 0)
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataSet");

  return 1;
}


int ISATISReaderDefault::RequestDataObject(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  return 0;
  SetDataObject(outputVector,0,source,vtkImageData::New());
  return 1; // 1 = Success
}


int ISATISReaderDefault::RequestInformation(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  return 0;
  vtkInformation* info = outputVector->GetInformationObject(0);
  int extent[] = {0,1,0,1,0,1};
  info->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);
  return 1;  // 1 = Success
}

int  ISATISReaderDefault::RequestData( vtkInformation* request,
                                       vtkInformationVector* outputVector,
                                       ISATISReaderSource* source,
                                       GTXClient* client)
{
  return 0;
  vtkImageData* image = vtkImageData::GetData(outputVector);
  // image could be null if the outputport data object cannot be safely cast to vtkImageData
  // for example wthe default delegate could be called to duty just for RequestData
  // if the real delegate failed
  if(image) image->SetExtent(0,1,0,1,0,1);

  return 1;
}

