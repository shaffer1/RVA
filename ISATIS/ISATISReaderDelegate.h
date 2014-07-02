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

// .NAME ISATISReaderDelegate - Delegate base
// .SECTION Description
// ISATISReaderDelegate is the base class to all delegates
// providing very basic common functionality among all
// delegates.
// .SECTION See Also
// ISATISReaderDefault
// ISATISReaderGrid
// ISATISReaderLine
// ISATISReaderPolygon
// .SECTION Todo
// Replace "typedef ... cchar" with something that's already defined

#ifndef __ISATISReaderDelegate_h
#define __ISATISReaderDelegate_h

#include "vtkObject.h"

class vtkAlgorithm;
class vtkInformation;
class vtkInformationVector;
class vtkDataObject;
class vtkDataSet;
class vtkPointSet;
class vtkUnstructuredGrid;
class vtkAbstractArray;
class ISATISReaderSource;

class GTXClient;
class GTXFileInfo;

typedef const char* cchar; //todo replace this with something that's already defined

class ISATISReaderDelegate : public vtkObject {
public:
  vtkTypeMacro(ISATISReaderDelegate,vtkObject);

  // Description:
  // Returns true (1) if this delegate can read the file, 0 otherwise
  virtual int CanRead(
    ISATISReaderSource* source,
    GTXClient* gtxclient,
    GTXFileInfo* fileInfo)=0; // 0 = No, 1 = Yes
  virtual int RequestDataObject(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client)=0;
  virtual int RequestInformation(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client)=0;
  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector* outputVector,
    ISATISReaderSource* source,
    GTXClient* client)=0;

  double maxZ, minZ; // You can test for a valid range by checking minZ <= maxZ


protected:

  // Description:
  // Prepares the data object to be displayed in ParaView
  void SetDataObject(vtkInformationVector* outputVector, int port, vtkAlgorithm* source, vtkDataObject* output);

  // Description:
  // Read all variables of a data set. Function relies on readOneVariable.
  void readAllVariables(vtkDataSet* output,vtkAlgorithm*source, GTXClient*client, vtkIdType dim[3]);

  // Description:
  // Read a single specified variable of a data set and store its values in
  // an appropriate array type.
  void readOneVariable(vtkDataSet* output,GTXClient*client, vtkIdType dim[3], cchar name);

  // Description:
  // Creates points to create a VTK object based on appropriate
  // ISATIS input data from GTXserver. Returns 1 for success otherwise 0 for
  // failure.
  int createPoints(vtkPointSet* data,GTXClient* client,const vtkIdType expectedSize, cchar names[3]);

  // Description:
  // Creates lines to create a VTK object based on appropriate
  // ISATIS input data from GTXserver. Returns 1 for success otherwise 0 for
  // failure.
  int createLines(vtkUnstructuredGrid* ugrid,vtkIdType numLines, vtkIdType numSamples, const char*relativename, cchar linenumname);

  // Description:
  // Finds the unique identifiers for X, Y, Z coordinates.
  // Method is used for retrieving appropriate X, Y, Z coordinates
  // from GTXserver. Returns 1 for success otherwise 0 for failure.
  int findXYZVarNames(GTXClient*, vtkStdString* x,vtkStdString* y,vtkStdString* z);

  ISATISReaderDelegate();
  virtual ~ISATISReaderDelegate();

private:
  ISATISReaderDelegate(const ISATISReaderDelegate&);  // Not implemented.
  void operator=(const ISATISReaderDelegate&);  // Not implemented.

  // Description:
  // Copy a macro variable array from ISATIS format to
  // a valid VTK array for use in ParaView. Returns
  // 1 on success otherwise 0 for failure.
  int copyMacroArray(vtkDataSet* output,GTXClient* client,vtkIdType nx,vtkIdType ny,vtkIdType nz,vtkIdType expectedSize,cchar vtkArrayName);

  // Description:
  // Copy a variable array from ISATIS format to
  // a valid VTK array for use in ParaView. Returns
  // 1 on success otherwise 0 for failure.
  int copyArray(int varType,vtkDataSet* output,GTXClient* client,vtkIdType nx,vtkIdType ny,vtkIdType nz,vtkIdType expectedSize,cchar vtkArrayName);

  // Description:
  // Creates a character array for use in ParaView.
  // Returns a pointer to the newly created array.
  // Returns a null pointer (0) on failure.
  vtkAbstractArray* createCharArray(GTXClient*client, vtkIdType nx,vtkIdType ny,vtkIdType nz, vtkIdType expectedSize,const char*name);

  // Description:
  // Creates a numeric array for use in ParaView.
  // Returns a pointer to the newly created array.
  // Returns a null pointer (0) on failure.
  vtkAbstractArray* createNumericArray(GTXClient*client, vtkIdType nx,vtkIdType ny,vtkIdType nz, vtkIdType expectedSize,const char*name);

  

}; // ISATISReaderDelegate






#endif