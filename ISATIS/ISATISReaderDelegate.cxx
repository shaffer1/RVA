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

#include "ISATISReaderDelegate.h"

#include <assert.h>

#include "GTXClient.hpp"
#include "GTXStringArray.hpp"
#include "GTXCharData.hpp"
#include "GTXDoubleData.hpp"

#include "vtkAlgorithm.h"
#include "vtkBitArray.h"
#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkPolyLine.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"

ISATISReaderDelegate::ISATISReaderDelegate()
{
  minZ = 2; // Used for 2D Faults
  maxZ = 1; // minZ is greater than maxZ so this is an invalid range!
}

ISATISReaderDelegate::~ISATISReaderDelegate()
{
}


void ISATISReaderDelegate::SetDataObject(vtkInformationVector* outputVector, int port,vtkAlgorithm*source, vtkDataObject* output)
{
  vtkInformation* info = outputVector->GetInformationObject(port);
  int extentType = output->GetExtentType();
  output->SetPipelineInformation(info);
  output->Delete();
  vtkInformation* algInfo = source->GetOutputPortInformation(port);
  algInfo->Set(vtkDataObject::DATA_EXTENT_TYPE(), extentType);
  // From vtkDataObject.h / VTK docs...
  //"The ExtentType will be left as VTK_PIECES_EXTENT for data objects such as vtkPolyData and vtkUnstructuredGrid.
  // The ExtentType will be changed to VTK_3D_EXTENT for data objects with 3D structure such as vtkImageData 
  // (and its subclass vtkStructuredPoints), vtkRectilinearGrid, and vtkStructuredGrid. The default is the have an 
  // extent in pieces, with only one piece (no streaming possible).
  // Reimplemented in vtkImageData, vtkRectilinearGrid, vtkStructuredGrid, vtkTemporalDataSet, and vtkImageStencilData."


  // From vtkUnstructuredGrid constructor....
  if(extentType == VTK_PIECES_EXTENT) {
    algInfo->Set(vtkDataObject::DATA_PIECE_NUMBER(), -1);
    algInfo->Set(vtkDataObject::DATA_NUMBER_OF_PIECES(), 1);
    algInfo->Set(vtkDataObject::DATA_NUMBER_OF_GHOST_LEVELS(), 0);
  }

}

void ISATISReaderDelegate::readAllVariables(vtkDataSet* output,vtkAlgorithm*source, GTXClient*client, vtkIdType dim[3])
{
  GTXStringArray vars = client->GetVariableList();
  const double oneOverCount = vars.GetCount() > 0 ? 1. / vars.GetCount() : 1;
  for (int ivar = 0; ivar < vars.GetCount(); ivar++) {        //Iterating thru all the variables of the file
    const char* varName = vars.GetValue(ivar);             //get the name of the first variable
    if(!varName)
      continue; // Paranoia

    source->SetProgressText(varName);
    source->SetProgress(ivar * oneOverCount);

    readOneVariable(output,client,dim,varName);
  } // for each variable name
}

void ISATISReaderDelegate::readOneVariable(vtkDataSet* output,GTXClient*client, vtkIdType dim[3], const char* name)
{

  client->SetVariable(name);
  GTXVariableInfo varInfo = client->GetVariableInfo();
  GTXVariableInfo::VariableType varType = varInfo.GetVariableType();

  // At this point, dim[] contains VTK style point-based dims
  // subtract 1 to get Isatis cell-based dims
  const vtkIdType nx = dim[0]-1, ny = dim[1]-1, nz = dim[2]-1;
  const gtx_long  expectedSize = nx * ny * nz;

  if(varType == GTXVariableInfo::VAR_TYPE_MACRO)
    copyMacroArray(output,client, nx, ny, nz, expectedSize,name); //// multiple scalars to copy
  else
    copyArray(varType, output, client, nx,ny,nz, expectedSize,name);

}

int ISATISReaderDelegate::copyMacroArray(vtkDataSet* output, GTXClient* client, vtkIdType nx, vtkIdType ny, vtkIdType nz,
        vtkIdType expectedSize, const char* vtkArrayName)
{
  // name[xxxxx]. Want to drop the [xxxxxx]
  vtkStdString baseName(vtkArrayName);
  size_t pos = baseName.find_last_of("[");
  if(pos!=std::string::npos)
    baseName.resize(pos);
  // see GTXClient example browser.cpp
  const GTXStringArray columnNames =  client->GetMacroAlphaIndices();
  const int columnNameCount = columnNames.GetCount();
  int result=1; // success

  for(int i =0; i < columnNameCount; i++) {
    const char* colNameShort = columnNames.GetValue(i);
    if(!colNameShort)
      continue; // Paranoid
    vtkStdString colName(baseName);
    colName += "[";
    colName +=colNameShort;
    colName += "]";

    client->SetAlphaIndice(colNameShort);
    if(! copyArray(GTXVariableInfo::VAR_TYPE_MACRO, output,client, nx,ny,nz, expectedSize,colName))
      result = 0; // don't use short circuiting otherwise later arrays will not be copied when result =0
  }
  if( columnNameCount )
    return result; // see GTXClient example browser.cpp
  // Use int indices only if alphanumeric columns names are missing

  GTXIntArray columnIndices = client->GetMacroIndices();
  const int columnIndexCount = columnIndices.GetCount();

  for (int i = 0; i<columnIndexCount; i++) {
    int index = columnIndices.GetValue(i);
    char temp[32];
    // MVM: this generates a warning. C++11 has snprintf. What about a stringstream?
    sprintf(temp,"[%d]",index);

    vtkStdString colName(baseName);
    colName += temp;

    client->SetIndice(index);
    if(! copyArray(GTXVariableInfo::VAR_TYPE_MACRO, output, client, nx,ny,nz, expectedSize,colName))
      result = 0; // don't use short circuiting otherwise later arrays will not be copied when result =0
  }
  return result;
}


int ISATISReaderDelegate::copyArray(int varType, vtkDataSet* output, GTXClient* client,
        vtkIdType nx, vtkIdType ny, vtkIdType nz, vtkIdType expectedSize, const char* vtkArrayName)
{
  assert(vtkArrayName);
  vtkAbstractArray *result = 0;

  switch(varType) {
  case GTXVariableInfo::VAR_TYPE_CHAR:
    result = createCharArray(client, nx,ny,nz, expectedSize,vtkArrayName);
    break;
  case GTXVariableInfo::VAR_TYPE_MACRO:
  case GTXVariableInfo::VAR_TYPE_FLOAT:
  case GTXVariableInfo::VAR_TYPE_XG:
  case GTXVariableInfo::VAR_TYPE_YG:
  case GTXVariableInfo::VAR_TYPE_ZG:
    result = createNumericArray(client, nx,ny,nz, expectedSize,vtkArrayName);
    break;

  case GTXVariableInfo::VAR_TYPE_INVALID:
    vtkErrorMacro(<<"Variable "<<vtkArrayName<<" is invalid - ignoring");

  default:
    vtkErrorMacro(<<"Unknown variable type "<<varType<<" - ignoring");
  }
  if(result) {
    result->SetName(vtkArrayName);

    if(expectedSize == result->GetNumberOfTuples())
      output->GetCellData()->AddArray(result);
    else
      output->GetFieldData()->AddArray(result);

    result->Delete();
  }
  return result != 0; // 1 == Success
}


vtkAbstractArray * ISATISReaderDelegate::createCharArray(GTXClient* client, 
        vtkIdType nx, vtkIdType ny, vtkIdType nz, vtkIdType expectedSize, const char* name)
{
  const GTXCharData stringArray = client->ReadCharVariable(false); // false = no compress
  const gtx_long  count = stringArray.GetCount();
  const char** rawArray = stringArray.GetValues();

  vtkStringArray* vtkArray = vtkStringArray::New();
  vtkArray->SetNumberOfValues(count);

  if(count != expectedSize) {
    vtkArray->Delete(); // result->Delete won't be happening for this array
    vtkErrorMacro(<<"Ignoring "<< (name?name:"<unknown>") 
            <<": Expected "<<expectedSize<<" values but only found "<<count<<" values.");
    return 0; // failed
  }
  gtx_long gtxIndex = -1;
  for(vtkIdType x=0; x<nx; x++)
    for(vtkIdType y=0; y<ny; y++)
      for(vtkIdType z=0; z<nz; z++) {
        const char* value = rawArray[++gtxIndex];
        const char* valueSafe =  value ? value : "null";
        vtkArray->SetValue(x+nx*(y+ny*z),valueSafe);
      }
  return(vtkAbstractArray*) vtkArray;
}
// note the day we support integer values we will break createLines
// because it assumes vtkDoubleArray
// Fix Just change the code in createLines to support the correct type.


template<class Tprimitive,class Tvtk>
static
vtkAbstractArray *createTypedArray(Tvtk* vtkArray, GTXClient*client, 
        vtkIdType nx, vtkIdType ny, vtkIdType nz, vtkIdType expectedSize, const char* name)
{
  const GTXDoubleData doubleData = client->ReadDoubleVariable(false); // false = no compress
  const gtx_long  count = doubleData.GetCount();
  const double* rawArray = doubleData.GetValues();

  const double undefinedValue = doubleData.GetUndefinedValue();
  const double nan =  vtkMath::Nan();

//	vtkDoubleArray* vtkArray = vtkDoubleArray::New();
  vtkArray->SetNumberOfValues(count);

  if(count != expectedSize) {
    vtkArray->Delete(); // result->Delete won't be happening for this array
//		vtkErrorMacro(<<"Ignoring "<<name <<": Expected "<<expectedSize<<" values but only found "<<count<<" values.");
    return 0;
  }
  gtx_long gtxIndex = -1;
  vtkIdType nanCount = 0;
  for(vtkIdType x=0; x<nx; x++)
    for(vtkIdType y=0; y<ny; y++)
      for(vtkIdType z=0; z<nz; z++) {
        const double value = rawArray[++gtxIndex];
        const double v = value == undefinedValue ?nanCount++, nan: value;
        vtkArray->SetValue(x+nx*(y+ny*z), (Tprimitive) v);
      }
  if(nanCount ==expectedSize) {
    vtkArray->Delete(); // result->Delete won't be happening for this array
//		vtkDebugMacro(<<"Ignoring "<<name <<": All NAN values");
    return 0;
  }
  return vtkArray;
}

vtkAbstractArray *ISATISReaderDelegate::createNumericArray(GTXClient*client, 
        vtkIdType nx, vtkIdType ny, vtkIdType nz, vtkIdType expectedSize, const char* name)
{

  GTXVariableInfo vinfo = client->GetVariableInfo();
  int bitLength = vinfo.GetBitLength();

  if( bitLength== 1)
    return createTypedArray<int, vtkBitArray>(vtkBitArray::New(),client,  nx, ny, nz, expectedSize,name);

  if( bitLength <= 8*sizeof(float))
    return createTypedArray<float, vtkFloatArray>(vtkFloatArray::New(),client,  nx, ny, nz, expectedSize,name);
  else
    return createTypedArray<double, vtkDoubleArray>(vtkDoubleArray::New(),client,  nx, ny, nz, expectedSize,name);
}

int ISATISReaderDelegate::createPoints(vtkPointSet* data, GTXClient* client, const vtkIdType expectedSize, const char** names)
{
    assert(names && names[0] && names[1] && names[2]);
    assert(data && data->GetPointData());
    const char *zzz = names[2];
    vtkPointData* pointData=data->GetPointData();
    vtkDoubleArray* xarray= vtkDoubleArray::SafeDownCast( pointData->GetArray(names[0]));
    vtkDoubleArray* yarray= vtkDoubleArray::SafeDownCast( pointData->GetArray(names[1]));
    vtkDoubleArray* zarray= strlen(names[2])>0 ? vtkDoubleArray::SafeDownCast( pointData->GetArray(names[2])) : NULL;
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(0);
    data->SetPoints(points);
    points->Delete();
    if(!xarray || !yarray ) {
        vtkErrorMacro("No X,Y coordinate arrays!");
        return 0;
    }
    if(xarray->GetNumberOfTuples() != expectedSize && yarray->GetNumberOfTuples() != expectedSize &&
            (zarray && zarray->GetNumberOfTuples() != expectedSize)) {
        vtkErrorMacro("Coordinate arrays are != expectedSize");
        return 0;
    }
    points->SetNumberOfPoints(expectedSize);
    vtkDebugMacro(<<"Creating "<<expectedSize<<" points");
    const double* x = xarray->GetPointer(0);
    const double* y = yarray->GetPointer(0);
    const double* z = zarray ? zarray->GetPointer(0) : NULL;
    assert(x && y);
    double ZDEFAULT = 0; // no Z values. This could be a parameter
    for(vtkIdType i=0; i<expectedSize; i++){
        double zValue = z!=NULL ? z[i] : ZDEFAULT;
        points->SetPoint(i,x[i],y[i],zValue);
        maxZ = (zValue > maxZ || i ==0) ? zValue : maxZ;
        minZ = (zValue < minZ || i ==0) ? zValue : minZ;
    }
    return 1;
}


int ISATISReaderDelegate::createPoints(vtkStructuredGrid* data, GTXClient* client, 
        const vtkIdType expectedNumCells, const vtkIdType expectedNumPts, const char** names, const double deltas[3])
{
  assert(names && names[0] && names[1] && names[2]);
  assert(data && data->GetCellData());
  const char *zzz = names[2];

  vtkCellData* cellData = data->GetCellData();
  vtkDoubleArray* xarray = vtkDoubleArray::SafeDownCast(cellData->GetArray(names[0]));
  vtkDoubleArray* yarray = vtkDoubleArray::SafeDownCast(cellData->GetArray(names[1]));
  vtkDoubleArray* zarray = strlen(names[2]) > 0 ? vtkDoubleArray::SafeDownCast(cellData->GetArray(names[2])) : NULL;

  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(0);

  if(!xarray || !yarray ) {
    vtkErrorMacro("No X,Y coordinate arrays!");
    return 0;
  }
  if(xarray->GetNumberOfTuples() != expectedNumCells && yarray->GetNumberOfTuples() != expectedNumCells &&
      (zarray && zarray->GetNumberOfTuples() != expectedNumCells)) {
    vtkErrorMacro("Coordinate arrays are != expectedSize");
    return 0;
  }

  points->SetNumberOfPoints(expectedNumPts);

  vtkDebugMacro(<<"Creating "<<expectedNumPts<<" points");

  const double* x = xarray->GetPointer(0);
  const double* y = yarray->GetPointer(0);
  const double* z = zarray ? zarray->GetPointer(0) : NULL;

  assert(x && y);

  double ZDEFAULT = 0; // no Z values. This could be a parameter
  
  // These are the pt-based dims.
  int ncoords[3]; 
  data->GetDimensions(ncoords);
  int ptindex = 0;
  int cellindex = 0;
  double xcoord, ycoord, zcoord;
  for (vtkIdType k = 0; k < ncoords[2]; k++) {
     for (vtkIdType j = 0; j < ncoords[1]; j++) {
        for (vtkIdType i = 0; i < ncoords[0]; i++) {
           
            // MVM: The cell-based indexing is one short of the pt-based indexing.
            // At the end of an i-row, the last index is repeated, but with 
            // a positive half-delta offset instead of negative.
            if (i < ncoords[0] - 1) {
               xcoord = x[cellindex] - deltas[0] * 0.5;
            }
            else {
                xcoord = x[cellindex] + deltas[0] * 0.5;
            }
           
            if (j < ncoords[1] - 1) {
                ycoord = y[cellindex] - deltas[1] * 0.5;
            }
            else {
                ycoord = y[cellindex] + deltas[1] * 0.5;

            }

            if (k < ncoords[2] - 1) {
                if (z) {
                    zcoord = z[cellindex] - deltas[2] * 0.5;
                }
                else {
                    zcoord = ZDEFAULT - deltas[2] * 0.5;
                }
            }
            else {
                if (z) {
                    zcoord = z[cellindex] + deltas[2] * 0.5;
                }
                else {
                    zcoord = ZDEFAULT + deltas[2] * 0.5;
                }
            }
            points->SetPoint(ptindex, xcoord, ycoord, zcoord);
            ptindex++; 

            // MVM: These is the end of the cell-based i-row length.
            if (i < ncoords[0] - 2) {
                cellindex++;
            }
        }
        // MVM: have to jump up 1 since we're behind from reusing at the
        // end of the i-row.
        cellindex++;

        // MVM: End of the cell-based j-row, reuse an i-row's worth
        // of indices for the j-th most points.
        if (j == ncoords[1] - 2) { 
            cellindex -= (ncoords[0] - 1);
        }
     }

     // MVM: End of the cell-based k-row, reuse an i-j plane's worth
     // of indices for the k-th most points.
     if (k == ncoords[2] - 2) {
         cellindex -= (ncoords[0] - 1) * (ncoords[1] - 1);
     }
  } 

  data->SetPoints(points);
  points->Delete();
  return 1;
}

int  ISATISReaderDelegate:: findXYZVarNames(GTXClient* client, vtkStdString* x,vtkStdString* y,vtkStdString* z)
{
  assert(x && y && z && client);
  GTXStringArray varList = client->GetVariableList();
  int ok = 0;
  for(int i =0; i<varList.GetCount(); i++) {
    const char* name = varList.GetValue(i);
    client->SetVariable(name);
    GTXVariableInfo::VariableType varType = client->GetVariableInfo().GetVariableType();
    if(varType == GTXVariableInfo::VAR_TYPE_XG) {
      *x=name;
      ok |=1;
    }
    if(varType == GTXVariableInfo::VAR_TYPE_YG) {
      *y=name;
      ok |= 2;
    }
    if(varType == GTXVariableInfo::VAR_TYPE_ZG) {

      *z=name;
      ok |= 4;
    }
  }
  vtkDebugMacro(<<"Using Grid Point Variable names:"<<x<<","<<y<<","<<z);
  assert(ok == 7 || ok == 3);
  return ok; // did we find X,Y and Z ?
}

int ISATISReaderDelegate::createLines(vtkUnstructuredGrid* ugrid,vtkIdType numLines, 
        vtkIdType numSamples, const char* relativename, const char* linenumname)
{
  assert(ugrid && relativename && linenumname);
  vtkDoubleArray* relativeArray = vtkDoubleArray::SafeDownCast(ugrid->GetPointData()->GetAbstractArray(relativename));
  vtkDoubleArray* lineArray = vtkDoubleArray::SafeDownCast(ugrid->GetPointData()->GetAbstractArray(linenumname));
  if( ! relativeArray || ! lineArray || relativeArray->GetNumberOfTuples() != numSamples 
          || lineArray->GetNumberOfTuples() != numSamples ) {
    vtkErrorMacro(<<"Could not find arrays of correct size "<< relativename << "," <<linenumname);
    return 0;

  }
  const double* relativeRaw = relativeArray->GetPointer(0);
  const double* lineRaw = lineArray->GetPointer(0);
  assert(relativeRaw && lineRaw);

  vtkIdType lastLineIndex = -1;
  vtkPolyLine * line = 0;
  vtkIdList* pointIds = 0;

  // Phase 1. Determine number of points per line and also check our assumptions 
  // about how the points are organized (based on our sample data)
  vtkSmartPointer<vtkIdList> histogram = vtkSmartPointer<vtkIdList>::New();
  histogram->SetNumberOfIds(numLines);

  for(int i =0; i<numLines; i++) histogram->SetId(i,0);

  vtkIdType lastIndex = -1;
  vtkIdType lastRelativeIndex = -1;
  for(vtkIdType index = 0; index <numSamples; index++) {
    vtkIdType lineIndex = lineRaw[index]-1; // gtx starts from one
    vtkIdType relIndex = relativeRaw[index]-1;
    if(lineIndex <0 || lineIndex >= numLines) {
      vtkErrorMacro( <<"Could not build line length histogram");
      return 0;
    }
    //Either (i) We're on the same line and the relative index has increased by one
    // or (ii) We've moved onto the next line and the relative index has reset to 0
    const bool expect = (lineIndex == lastIndex && relIndex ==lastRelativeIndex+1)
                        || (lineIndex == lastIndex +1 && relIndex ==0);
    if(!expect) {
      vtkErrorMacro( <<"Expected line Index to increase");
      return 0;

    }
    (*histogram->GetPointer( lineIndex)) ++;
    lastIndex = lineIndex;
    lastRelativeIndex = relIndex;
  }
  // Phase 2.  Assumptions met, so just set the pointids in the simplest way possible
  vtkIdType pointIndex = 0;
  for(int i=0; i<numLines; ++i) { // Line indices start at 1 not 0
    vtkPolyLine * line = vtkPolyLine::New();
    vtkIdList* pointIds = line->GetPointIds();
    const vtkIdType numPointsPerLine = histogram->GetId(i);
    pointIds->SetNumberOfIds(numPointsPerLine);
    for(int j=0; j<numPointsPerLine; ++j) {
      pointIds->SetId(j, pointIndex);
      ++pointIndex;
    }
    // Insert line into array
    ugrid->InsertNextCell(VTK_POLY_LINE,pointIds);
    line->Delete();
  }
  bool numPointIndicesOK = pointIndex == numSamples;
  if(!numPointIndicesOK) {
    vtkErrorMacro(<<"Incorrect number of point indices set" <<pointIndex);
    return 0;
  }
  return 1;
}


