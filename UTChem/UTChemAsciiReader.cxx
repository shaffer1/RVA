/*=========================================================================

Program:   RVA
Module:    UTChemConcReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Duggirala, D McWherter, U Yadav

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "UTChemAsciiReader.h"
#include "UTChemInputReader.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cassert>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cctype>

#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkStructuredGrid.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkDataObject.h"
#include "vtkCellData.h"
#include "vtkImageData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkRectilinearGrid.h"

#include <RVA_Util.h>

UTChemAsciiReader::UTChemAsciiReader() :
  dataObj(NULL), FileName(0) 
{
   *this->phaseName = '\0';

  this->InputInfo = new UTChemInputReader("");
  this->SetDebug(1);
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

UTChemAsciiReader::~UTChemAsciiReader() 
{
  freeDataVectors();
  SetFileName(0);
  delete this->InputInfo;
  this->InputInfo=NULL;
  if (dataObj) 
  {
    dataObj->Delete();
  }
  dataObj=NULL;
}

int UTChemAsciiReader::ProcessRequest(vtkInformation* request,
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{
  // MVM: not sure this is really necessary.
  if (!request || !outputVector) {
    return 0; // Paranoia e.g. this object deleted but still in pipeline
  }

  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
  {
    return RequestDataObject(request,inputVector,outputVector);
  }

  if (request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
  {
    return RequestInformation(request,inputVector,outputVector);
  }

  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
  {
    return RequestData(request,inputVector,outputVector);
  }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

int UTChemAsciiReader::FillOutputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataSet");
  return 1;
}

int UTChemAsciiReader::RequestDataObject(vtkInformation* req, 
                                         vtkInformationVector** inVect, 
                                         vtkInformationVector* outVect)
{
  vtkInformation * info = outVect->GetInformationObject(0);
  // MVM: why reload?
  reloadInputFile(FileName);
  if (dataObj) 
  {
    dataObj->Delete();
  }

  dataObj = InputInfo->getObject(info); // also does SetPipelineInformation
  dataObj->Register(this);
  int extType = dataObj->GetExtentType();
  vtkInformation * algInfo = this->GetOutputPortInformation(0);
  algInfo->Set(vtkDataObject::DATA_EXTENT_TYPE(), extType);

  return 1;
}

int UTChemAsciiReader::RequestInformation(vtkInformation *vtkNotUsed(request), 
                                          vtkInformationVector **vtkNotUsed(inputVector),
                                          vtkInformationVector *outVec)
{
  if (!FileName)
  {
    return 0;
  }
  
  if (timeList.size() > 0) 
  {
    vtkErrorMacro(<<"Trying to reading file more than once!");
  }
  else 
  {
	  int success = readFile(); // 0 if failed, 1 if successful
	  
	  if (!success) 
    {
		  vtkErrorMacro("File parsing failed");
		  return 0; // Fail
	  }
  }
  
  vtkInformation * outInfo  = outVec->GetInformationObject(0);

  // MVM: move to ::RequestUpdateExtent?
  // ParaView plugins 101... Set the extent or you won't see anything :)
  int extent[6] = {0,nx,0,ny,0,nz}; // For CellData of nx*ny*nz cells

  double timerange[2] = { timeList[0], timeList[timeList.size()-1] };

  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timerange, 2);

  // Discrete time-steps are what we need
  // This was useful: http://www.paraview.org/pipermail/paraview/2011-April/021264.html

  // Uses direct pointer to internal array of std:vector
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), &timeList[0],(int) timeList.size());

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);

  return 1; // Load file data as soon as we open it
}

int UTChemAsciiReader::RequestData(vtkInformation *vtkNotUsed(request), 
                                   vtkInformationVector **vtkNotUsed(inputVector),
                                   vtkInformationVector *outVec)
{
  if (!FileName) 
  {
    return 0; // Error
  }

  vtkInformation * outInfo  = outVec->GetInformationObject(0);
  double* requestedTimeSteps = NULL;
  unsigned bestidx = 0;
  currentTimeStep = NULL;

  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS())) 
  {
    requestedTimeSteps = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS());
  }
 
  if (requestedTimeSteps == NULL) 
  {
    vtkErrorMacro(<<"Strange. No Time steps were requested by Paraview")
    return 0;
  }
  
  bestidx = findClosestTimeStep(requestedTimeSteps[0]);

  if ( bestidx>= allData.size()) 
  {
    vtkErrorMacro(<< "No data available: Invalid time step index " << bestidx)
    return 0; // Failure
  }
  assert(timeList.size() == allData.size());
	assert(InputInfo);

  return buildVTKObject(bestidx, outInfo);
}

void UTChemAsciiReader::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "File name: "<< (FileName ? FileName : "(none)") << "\n";
  Superclass::PrintSelf(os, indent);
}

std::string UTChemAsciiReader::getInputFileFromFileName(const char* filename)
{
  std::string tmp(filename);

  // Two different types since windows file paths are different than *nix
#ifdef _WIN32
  std::string inputfile(tmp.substr(0, tmp.find_last_of("\\")));
  inputfile.append("\\INPUT");
#else
  std::string inputfile(tmp.substr(0, tmp.find_last_of("/")));
  inputfile.append("/INPUT");
#endif
  return inputfile;
}

// Supported File Types
// Returns empty string for non-supported types
const char* UTChemAsciiReader::fileExtensionToLabel(std::string&ext)
{
  if (ext == "CONCP" || ext == "COMP_AQ" || ext == "COMP_ME" || ext == "COMP_OIL") return "Concentration";
  if (ext == "SATP") return "Saturation";
  if (ext == "PRESP") return "Pressure";
  if (ext == "VISC") return "Viscosity";
  if (ext == "TEMPP") return "Temperature";
  if (ext == "PERM") return "Permeability";
  if (ext == "ALKP") return "Alkaline";
  return "";
}

// MVM: what is this for? What names need to be sanitized?
static std::string sanitizeName(std::string name)
{
  const char *cname = name.c_str();
  std::string ret;
  
  // MVM add underscore if it starts with a numeral
  if (isdigit(*cname)) 
  { 
    ret +='_';
  }
  
  // MVM replace spaces, pluses, or minuses with underscore
  for(; *cname ; cname++)
  {
    char c = *cname;
    if (isalnum(c) || c == '_') 
    {
      ret += c;
    }
    else if ((c==' ' || c == '+' || c == '-') 
            && ret.length() > 0 && *(ret.end()-1) != '_') 
    {
      ret += '_';
    }
  }
  return ret;
}

// Sets floatarray's name using instance vars componentNames and phaseName
void UTChemAsciiReader::setMeaningfulArrayName(vtkFloatArray* array,
        int thePhase, std::string arrName, bool absolutePhase)
{
  if (!arrName.empty()) 
  {
    std::string niceName = sanitizeName(arrName);
    array->SetName(niceName.c_str());
    return;
  }

  std::ostringstream name;
  name << fileExtensionToLabel(file_ext);
  name << setfill('0') << setw(2) << phase;
  if (componentNames.count(thePhase)) 
  {
    name<<"_"<<componentNames[thePhase];
  } 
  else if (absolutePhase) {
    // absolutePhase for instances such as ALKP file where
    // phases can mean something other than these standard
    // default parameters.
    name << "_";
    switch (phase) {
      case 1:
        name << "Water";
        break;
      case 2:
        name << "Oil";
        break;
      case 3:
        name << "Surfactant";
        break;
      case 4:
        // Polymer or silicate (KGOPT=3) [weight percent]
        // TODO: Read in KGOPT for this
        name << "Polymer";
        break;
      case 5:
        name << "Anion";
        break;
      case 6:
        name << "Cation";
        break;
      case 7:
        name << "Alcohol_1";
        break;
      case 8:
        if(InputInfo->igas >= 1)
          name << "Gas";
        else
          name << "Alcohol_2";
        break;
    }
  }

  if (*phaseName != '\0') 
  {
    name<< "_"<< phaseName;
    *phaseName = '\0';
  }
  std::string niceName(sanitizeName(name.str().c_str()));
  array->SetName(niceName.c_str());
}

void UTChemAsciiReader::freeDataVectors()
{
  currentTimeStep = NULL;
  componentNames.clear();
  timeList.clear();
  //foreach timestep
  // foreach phase
  for (unsigned int timestepIndex = 0 ; timestepIndex < allData.size(); ++timestepIndex) {
    IntegerTovtkFloatArrayMap* scalars = allData[timestepIndex];
    if (!scalars)
    {
      continue;
    }
    
    IntegerTovtkFloatArrayMap_it it = scalars->begin(), end = scalars->end();
    
    for(; it != end; it++) 
    {
      vtkFloatArray* array = (*it).second;
      //assert(array);
      if (array) 
      {
        array->Delete();
      }
    }
    scalars->clear();
    delete scalars;
    scalars = NULL;
  }
  allData.clear();
  nx=-1,ny=-1,nz=-1;
  fileLength = 0;
  line_num = 0;
  layer = 0;
  timestep=0;
  time=0;
}

// MVM: why are we reloading? Just because the ctor makes a 
// InputInfo object without a file?
void UTChemAsciiReader::reloadInputFile(const char*filename) {
  std::string inputfile(getInputFileFromFileName(filename));
  delete this->InputInfo;
  this->InputInfo = new UTChemInputReader(inputfile); 
}

// MVM: ... why is this handling time and not using a Temporal Filter?
// Returns index of closest known timestep for requested animation time
unsigned UTChemAsciiReader::findClosestTimeStep(double& reqTime)
{
  unsigned bestindex = 0;
  for (unsigned i = 1 ; i < timeList.size(); ++i) 
  {
    // Find closest timestep (since may not be exact)
    if (abs(reqTime - timeList[i]) < abs(reqTime - timeList[bestindex])) 
    {
      bestindex = i;
      if (reqTime == timeList[bestindex]) 
      {
        break;
      }
    }
  }
  return bestindex;
}

// Helper method to determine which kind of vtk object to construct
int UTChemAsciiReader::buildVTKObject(const unsigned& bestidx, vtkInformation* outInfo)
{
  int ret = 0;

  if (InputInfo == NULL)
  {
    return 0;
  }

	vtkDataSet * dataSet = vtkDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  currentTimeStep = allData[bestidx];
  assert(currentTimeStep);
  
  if (!currentTimeStep) 
  {
    vtkErrorMacro(<<"No data arrays are not unset for time step index "<<bestidx)
    return 0;
  }
 
  if (currentTimeStep->size() == 0) 
  {
    vtkErrorMacro(<<"Strange... This timestep has no data arrays. time index="<<bestidx)
  }

  // Do some magic and figure out what to make
  switch (InputInfo->getObjectType()) 
  {
    case 0:
      ret = buildImageData(dataSet);
      break;
    case 1:
      ret = buildRGridData(dataSet);
      break;
		case 2:
			ret = buildSGridData(dataSet);
			break;
    default:
      break;
  }
	
  if (ret)
	{
		dataSet->GetInformation()->Set(vtkDataObject::DATA_TIME_STEPS(), &timeList[bestidx], 1);
		IntegerTovtkFloatArrayMap_it it = currentTimeStep->begin(), end = currentTimeStep->end();
		for (; it != end; it++) 
    {
			vtkFloatArray* array = (*it).second;
			dataSet->GetCellData()->AddArray(array);
		}
		dataSet->GetCellData()->AddArray(InputInfo->cellVolume);
  }
  return ret;
}

// helper method to construct an image data object for a specific animation time request
int UTChemAsciiReader::buildImageData(vtkDataSet * dataSet)
{
  vtkImageData * imgData = vtkImageData::SafeDownCast(dataSet);

  imgData->Initialize();

  if (InputInfo->icoord == 1) {
  	imgData->SetDimensions(nx+1, ny+1, nz+1); // For CellData of nx*ny*nz cells
  	imgData->SetSpacing(InputInfo->dx1,InputInfo->dy1,InputInfo->dz1);
  }
  else if (InputInfo->icoord == 2) {
	imgData->SetDimensions(nx+1, 1, nz+1);
  	imgData->SetSpacing(InputInfo->dx1, 0, InputInfo->dz1);
  }
	

  return 1;
}

// helper method to construct an rectilinear data object for a specific animation time request
int UTChemAsciiReader::buildRGridData(vtkDataSet * dataSet)
{
  vtkRectilinearGrid * rgrid = vtkRectilinearGrid::SafeDownCast(dataSet);
  vtkPoints * points = NULL;

  rgrid->Initialize();

  // "The coordinates are assigned to the rectilinear grid. Make sure that
  // the number of values in each of the XCoordinates, YCoordinates, 
  // and ZCoordinates is equal to what is defined in SetDimensions()." - from  vtk example
  //
  // nx..z is the number of cells and we have cell-centered data

  	rgrid->SetDimensions(nx+1, ny+1, nz+1); // For CellData of nx*ny*nz cells

  	assert(InputInfo->xdim &&InputInfo->ydim &&InputInfo->zdim);
  	if(InputInfo->xdim &&InputInfo->ydim &&InputInfo->zdim ) {
    	rgrid->SetXCoordinates((vtkDataArray*)InputInfo->xdim);
    	rgrid->SetYCoordinates((vtkDataArray*)InputInfo->ydim);
    	rgrid->SetZCoordinates((vtkDataArray*)InputInfo->zdim);
  	}
  
	/*
	else {
	  vtkDoubleArray *y = vtkDoubleArray::New();
	  y->InsertNextValue(0.0);
	  rgrid->SetDimensions(nx+1, 1, nz+1);
	  rgrid->SetXCoordinates((vtkDataArray*)InputInfo->xdim);
	  rgrid->SetYCoordinates((vtkDataArray*)y);
	  rgrid->SetZCoordinates((vtkDataArray*)InputInfo->zdim);
	  y->Delete();
  }*/

  return 1;
}

// helper method to construct an structured grid data object for a specific animation time request
int UTChemAsciiReader::buildSGridData(vtkDataSet * dataSet)
{
  vtkStructuredGrid * sgrid = vtkStructuredGrid::SafeDownCast(dataSet);

  sgrid->Initialize();

  sgrid->SetDimensions(nx+1, ny+1, nz+1); // For CellData of nx*ny*nz cells

	assert(InputInfo->points && InputInfo->points->GetNumberOfPoints()==(nx+1)*(ny+1)*(nz+1));
	if (InputInfo->points) 
  {
		sgrid->SetPoints(InputInfo->points);
  }

  return 1;
}

void UTChemAsciiReader::readNXNYnumericalValuesIntoArray(float*output)
{
  int i = 0;
  int expected = nx * ny;
 
  while (i < expected) 
  {
	  stream >> output[i];
	  i++;
  }
}

// Uses current phase and layer state to find correct float array
// Then calls readNXNYnumericalValuesIntoArray to perform the actual numerical parsing
int UTChemAsciiReader::readLayerValues(std::string name, bool absolutePhase)
{
  unsigned arraySize = nx*ny*nz;

  if (currentTimeStep == NULL || arraySize==0)
  {
    //throw std::exception("Can't readLayerValues when there's no current time step (or proper dimensions)");
	  throw std::runtime_error("Can't readLayerValues when there's no current time step (or proper dimensions)");
	}
  
  if (phase<0) 
  {
//    throw std::exception("Negative phase values are unsupported"); // meaningful name requires non-neg
	  throw std::runtime_error("Negative phase values are unsupported"); 
  }
  
  if ( !currentTimeStep->count(phase)) 
  {
    vtkFloatArray* floatArray = vtkFloatArray::New();
    floatArray->SetNumberOfValues(arraySize);
    floatArray->FillComponent(0,std::numeric_limits<float>::quiet_NaN() );
    setMeaningfulArrayName(floatArray, phase, name, absolutePhase );
    (*currentTimeStep)[phase]=floatArray;
  }
  oneGrid = (*currentTimeStep)[phase]->GetPointer(0);
  assert(oneGrid);
  assert(layer>0 && layer<=nz);
  if (!oneGrid || layer<=0 || layer > nz) 
  {
    //throw std::exception("Unexpected layer value");
	  throw std::runtime_error("Unexpected layer value");
  }
  float* ptr = oneGrid+(nx*ny*(layer-1));

  readNXNYnumericalValuesIntoArray(ptr); 

  return 1; //OK
}


// Returns either: 0=Bad file (and no stream is open) or
// 1=Good stream (and file extension and length of file is known)
int UTChemAsciiReader::initializeStream()
{
  this->file_ext = toUpperCaseFileExtension(FileName);
  if (file_ext.empty()) 
  {
    vtkErrorMacro(<<"File extension not found:"<<FileName)
    return 0;
  }
  try {
    stream.exceptions( std::ifstream::badbit | std::ifstream::failbit);
    stream.open(FileName); // may set failbit

    stream.seekg (0, ios::end);
    fileLength  = (double) stream.tellg(); // approximate
    if (fileLength < 1)
    { 
      fileLength = 1;// paranoia as we divide by fileLength later
    }
    stream.seekg (0, ios::beg);

    stream.exceptions( std::ifstream::badbit );
    // getline may set std::ifstream::failbit if last line is blank-
    //"the function extracts no characters, it calls is.setstate(ios_base::failbit) which may throw ios_base::failure (27.5.5.4)."

  } catch(const std::exception& e) {
    vtkErrorMacro(<<"Could not open the file. Exception:"<<e.what());
    try {
      stream.close();
    } catch (...) {}
    return 0; // Failed
  }
  return 1; // Success
}

const char* UTChemAsciiReader::readNextLine(bool mustBeNonEmpty) {
	  std::getline(stream, nextLine);
	  if (mustBeNonEmpty && 0 == nextLine.length()) 
    {
//		  throw std::exception("Expected to read non-blank line from file");
		  throw std::runtime_error("Expected to read non-blank line from file");
	  }
    const char* c_str= nextLine.c_str();
    line_num++;
	  return c_str;
}

// Reads a UTChem data file (.CONC / .VISC etc )
int UTChemAsciiReader::readFile()
{
  if (!FileName) 
  {
    return 0;
  }
  freeDataVectors(); // we reset our own parsing state here

  this->SetProgressText(FileName);

  if (!initializeStream()) 
  {
    return 0; // Stream has been closed for us
  }

  bool failed = false;

  try {
	  
    readHeader(); // gets valid nx,ny,nz or throws exception. Subclasses should also reset their state here

    while (true) 
    {
      const char* c_str= readNextLine(false);

      if (stream.eof()) 
      {
	      break;
	  	}

      while (*c_str == ' ') 
      {
        c_str++; // skip leading spaces
	    }

      if (!*c_str) 
      {// empty line. Rinse and Repeat
        continue;
	    }

      int parsed = parseLine(c_str);

      if (!parsed) 
      {
        vtkErrorMacro(<<"Could not parse line #"<<line_num<<":'"<<nextLine<<"'");
        //throw std::exception("Parse failed");
		    throw std::runtime_error("Parse failed");
      }
    }// while
  } catch (const std::exception& e) {
    failed = true;
    vtkErrorMacro(<<"Exception :" <<e.what());
  }
  try {
    stream.close();
  } catch (...) {}

  currentTimeStep = NULL;
  vtkDebugMacro(<<"Lines read:"<<line_num);
  
  if (!failed) 
  {
    vtkDebugMacro(<<"File reading completed without error.")
  } 
  else 
  {
    freeDataVectors();
  }
 
  this->UpdateProgress(1.0);

  vtkDebugMacro(<<" nx*ny*nz="<<(nx*ny*nz)<<" timeList size = "<<timeList.size()<<" data size = "<<allData.size() )

	return !failed && validFileRead(); // to be valid we did not choke and read at least one time step
}

bool UTChemAsciiReader::validFileRead()
{
  return timeList.size() > 0;
}
