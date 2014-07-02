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

#include <cassert>

#include "ISATISReaderSource.h"
#include "ISATISReaderLine.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"

vtkStandardNewMacro(ISATISReaderLine)

int ISATISReaderLine::CanRead(
  ISATISReaderSource* source,
  GTXClient* client,
  GTXFileInfo* fileInfo)
{
  if(fileInfo->GetFileType()==GTXFileInfo::FILE_TYPE_GRAVITY_LINES
      || fileInfo->GetFileType()==GTXFileInfo::FILE_TYPE_CORE_LINES)
    return 1; // 1 = Yes
  return 0; // No
  //todo: Maybe this reader can handle Core and Fault lines too but we don't have any example data
}


int ISATISReaderLine::RequestDataObject(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  SetDataObject(outputVector,0,source,vtkUnstructuredGrid::New());
  return 1;
}

int ISATISReaderLine::RequestInformation(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{


//	outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);

  return 1; // 0=Fail, 1 = Success
}


int  ISATISReaderLine::RequestData(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)

{
  vtkUnstructuredGrid*ugrid = vtkUnstructuredGrid::GetData(outputVector,0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);


  GTXFileInfo finfo = client->GetFileInfo();
  const gtx_long numSamples = finfo.GetSampleNumber();
  const int numLines = finfo.GetItemNumber(); // Number of lines to create
  const char* xcoordname = finfo.GetXCoordinateVariableName();
  const char* ycoordname = finfo.GetYCoordinateVariableName();
  const char* zcoordname = finfo.GetZCoordinateVariableName();
  const char * relativename = finfo.GetRelativeNumberVariableName();
  const char * linenumname = finfo.GetLineNumberVariableName();

  assert(xcoordname && ycoordname && zcoordname && relativename && linenumname);

  cchar xyznames[3] = {xcoordname,ycoordname,zcoordname};

  if(numSamples<=0 || numLines<=0) {
    vtkErrorMacro(<<"Line had zero (or negative) samples or no lines defined - ignoring");
    return 0;
  }
  if(!xcoordname || !ycoordname || !zcoordname || strlen(xcoordname)==0 || strlen(ycoordname) ==0 || strlen(zcoordname)==0 ) {
    vtkErrorMacro(<<"No X,Y,Z coord arrays - can't create line");
    return 0;
  }
  if(!relativename || !linenumname) {
    vtkErrorMacro(<<"Relative Line Index Information is missing - can't create lines");
    return 0;
  }

  ugrid->Allocate(numLines);

  vtkIdType dim[3] = { numSamples,1,1};

  source->SetProgressText("Reading Variables");
  readAllVariables(ugrid, source,client,dim);

  source->SetProgressText("Creating Lines");

  if( ! createPoints(ugrid,client,(vtkIdType) numSamples,xyznames) )
    return 0;

  return createLines(ugrid,numLines,numSamples,relativename,linenumname);  // 1 = Success
}


