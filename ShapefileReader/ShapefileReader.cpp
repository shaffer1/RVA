#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "ShapefileReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkPolyLine.h"
#include "vtkCellArray.h"
#include "vtkTriangle.h"
#include "vtkSmartPointer.h"
#include "vtkPolygon.h"
#include "vtkTriangleFilter.h"
#include "vtkDoubleArray.h"
#include "vtkStringArray.h"
#include "vtkIntArray.h"
#include "vtkStripper.h"
#include "vtkTriangleStrip.h"

#define POINTOBJ 1
#define LINEOBJ 2
#define FACEOBJ 3
#define MESHOBJ 4

vtkStandardNewMacro(ShapefileReader);

//---------------------------------------------------------------------------
ShapefileReader::ShapefileReader()
{
  FileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->DebugOn();

}

//----------------------------------------------------------------------------
ShapefileReader::~ShapefileReader()
{
  this->SetFileName(0);
}

//----------------------------------------------------------------------------
void ShapefileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}

int ShapefileReader::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  info->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int ShapefileReader::RequestInformation (
    vtkInformation * request,
    vtkInformationVector** inputVector,
    vtkInformationVector *outputVector)
{
  vtkInformation* info = outputVector->GetInformationObject(0);

  info->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), 1);

  return this->Superclass::RequestInformation(request, inputVector, outputVector);;
}

//----------------------------------------------------------------------------
int ShapefileReader::RequestData(
                                 vtkInformation* vtkNotUsed( request ),
                                 vtkInformationVector** vtkNotUsed(inputVector),
                                 vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* polydata = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (illegalFileName()){
    return 0;
  }

  if (incompleteSrcFiles()){
    return 0;
  }

  SHPHandle shpHandle = SHPOpen(this->FileName, "rb");
  int fileType = 0;

  if (shpHandle == NULL){
    vtkErrorMacro("shp file read failed.");
    return 0;
  }

  vtkDebugMacro("file type: " << shpHandle->nShapeType);

  switch (shpHandle->nShapeType) {
    case SHPT_NULL:
      vtkErrorMacro("NULL Object type." << this->FileName);
      SHPClose(shpHandle);
      return 0;
      break;
    case SHPT_POINT:
    case SHPT_POINTZ:
    case SHPT_POINTM:
    case SHPT_MULTIPOINT:
    case SHPT_MULTIPOINTZ:
    case SHPT_MULTIPOINTM:
      fileType = POINTOBJ;
      break;
    case SHPT_ARC:
    case SHPT_ARCZ:
    case SHPT_ARCM:
      fileType = LINEOBJ;
      break;
    case SHPT_POLYGON:
    case SHPT_POLYGONZ:
    case SHPT_POLYGONM:
      fileType = FACEOBJ;
      break;
    case SHPT_MULTIPATCH:
      fileType = MESHOBJ;
      break;
    default:
      vtkErrorMacro("Unknown Object type." << this->FileName);
      SHPClose(shpHandle);
      return 0;
  }

  int pCount=0, pStart=0, pEnd=0;
  int vTracker = 0, pid=0;
  SHPObject* shpObj;

  vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkCellArray> strips = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkCellArray> pLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkCellArray> verts = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkTriangleFilter> tFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  vtkSmartPointer<vtkStripper> stripper = vtkSmartPointer<vtkStripper>::New();
  vtkSmartPointer<vtkPolyData> pdRaw = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPolyData> pdRefined = vtkSmartPointer<vtkPolyData>::New();

  //looping through all records in file
  for (int rIndex = 0; rIndex < shpHandle->nRecords; rIndex++){
    
    shpObj = SHPReadObject(shpHandle, rIndex);

    //making sure shp object type is consistent with file type
    if (shpObj->nSHPType != shpHandle->nShapeType){
      vtkErrorMacro("Inconsistent shape type of record: " << rIndex 
        << " in file " << this->FileName);
      continue;
    }

    pCount = shpObj->nParts;
    
    switch (fileType){
    case POINTOBJ:{

      vtkIdType *ids = new vtkIdType[shpObj->nVertices];

      for (int vIndex = 0; vIndex < shpObj->nVertices; vIndex++){
        pts->InsertPoint(pid,shpObj->padfX[vIndex],
                             shpObj->padfY[vIndex],
                             shpObj->padfZ[vIndex]);
        ids[vIndex] = pid++;
      }
      verts->InsertNextCell(shpObj->nVertices, ids);
      delete ids;
      ids = NULL;
      break;

    }
    case LINEOBJ:{
      
      for (int pIndex = 0; pIndex < pCount; pIndex++){
        pStart = shpObj->panPartStart[pIndex];

        if (pIndex == (pCount-1)){
          pEnd = shpObj->nVertices;
        }else{
          pEnd = shpObj->panPartStart[pIndex+1];
        }

        vtkSmartPointer<vtkPolyLine> pLine = vtkSmartPointer<vtkPolyLine>::New();
        pLine->GetPointIds()->SetNumberOfIds(pEnd-pStart); 

        for (int vIndex = pStart; vIndex < pEnd; vIndex++){
          pts->InsertNextPoint(shpObj->padfX[vIndex],
                               shpObj->padfY[vIndex],
                               shpObj->padfZ[vIndex]);
          pLine->GetPointIds()->SetId(vIndex-pStart, vTracker+vIndex-pStart);
        }
        pLines->InsertNextCell(pLine);
        vTracker += pEnd-pStart;
      }
      break;

    }
    case FACEOBJ:{

      for (int pIndex = 0; pIndex < pCount; pIndex++){
        pStart = shpObj->panPartStart[pIndex];

        if (pIndex == (pCount-1)){
          pEnd = shpObj->nVertices;
        }else{
          pEnd = shpObj->panPartStart[pIndex+1];
        }
        vtkSmartPointer<vtkPolygon> poly = vtkSmartPointer<vtkPolygon>::New();
        vtkSmartPointer<vtkPolyLine> pLine = vtkSmartPointer<vtkPolyLine>::New();
        poly->GetPointIds()->SetNumberOfIds(pEnd-pStart); 
        pLine->GetPointIds()->SetNumberOfIds(pEnd-pStart); 

        for (int vIndex = pStart; vIndex < pEnd; vIndex++){
          pts->InsertNextPoint(shpObj->padfX[vIndex],
                               shpObj->padfY[vIndex],
                               shpObj->padfZ[vIndex]);
          poly->GetPointIds()->SetId(vIndex-pStart, vTracker+vIndex-pStart);
          pLine->GetPointIds()->SetId(vIndex-pStart, vTracker+vIndex-pStart);
        }

        polys->InsertNextCell(poly);
        pLines->InsertNextCell(pLine);
        vTracker += pEnd-pStart;
      }
      break;

    }
    case MESHOBJ:{

      switch (*(shpObj->panPartType)){

      case SHPP_TRISTRIP:{
        for (int pIndex = 0; pIndex < pCount; pIndex++){
          pStart = shpObj->panPartStart[pIndex];

          if (pIndex == (pCount-1)){
            pEnd = shpObj->nVertices;
          }else{
            pEnd = shpObj->panPartStart[pIndex+1];
          }

          vtkSmartPointer<vtkTriangleStrip> strip = vtkSmartPointer<vtkTriangleStrip>::New();
          strip->GetPointIds()->SetNumberOfIds(pEnd-pStart);

          for (int vIndex = pStart; vIndex < pEnd; vIndex++){
            pts->InsertNextPoint(shpObj->padfX[vIndex],
                                 shpObj->padfY[vIndex],
                                 shpObj->padfZ[vIndex]);
            strip->GetPointIds()->SetId(vIndex-pStart, vTracker+vIndex-pStart);
          }

          strips->InsertNextCell(strip);
          vTracker += pEnd-pStart;
        }
        break;
      }//SHPP_TRISTRIP
      case SHPP_TRIFAN:{
        for (int pIndex = 0; pIndex < pCount; pIndex++){
          pStart = shpObj->panPartStart[pIndex];

          if (pIndex == (pCount-1)){
            pEnd = shpObj->nVertices;
          }else{
            pEnd = shpObj->panPartStart[pIndex+1];
          }

          vtkSmartPointer<vtkTriangleStrip> strip = vtkSmartPointer<vtkTriangleStrip>::New();
          strip->GetPointIds()->SetNumberOfIds(pEnd-pStart);

          for (int vIndex = pStart; vIndex < pEnd; vIndex++){
            pts->InsertNextPoint(shpObj->padfX[vIndex],
                                 shpObj->padfY[vIndex],
                                 shpObj->padfZ[vIndex]);
            strip->GetPointIds()->SetId(vIndex-pStart, vTracker+vIndex-pStart);
          }

          strips->InsertNextCell(strip);
          vTracker += pEnd-pStart;
        }
        break;
      }//SHPP_TRIFAN
      case SHPP_OUTERRING:
      case SHPP_INNERRING:
      case SHPP_FIRSTRING:
      case SHPP_RING:{
        for (int pIndex = 0; pIndex < pCount; pIndex++){
        pStart = shpObj->panPartStart[pIndex];

        if (pIndex == (pCount-1)){
          pEnd = shpObj->nVertices;
        }else{
          pEnd = shpObj->panPartStart[pIndex+1];
        }
        vtkSmartPointer<vtkPolygon> poly = vtkSmartPointer<vtkPolygon>::New();
        vtkSmartPointer<vtkPolyLine> pLine = vtkSmartPointer<vtkPolyLine>::New();
        poly->GetPointIds()->SetNumberOfIds(pEnd-pStart); 
        pLine->GetPointIds()->SetNumberOfIds(pEnd-pStart); 

        for (int vIndex = pStart; vIndex < pEnd; vIndex++){
          pts->InsertNextPoint(shpObj->padfX[vIndex],
                               shpObj->padfY[vIndex],
                               shpObj->padfZ[vIndex]);
          poly->GetPointIds()->SetId(vIndex-pStart, vTracker+vIndex-pStart);
          pLine->GetPointIds()->SetId(vIndex-pStart, vTracker+vIndex-pStart);
        }

        polys->InsertNextCell(poly);
        pLines->InsertNextCell(pLine);
        vTracker += pEnd-pStart;
      }
      break;
      }//rings
      }//panPartType
      break;
    
    }//MESHOBJ
    }//fileType

    SHPDestroyObject(shpObj);
  }// rIndex
  SHPClose(shpHandle);
  
  DBFHandle dbfHandle = DBFOpen(this->FileName, "rb");
  int fieldCount = DBFGetFieldCount(dbfHandle);
  
  for (int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++){
    
    DBFFieldType fieldType;
    const char *typeName;
    char nativeFieldType;
    char pszFieldName[12];
    int pnWidth=0, pnDecimals=0, recordCount=0;

    nativeFieldType = DBFGetNativeFieldType(dbfHandle, fieldIndex);
    fieldType = DBFGetFieldInfo(dbfHandle, fieldIndex, pszFieldName, &pnWidth, &pnDecimals);
    recordCount = DBFGetRecordCount(dbfHandle);

    vtkDebugMacro("record count: " << recordCount);

    switch (fieldType){
    case FTString:{

      vtkSmartPointer<vtkStringArray> cellData = vtkSmartPointer<vtkStringArray>::New();

      cellData->SetName(pszFieldName);
      
      for (int recordIndex = 0; recordIndex < recordCount; recordIndex++){
        if (DBFIsAttributeNULL(dbfHandle, recordIndex, fieldIndex)){
          cellData->InsertNextValue("NULL");
        }else{
          cellData->InsertNextValue(DBFReadStringAttribute(dbfHandle, recordIndex, fieldIndex));
        }
      }
      
      polydata->GetCellData()->AddArray(cellData);

      break;
    }
    case FTInteger:{

      vtkSmartPointer<vtkIntArray> cellData = vtkSmartPointer<vtkIntArray>::New();

      cellData->SetName(pszFieldName);
      
      for (int recordIndex = 0; recordIndex < recordCount; recordIndex++){
        if (DBFIsAttributeNULL(dbfHandle, recordIndex, fieldIndex)){
          cellData->InsertNextValue(0);
        }else{
          cellData->InsertNextValue(DBFReadIntegerAttribute(dbfHandle, recordIndex, fieldIndex));
        }
      }
      
      polydata->GetCellData()->AddArray(cellData);

      break;
    }
    case FTDouble:{

      vtkSmartPointer<vtkDoubleArray> cellData = vtkSmartPointer<vtkDoubleArray>::New();

      cellData->SetName(pszFieldName);
      
      for (int recordIndex = 0; recordIndex < recordCount; recordIndex++){
        if (DBFIsAttributeNULL(dbfHandle, recordIndex, fieldIndex)){
          cellData->InsertNextValue(0.0);
        }else{
          cellData->InsertNextValue(DBFReadDoubleAttribute(dbfHandle, recordIndex, fieldIndex));
        }
      }
      
      polydata->GetCellData()->AddArray(cellData);

      break;
    }
    case FTLogical:{
      //what is this? lack of sample data!
      break;
    }
    case FTInvalid:{
      vtkErrorMacro("Invalid DBF field type");
      break;
    }    
    }
  }

  DBFClose(dbfHandle);

  
  //pdRaw->SetPoints(pts);
  //pdRaw->SetPolys(polys);
  //pdRaw->SetLines(pLines);
  //pdRaw->SetVerts(verts);

  //tFilter->SetInput(pdRaw);
  //tFilter->Update();
  //pdRefined = tFilter->GetOutput();

  //polydata->SetPoints(pdRefined->GetPoints());
  //polydata->SetPolys(pdRefined->GetPolys());
  //polydata->SetLines(pdRefined->GetLines());
  //polydata->SetVerts(pdRefined->GetVerts());
  
  
  polydata->SetPoints(pts);
  polydata->SetLines(pLines);
  polydata->SetVerts(verts);
  polydata->SetStrips(strips);

  return 1;
}


static bool isFileBad(const char* fname, int extOffset, const char* upperExt){
  
  ifstream testStream(fname);

  //testing lowercase extension
  if ( testStream.good() ){
    testStream.close();
    return false;
  }
  
  //constructing uppercase extension
  std::string upperExtFname = fname;
  upperExtFname.replace(extOffset, 4, upperExt);
  testStream.open(upperExtFname.c_str());

  //testing uppercase extension
  if ( testStream.good() ){
    testStream.close();
    return false;
  }

  return true;
}



bool ShapefileReader::incompleteSrcFiles(){

  //constructing files names ending with three extensions
  std::string shxName = this->FileName;
  std::string dbfName = this->FileName;
  int extOffset = shxName.find_last_of(".");

  shxName.replace(extOffset, 4, ".shx");
  dbfName.replace(extOffset, 4, ".dbf");

  //check for readability of required .shx/SHX .shp/SHP .dbf/DBF files

  if ( isFileBad(this->FileName, extOffset, ".SHX") ) {
    vtkErrorMacro("Could not find/read the .shx/SHX file.");
    return true;
  }

  if ( isFileBad(shxName.c_str(), extOffset, ".SHP") ) {
    vtkErrorMacro("Could not find/read the .shp/SHP file.");
    return true;
  }

  if ( isFileBad(dbfName.c_str(), extOffset, ".DBF") ) {
    vtkErrorMacro("Could not find/read the .dbf/DBF file.");
    return true;
  }

  return false;
}



bool ShapefileReader::illegalFileName(){

  //check for null pointer
  if(!this->FileName)  {
    vtkErrorMacro("No FileName specified.");
    return true;
  }

  //check for zero length string
  if(strlen(this->FileName)==0)  {
    vtkErrorMacro("A zero length FileName.");
    return true;
  }

  return false;
}


void ShapefileReader::addTriangle(vtkSmartPointer<vtkCellArray> triangles, int a, int b, int c){

  vtkTriangle* t = vtkTriangle::New();

  t->GetPointIds()->SetId(0, a);
  t->GetPointIds()->SetId(1, b);
  t->GetPointIds()->SetId(2, c);

  triangles->InsertNextCell(t);
  
  t->Delete();
}


