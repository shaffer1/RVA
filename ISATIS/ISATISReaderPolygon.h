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

// .NAME ISATISReaderPolygon - Polygon Reader
// .SECTION Description
// ISATISReaderPolygon is the delegate which handles all ISATIS
// polygon data. It reads polygons and displays them properly in ParaView.
// .SECTION See Also
// ISATISReaderDelegate
// ISATISReaderDefault
// ISATISReaderGrid
// ISATISReaderLine

#ifndef __ISATIS_READER_POLYGON_H
#define __ISATIS_READER_POLYGON_H

#include "ISATISReaderDelegate.h"
#include "GTXPolygon.hpp"

class ISATISReaderPolygon;
class ISATISReaderSource;
class GTXClient;
class GTXFileInfo;
class vtkPoints;

class ISATISReaderPolygon : public ISATISReaderDelegate {
public:
  vtkTypeMacro(ISATISReaderPolygon,ISATISReaderDelegate);

  static ISATISReaderPolygon* New();

  // Description:
  // Verifies the input file is a valid polygon file.
  // Returns 1 for success (can read) otherwise 0 for
  // failure (cannot read).
  int CanRead(
    ISATISReaderSource* source,
    GTXClient* client,
    GTXFileInfo* fileInfo);

  // Description:
  // Creates and sets data object. Always returns 1.
  int RequestDataObject(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

  // Description:
  // Always returns 1.
  int RequestInformation(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

  // Description:
  // Populates the data object with real data from
  // GTXserver. Returns 1 for success otherwise 0 for
  // failure.
  int RequestData(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

protected:
  ISATISReaderPolygon() {
  }
  ~ISATISReaderPolygon() {
  }

private:
  gtx_long verticesToPoints(GTXPolygon& polygon, vtkPoints * points, int bottomOrTop);
};

#endif

