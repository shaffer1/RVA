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

#ifndef __ISATIS_FAULT_PROCESSOR_H
#define __ISATIS_FAULT_PROCESSOR_H

#include "vtkInformationVector.h"
#include "vtkPolyData.h"
class vtkTriangle;
class vtkPoints;

class vtkCellArray;
class vtkIntArray;

class GTXFaultSystem;
class GTXFaultSegment;


class ISATISFaultProcessor {

public:

  ISATISFaultProcessor(double _minZ, double _maxZ,double extend) : minZ(0), maxZ(0)
  {
    if( _minZ <= _maxZ) {
     double diff =  extend* (_maxZ-_minZ);
      minZ = _minZ - diff;
      maxZ = _maxZ + diff;
    }
  }

  virtual ~ISATISFaultProcessor(){}

  int process(vtkInformationVector* outputVector, GTXFaultSystem* faultSystem);

private:
  ISATISFaultProcessor(const ISATISFaultProcessor&); // Not implemented.
  void operator=(const ISATISFaultProcessor&); // Not implemented.

  // member variables
  double minZ, maxZ;

  // internal functions

  // Adds a new triangle (as a vtkCellArray) to the triangles array. Vars a,b,c are the triangle indicies
  void addTriangle(vtkCellArray* triangles, int a, int b, int c);

  // Converts a GTX fault system into a list of points.
  // Returns number of valid segments (=one triangle per 3D fault, two triangles for each 2D line segment)
  int populateVertexPool(vtkPoints* points, vtkIntArray* priorities, GTXFaultSystem* faultSystem);

  // returns true if the segment contains exact same points, 4 coordinates
  // for 2D faults and 9 coordiantes for 3D faults
  bool invalidSeg(GTXFaultSegment* seg, bool isThreeD);

};

#endif //__ISATIS_FAULT_PROCESSOR_H