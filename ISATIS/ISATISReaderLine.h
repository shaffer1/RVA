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

// .NAME ISATISReaderLine - Line Delegate
// .SECTION Description
// ISATISReaderLine is the delegate which handles all ISATIS
// line data. It reads lines and displays them properly in ParaView.
// .SECTION See Also
// ISATISReaderDelegate
// ISATISReaderDefault
// ISATISReaderGrid
// ISATISReaderPolygon

#ifndef __ISATIS_READER_LINE_H
#define __ISATIS_READER_LINE_H

#include "ISATISReaderDelegate.h"
class ISATISReaderLine;
class ISATISReaderSource;
class GTXClient;
class GTXFileInfo;
class vtkUnstructuredGrid;

class ISATISReaderLine : public ISATISReaderDelegate {
public:
  vtkTypeMacro(ISATISReaderLine,ISATISReaderDelegate);

  static ISATISReaderLine* New();


  // Description:
  // Verifies that the input data
  // is a lines file. Returns 1 for success (can read)
  // and 0 for failure (cannot read).
  virtual int CanRead(
    ISATISReaderSource* source,
    GTXClient* client,
    GTXFileInfo* fileInfo);

  // Description:
  // Creates and sets data object. Always returns 1.
  virtual int RequestDataObject(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

  // Description:
  // Retrieves various information about the line(s) checking
  // that there is sufficient data to display appropriately.
  // Returns 1 for success otherwise 0 for failure.
  virtual int RequestInformation(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

  // Description:
  // Populates the data object with real data from
  // GTXserver. Returns 1 for success otherwise 0 for
  // failure.
  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client);

protected:
  ISATISReaderLine() {
  }
  virtual ~ISATISReaderLine() {
  }

private:
  // void createGridPoints(vtkStructuredGrid* sgrid,GTXClient* client,int dim[3]);
};

#endif

