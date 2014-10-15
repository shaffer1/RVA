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

#if defined _MSC_VER
#pragma warning( once: 4996 )
#define _CRT_SECURE_NO_WARNINGS (1)

#endif

// Include our header
#include "CutBetweenWellsFilter.h"

// Standard libraries
#include <cassert>
#include <time.h>

// Useful vtk/paraview headers
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"

#include "vtkIdList.h"
#include "vtkSmartPointer.h"
#include "vtkTimeStamp.h"

#include "vtkUnstructuredGrid.h"

#include "vtkCell.h"
#include "vtkPoints.h"
#include "vtkLine.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"

#include "vtkDoubleArray.h"
#include "vtkCellArray.h"


// --------------------------------------------------------
vtkStandardNewMacro(CutBetweenWellsFilter);

// constructor
CutBetweenWellsFilter::CutBetweenWellsFilter()
{
  cache = NULL;
  cached = false;
  cutBounds = new double[4];
  lineSegsCount = 0;

  this->SetNumberOfInputPorts(3);
  this->SetNumberOfOutputPorts(1);
}

CutBetweenWellsFilter::~CutBetweenWellsFilter()
{
  for (int i = 0; i < lineSegsCount; i++){
    delete [] lineSegs[i];
  }
  delete [] lineSegs; lineSegs = NULL;

  delete [] cache; cache = NULL;
  
  delete [] cutBounds; cutBounds = NULL;
}


int CutBetweenWellsFilter::FillInputPortInformation(int port, vtkInformation * info)
{
  if (port == 0){
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataSet");
  }else{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  }

  return 1;
}


int CutBetweenWellsFilter::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector){

  vtkDataSet* input = vtkDataSet::GetData(inputVector[0]);
  vtkDataSet* inputTwo = vtkDataSet::GetData(inputVector[1]);
  vtkDataSet* inputThree = vtkDataSet::GetData(inputVector[2]);

  if (vtkTimeStamp(buildTime) < inputTwo->GetMTime() || vtkTimeStamp(buildTime) < inputThree->GetMTime()){
    this->cached = false;
  }

  if (!cached){
    buildTime.Modified();
    constructLineSegs(inputVector);
  }
  
  // Retrieve the number of cells and points in the original dataset
  vtkIdType numCells = input->GetNumberOfCells();
  vtkIdType numPts = input->GetNumberOfPoints();

  // Allocate working space for new and old cell point lists.
  vtkSmartPointer<vtkIdList> ptIds = vtkSmartPointer<vtkIdList>::New();
  vtkSmartPointer<vtkIdList> newPtIds = vtkSmartPointer<vtkIdList>::New();
  ptIds->Allocate(VTK_CELL_SIZE);
  newPtIds->Allocate(VTK_CELL_SIZE);
  
  // Allocate space for a new set of points.
  vtkSmartPointer<vtkPoints> newPts = vtkSmartPointer<vtkPoints>::New();
  newPts->Allocate(numPts, numPts);

  // Allocate space for data associated with the new set of points and cells.
  vtkUnstructuredGrid* output = vtkUnstructuredGrid::GetData(outputVector);
  vtkPointData* inPD = input->GetPointData(), *outPD = output->GetPointData();
  vtkCellData *inCD=input->GetCellData(), *outCD=output->GetCellData();
  output->Allocate(numCells);
  outPD->CopyGlobalIdsOn();
  outPD->CopyAllocate(inPD);
  outCD->CopyGlobalIdsOn();
  outCD->CopyAllocate(inCD);

  double bounds[6];
  double p[3];

  if (!cached) {
    delete[] cache;
    cache = new bool[numCells];
  }

  for(vtkIdType cellId = 0; cellId < numCells; cellId++){
    
    // Get the list of points for this cell.
    vtkCell *cell = input->GetCell(cellId);
    cell->GetBounds(bounds);
    
    // cutBounds defines a gross boundary so we can quickly ignore distant cells
    if (!((bounds[0] <= cutBounds[0] && bounds[1] >= cutBounds[0]) ||
          (bounds[0] >= cutBounds[0] && bounds[1] <= cutBounds[1]) ||
          (bounds[0] <= cutBounds[1] && bounds[1] >= cutBounds[1])
          )){
      continue;
    }

    input->GetCellPoints(cellId, ptIds);
    vtkIdType numIds = ptIds->GetNumberOfIds();

    bool includeCell = cached ?  cache[cellId] : checkCellInclusion(numIds, input, ptIds, bounds);

    if (includeCell){
      newPtIds->Reset();
      
      for (vtkIdType i = 0; i < numIds; i++){
        input->GetPoint(ptIds->GetId(i), p);
        vtkIdType newId = newPts->InsertNextPoint(p);
        vtkIdType oldId = ptIds->GetId(i);
        outPD->CopyData(inPD, oldId, newId);
        newPtIds->InsertNextId(newId);
      }

      // Store the new cell in the output.
      vtkIdType newCellId = output->InsertNextCell(cell->GetCellType(),newPtIds);
      outCD->CopyData(inCD, cellId, newCellId);
    } // if
    cache[cellId] = includeCell;
  }
  cached = true;

  output->SetPoints(newPts);
  output->Squeeze();
  
  return 1;
}


bool CutBetweenWellsFilter::checkCellInclusion(vtkIdType numIds, vtkDataSet* input, vtkSmartPointer<vtkIdList> ptIds, double* bounds) {
  
  double t = 0;
  double p[3];
  double proj[3];

  for (vtkIdType i = 0; i < numIds; i++){

    input->GetPoint(ptIds->GetId(i), p);

    for (int j = 0; j < lineSegsCount; j++){
      //getProjection(p, lineSegs[j], proj);
      vtkLine::DistanceToLine(p, lineSegs[j], lineSegs[j]+3, t, proj);

      // 1 - closest point inside cell
      // 2 - closest point on x-axis boundary but not corner
      // 3 - closest point on y-axis boundary but not corner
      if ( (proj[0] > bounds[0] && proj[0] < bounds[1] && proj[1] > bounds[2] && proj[1] < bounds[3]) || 
        ((proj[0] == bounds[0] || proj[0] == bounds[1]) && (proj[1] > bounds[2] && proj[1] < bounds[3])) ||
        ((proj[1] == bounds[2] || proj[1] == bounds[3]) && (proj[0] > bounds[0] && proj[0] < bounds[1]))){
          return true;
      }
    }
  }

  return false;
}

// constructs multiple lines connecting well blocks in two wells
void CutBetweenWellsFilter::constructLineSegs(vtkInformationVector **inputVector){
  
  vtkInformation* inInfoOne = inputVector[1]->GetInformationObject(0);
  vtkPolyData* inOne = vtkPolyData::SafeDownCast(inInfoOne->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation* inInfoTwo = inputVector[2]->GetInformationObject(0);
  vtkPolyData* inTwo = vtkPolyData::SafeDownCast(inInfoTwo->Get(vtkDataObject::DATA_OBJECT()));
  
  int one = inOne->GetLines()->GetNumberOfCells();
  int two = inOne->GetLines()->GetNumberOfCells();

  int ptCountOne = inOne->GetNumberOfPoints();
  int ptCountTwo = inTwo->GetNumberOfPoints();

  if (!(ptCountOne > 0 && ptCountTwo > 0)){
    vtkErrorMacro(<<"Not enough points provided");
  }

  int ptCountLonger = ptCountOne;
  int ptCountShorter = ptCountTwo;

  vtkPolyData* inLonger = inOne;
  vtkPolyData* inShorter = inTwo;

  if (ptCountTwo > ptCountOne){
    ptCountLonger = ptCountTwo;
    ptCountShorter = ptCountOne;
    inLonger = inTwo;
    inShorter = inOne;
  }

  int i = 0;
  int t = 0;
  double longer[3];
  double shorter[3];
  lineSegs = new double* [ptCountLonger];
  lineSegsCount = ptCountLonger;
  
  for (i; i < ptCountShorter; i++){
    lineSegs[i] = new double[6];

    inLonger->GetPoint(i, lineSegs[i]);
    inShorter->GetPoint(i, lineSegs[i]+3);
  }

  t = i-1;

  for (; i < ptCountLonger; i++){
    lineSegs[i] = new double[6];

    inLonger->GetPoint(i, lineSegs[i]);

    lineSegs[i][3] = lineSegs[t][3];
    lineSegs[i][4] = lineSegs[t][4];
    lineSegs[i][5] = lineSegs[t][5];
  }

  calculateCutBounds();
}

//calculates max/min x-y coordinates of all well blocks
void CutBetweenWellsFilter::calculateCutBounds(){
  
  for (int i = 0; i < lineSegsCount; i++){

    if (lineSegs[i][0] <= cutBounds[0] || i == 0)
      cutBounds[0] = lineSegs[i][0];

    if (lineSegs[i][0] >= cutBounds[1]|| i == 0)
      cutBounds[1] = lineSegs[i][0];

    if (lineSegs[i][1] <= cutBounds[2]|| i == 0)
      cutBounds[2] = lineSegs[i][1];

    if (lineSegs[i][1] >= cutBounds[3]|| i == 0)
      cutBounds[3] = lineSegs[i][1];

    if (lineSegs[i][3] <= cutBounds[0])
      cutBounds[0] = lineSegs[i][3];

    if (lineSegs[i][3] >= cutBounds[1])
      cutBounds[1] = lineSegs[i][3];

    if (lineSegs[i][4] <= cutBounds[2])
      cutBounds[2] = lineSegs[i][4];

    if (lineSegs[i][4] >= cutBounds[3])
      cutBounds[3] = lineSegs[i][4];
  }
}


/*
void CutBetweenWellsFilter::getProjection(double* p, double * lineSeg, double* proj)
{
  // p: [x,y] or [x,y,z]
  // lineSegs: [lineStart[x,y,z], lineEnd[x,y,z]]

  if (!(p && lineSeg && proj)){
    vtkErrorMacro(<<"Invalid pointer or vector size in getProjection(double* p, std::vector<std::vector<double>> * lineSeg, double* proj)");
  }
  
  double scalar = getScalar(p, lineSeg, lineSeg+3, getMagnitude(lineSeg, lineSeg+3));
  
  if (scalar <= 0){
    proj[0] = lineSeg[0];
    proj[1] = lineSeg[1];
    proj[2] = lineSeg[2];
  }else if (scalar >= 1){
    proj[0] = lineSeg[3];
    proj[1] = lineSeg[4];
    proj[2] = lineSeg[5];
  }else{
    proj[0] = (lineSeg[0] + scalar*(lineSeg[3] - lineSeg[0]));
    proj[1] = (lineSeg[2] + scalar*(lineSeg[4] - lineSeg[2]));
    proj[2] = (p[2]);
  }
}

double CutBetweenWellsFilter::getMagnitude(double * pA, double * pB)
{
  if (!(pA && pB)){
    vtkErrorMacro(<<"Invalid pointer or vector size in getMagnitude(std::vector<double> * pA, std::vector<double> * pB)");
    return 0;
  }
  
  double dX = pA[0]-pB[0];
  double dY = pA[1]-pB[1];

  return (double)sqrt(dX*dX + dY*dY);
}

double CutBetweenWellsFilter::getScalar(double* p, double * lS, double * lE, double m)
{
  if (!(p && lS && lE)){
    vtkErrorMacro(<<"Invalid pointer or vector size in getScalar(double* p, std::vector<double> * lS, std::vector<double> * lE, double m)");
    return 0;
  }

  return ((p[0]-lS[0])*(lE[0]-lS[0]) + (p[1]-lS[1])*(lE[1] -lS[1]))/(m*m);
}

*/
