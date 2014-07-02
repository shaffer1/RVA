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
#include "ISATISReaderPolygon.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolygon.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(ISATISReaderPolygon)

int ISATISReaderPolygon::CanRead(
  ISATISReaderSource* source,
  GTXClient* client,
  GTXFileInfo* fileInfo)
{
  if(fileInfo->GetFileType()==GTXFileInfo::FILE_TYPE_POINTS && fileInfo->GetPolygonFlag())
    return 1; // 1 = Yes
  return 0; // No
}



int ISATISReaderPolygon::RequestDataObject(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  SetDataObject(outputVector,0,source,vtkUnstructuredGrid::New());
  return 1;
}

int ISATISReaderPolygon::RequestInformation(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  return 1; // 0=Fail, 1 = Success
}


int ISATISReaderPolygon::RequestData(
  vtkInformation* request,
  vtkInformationVector* outputVector,
  ISATISReaderSource* source,
  GTXClient* client)
{
  vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::GetData(outputVector,0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  source->SetProgressText("Reading Polygons");

  GTXPolygonSystem psys = client->ReadPolygons(); // Get polygon information

  ugrid->Allocate(); // Don't crash
  vtkPoints * points = vtkPoints::New();
  vtkIdType pointId = 0;
  // Assume 3D only for now
  const gtx_long polycount = psys.GetPolygonsNumber();
  

  for(gtx_long i=0; i<polycount; ++i) {
    GTXPolygon polygon = psys.GetPolygon(i);
    for(int bottomOrTop = 0; bottomOrTop < 2 ; bottomOrTop++) {
      // Convert vertices to point data
      const gtx_long numPts = verticesToPoints(polygon, points,bottomOrTop);
      if(numPts>0) {
         double pZMin = polygon.GetZMin();
         double pZMax = polygon.GetZMax();
         bool notSet = minZ > maxZ;
          minZ = (pZMin < minZ || notSet) ? pZMin : minZ;
          maxZ = (pZMax > maxZ || notSet) ? pZMax : maxZ;
      }
      // Set up the polygon
      vtkPolygon * poly = vtkPolygon::New();
      poly->GetPointIds()->SetNumberOfIds(numPts);
      for(gtx_long j=0; j<numPts; ++j,++pointId)
        poly->GetPointIds()->SetId(j,pointId);
      ugrid->InsertNextCell(poly->GetCellType(), poly->GetPointIds());
      poly->Delete();
    }
  }
  ugrid->SetPoints(points);
  points->Delete();

  return 1;
}

gtx_long ISATISReaderPolygon::verticesToPoints(GTXPolygon& polygon, vtkPoints * points, int bottomOrTop)
{
  const gtx_long vertices = polygon.GetVerticesNumber();
  gtx_long ret = 0;

  const double z = bottomOrTop ? polygon.GetZMin() : polygon.GetZMax();
  for(gtx_long i=0; i<vertices; ++i) {
    const double x = polygon.GetXVertices(i);
    const double y = polygon.GetYVertices(i);
    points->InsertNextPoint(x,y,z);
    ret++;
  }

  return ret;
}



