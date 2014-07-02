/*=========================================================================

Program:   RVA
Module:    ISATISReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, D McWherter, J Li

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <cassert>

#include "gtxfaultsystem.hpp"
#include "gtxfault.hpp"
#include "gtxfaultinfo.hpp"

#include "ISATISFaultProcessor.h"

#include "vtkSmartPointer.h"
#include "vtkIntArray.h"
#include "vtkTriangle.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"

// Main entry point to create faults
// Faults are created on output port 1
// Called by ISATISReaderSource.RequestData

int ISATISFaultProcessor::process(
                                  vtkInformationVector* outputVector, 
                                  GTXFaultSystem* faultSystem)
{
  assert(outputVector && faultSystem);

  vtkPolyData* polyData = vtkPolyData::GetData(outputVector, 1);
  assert(polyData);
  if(!polyData) return 0; // Paranoia

  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkIntArray> priorities = vtkSmartPointer<vtkIntArray>::New();
  priorities->SetName("priority");

  int numTriangles = populateVertexPool(points,priorities, faultSystem);

  for (int i = 0; i < numTriangles; i++){
    addTriangle(triangles, 3*i, 3*i+1, 3*i+2);
  }
  
  polyData->SetPolys(triangles);
  polyData->SetPoints(points);
  polyData->GetCellData()->AddArray(priorities);

  return 1;
}



/*
 Internal helper method - Converts a GTX fault system into a list of points.
  Returns number of valid segments (=one triangle per 3D fault, two triangles for each 2D line segment)

 */
int ISATISFaultProcessor::populateVertexPool(
                                            vtkPoints* points, vtkIntArray* priorities,
                                            GTXFaultSystem* faultSystem)
{
  
  assert(faultSystem);
  assert(points && priorities);
  
  int totalSegs = 0;
  int numFaults = faultSystem->GetFaultsNumber();
  bool is2DFault = faultSystem->GetFaults2DFlag();

  for (int i = 0; i < numFaults; i++){

    GTXFault fault = faultSystem->GetFault(i);
    const char* unused = fault.GetName();

    int numSegs = fault.GetSegmentsNumber();

    for (int j = 0; j < numSegs; j++){

      GTXFaultSegment seg = fault.GetSegment(j);
      int priority = seg.GetPriority();
      // Some test data included a fault segment with a zero area triangle.
      if (invalidSeg(&seg, !faultSystem->GetFaults2DFlag()))
          continue;
 
      if (is2DFault){
        points->InsertNextPoint(seg.GetX1(), seg.GetY1(), minZ); 
        points->InsertNextPoint(seg.GetX1(), seg.GetY1(), maxZ);
        points->InsertNextPoint(seg.GetX2(), seg.GetY2(), minZ); 
        
        points->InsertNextPoint(seg.GetX1(), seg.GetY1(), maxZ);
        points->InsertNextPoint(seg.GetX2(), seg.GetY2(), minZ); 
        points->InsertNextPoint(seg.GetX2(), seg.GetY2(), maxZ); 
      }else{
        points->InsertNextPoint(seg.GetX1(), seg.GetY1(), seg.GetZ1());
        points->InsertNextPoint(seg.GetX2(), seg.GetY2(), seg.GetZ2());
        points->InsertNextPoint(seg.GetX3(), seg.GetY3(), seg.GetZ3());
      }
     
      int trianglesAdded = is2DFault ? 2 : 1;
      totalSegs += trianglesAdded;

      for(int k=0;k<trianglesAdded;k++) {
        priorities->InsertNextValue(priority);
      }
    }
  }

  return totalSegs;
}



/*
  Internal helper function that creates a triangle, sets vertices
  offsets and inserts it in a cell array
 */
void ISATISFaultProcessor::addTriangle(
                                       vtkCellArray* triangles, 
                                       int a, int b, int c)
{
  vtkTriangle* t = vtkTriangle::New();

  t->GetPointIds()->SetId(0, a);
  t->GetPointIds()->SetId(1, b);
  t->GetPointIds()->SetId(2, c);

  triangles->InsertNextCell(t);
  
  t->Delete();
}



/*
  returns true if the segment contains exact same points, 4 coordinates
  for 2D faults and 9 coordiantes for 3D faults
 */
bool ISATISFaultProcessor::invalidSeg(GTXFaultSegment* seg, bool isThreeD)
{
  assert(seg);
  if(!seg) return true;

  return seg->GetX1() == seg->GetX2() &&
         seg->GetY1() == seg->GetY2() &&
         (isThreeD ? seg->GetX2() == seg->GetX3() &&
                     seg->GetY2() == seg->GetY3() &&
                     seg->GetZ1() == seg->GetZ2() &&
                     seg->GetZ2() == seg->GetZ3() 
                   : true);
}



/*
int ISATISFaultProcessor::processAsPolyLines(
                                              vtkPolyData* polyData, 
                                              GTXFaultSystem* faultSystem)
{
  vtkSmartPointer<vtkCellArray> polyLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  int numPolyLines = 0;
  int numFaults = faultSystem->GetFaultsNumber();

  for (int i = 0; i < 1; i++){

    GTXFault fault = faultSystem->GetFault(i);
    int numSegs = fault.GetSegmentsNumber();

    for (int j = 0; j < numSegs; j++){

      GTXFaultSegment seg = fault.GetSegment(j);
      vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();

      //points->InsertNextPoint(seg.GetX1(), seg.GetY1())
      //points->InsertNextPoint(seg.GetX2(), seg.GetY2())
      
      polyLine->GetPointIds()->SetId(0, 2*numPolyLines);
      polyLine->GetPointIds()->SetId(1, 2*numPolyLines+1);

      numPolyLines++;
      polyLines->InsertNextCell(polyLine);
    }
  }

  polyData->SetPolys(polyLines);
  polyData->SetPoints(points);

  return 1;
}



int ISATISFaultProcessor::processAsTriangles(
                                              vtkPolyData* polyData, 
                                              GTXFaultSystem* faultSystem)
{
  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  int numTriangles = 0;
  int numFaults = faultSystem->GetFaultsNumber();

  for (int i = 0; i < 1; i++){

    GTXFault fault = faultSystem->GetFault(i);
    int numSegs = fault.GetSegmentsNumber();

    for (int j = 0; j < numSegs; j++){

      GTXFaultSegment seg = fault.GetSegment(j);

      points->InsertNextPoint(seg.GetX1(), seg.GetY1(), seg.GetZ1());
      points->InsertNextPoint(seg.GetX2(), seg.GetY2(), seg.GetZ2());
      points->InsertNextPoint(seg.GetX3(), seg.GetY3(), seg.GetZ3());
    
      addTriangle(triangles, 3*numTriangles, 3*numTriangles+1, 3*numTriangles+2);
      numTriangles++;
    }
  }

  polyData->SetPolys(triangles);
  polyData->SetPoints(points);

  return 1;
}
*/