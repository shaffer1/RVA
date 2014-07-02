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

// .NAME ISATISReaderDefault - Default Delegate
// .SECTION Description
// ISATISReaderDefault is the default delegate for ISATISReaderSource.
// It is the fall-back delegate when no appropriate delegate can be
// selected. When the pipeline completes, this delegate will always
// display a unit cube.
// .SECTION See Also
// ISATISReaderDelegate
// ISATISReaderGrid
// ISATISReaderLine
// ISATISReaderPolygon

#ifndef __ISATISReaderDefault_h
#define __ISATISReaderDefault_h

#include "ISATISReaderDelegate.h"


class ISATISReaderDefault : public ISATISReaderDelegate {
public:
  vtkTypeMacro(ISATISReaderDefault,ISATISReaderDelegate);
  static ISATISReaderDefault *New();

  // Description:
  // Default method always returning 1.
  virtual int CanRead(ISATISReaderSource* source,GTXClient* gtxclient, GTXFileInfo* fileInfo);
  virtual int RequestDataObject(vtkInformation* request,vtkInformationVector* outputVector,ISATISReaderSource* source,GTXClient* gtxClient);
  virtual int RequestInformation(vtkInformation* request,vtkInformationVector* outputVector,ISATISReaderSource* source,GTXClient* gtxClient);
  virtual int RequestData(vtkInformation* request,vtkInformationVector* outputVector,ISATISReaderSource* source,GTXClient* gtxClient);
  virtual int FillOutputPortInformation(int port, vtkInformation *info);

protected:
  ISATISReaderDefault() {}
  virtual ~ISATISReaderDefault() {}
};


#endif