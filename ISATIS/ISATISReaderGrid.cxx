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
#include "ISATISReaderGrid.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkDataArray.h"
#include "vtkImageData.h"


vtkStandardNewMacro(ISATISReaderGrid)

int ISATISReaderGrid::CanRead(
  ISATISReaderSource* source,
  GTXClient* client,
  GTXFileInfo* fileInfo)
{
  if(fileInfo->GetFileType()==GTXFileInfo::FILE_TYPE_GRID
    && (fileInfo->GetDimension() ==3 || fileInfo->GetDimension() == 2)) {
      return 1; // 1 = Yes
  }
  return 0; // No
}


int ISATISReaderGrid::RequestDataObject(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  SetDataObject(outputVector,0,source,vtkStructuredGrid::New());
  return 1;
}

int ISATISReaderGrid::RequestInformation(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  vtkStructuredGrid*sgrid = vtkStructuredGrid::GetData(outputVector,0);
  vtkInformation* gridInfo = outputVector->GetInformationObject(0);
  assert(sgrid && gridInfo);

  GTXFileInfo fi = client->GetFileInfo();

  // MVM: These are +1 because Isatis is counting cells, but for 
  // vtkStructuredGrid we need the point-based dimension.
  int dim[3] = {fi.GetGridNX()+1,fi.GetGridNY()+1,fi.GetGridNZ()+1};
  
  // MVM: These are then index-range min, max of the points dimensions.
  int ext[6] = {0,dim[0]-1,0,dim[1]-1,0,dim[2]-1};
  long num_points = dim[0]*dim[1]*dim[2];
  if(num_points<=0)
    return 0; // 0 == Failure

  sgrid->SetDimensions(dim);
  gridInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);

  return 1; // 0=Fail, 1 = Success
}



int ISATISReaderGrid::RequestData(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
 
  vtkStructuredGrid*sgrid = vtkStructuredGrid::GetData(outputVector,0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  GTXFileInfo fi = client->GetFileInfo();

 
  vtkIdType dimVtk[3] = {fi.GetGridNX()+1,fi.GetGridNY()+1,fi.GetGridNZ()+1}; // This needed for 64-bit compilation
  // MVM: double check the claim that using two dimension arrays is needed for
  // 64-bit compilation.
  int dim[3] = {fi.GetGridNX()+1,fi.GetGridNY()+1,fi.GetGridNZ()+1};
  double deltas[3] = {fi.GetGridDX(), fi.GetGridDY(), fi.GetGridDZ()};

  sgrid->SetDimensions(dim);

  source->SetProgressText("Reading Variables");
  readAllVariables(sgrid, source,client,dimVtk);
  source->SetProgressText("Creating Points");

  vtkIdType expectedNumCells = (dim[0]-1) * (dim[1]-1) * (dim[2]-1);
  vtkIdType expectedNumPts = dim[0] * dim[1] * dim[2];

  vtkStdString x,y,z;
  int found = findXYZVarNames(client, &x,&y,&z);
  if((found&3) != 3) { // Make sure we have at least X and Y since x means found |= 1 and y means found |= 2
    return 0;
  }
  const char* xyznames[3] = {x,y,z};

  return createPoints(sgrid,client,expectedNumCells, expectedNumPts,xyznames,deltas); // 1 = success
}
