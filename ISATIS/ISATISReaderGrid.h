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

// .NAME ISATISReaderGrid - Grid Delegate
// .SECTION Description
// ISATISReaderGrid is the delegate which handles all ISATIS
// grid data. It reads grids and displays them properly in ParaView.
// .SECTION See Also
// ISATISReaderDelegate
// ISATISReaderDefault
// ISATISReaderLine
// ISATISReaderPolygon

#ifndef __ISATIS_READER_GRID_H
#define __ISATIS_READER_GRID_H

#include "ISATISReaderDelegate.h"

class ISATISReaderGrid;
class ISATISReaderSource;
class GTXClient;
class GTXFileInfo;
class vtkStructuredGrid;


class ISATISReaderGrid : public ISATISReaderDelegate {
public:
  vtkTypeMacro(ISATISReaderGrid,ISATISReaderDelegate);

  static ISATISReaderGrid* New();


  // Description:
  // Checks if file type is a grid file. Returns 1 for
  // success (can read) otherwise 0 for failure (cannot read).
  virtual int CanRead(
    ISATISReaderSource* source,
    GTXClient* client,
    GTXFileInfo* fileInfo);

  // Description:
  // Creates and sets appropriate data object. Always returns 1.
  virtual int RequestDataObject(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

  // Description:
  // Retrieves various grid information such as grid dimensions
  // and checks that the grid has at least 1 point.
  // Returns 1 for success or 0 for failure.
  virtual int RequestInformation(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

  // Description:
  // Populates the data object with real data read from
  // GTXserver. Returns 1 for success otherwise 0 for failure.
  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

protected:
  ISATISReaderGrid(){}
  virtual ~ISATISReaderGrid(){}

private:
  // void createGridPoints(vtkStructuredGrid* sgrid,GTXClient* client,int dim[3]);
};

#endif