/*=========================================================================

Program:   RVA
Module:    UTChemWellReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Li, D McWherter

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// Include our header
#include "UTChemWellReader.h"
#include "UTChemInputReader.h"

// Standard libraries
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>

// Useful vtk/paraview headers
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTable.h"

#include <RVA_Util.h>




// --------------------------------------------------------
vtkStandardNewMacro(UTChemWellReader);

// constructor
UTChemWellReader::UTChemWellReader()
{
	readHeader();
  this->SetNumberOfOutputPorts(2);
}

UTChemWellReader::~UTChemWellReader()
{
}



int UTChemWellReader::CanReadFile(const char* filename)
{
  if(!filename)
    return 0;

  // MVM: at this point, has UTChemAsciiReader grabbed the 
  // extension? if not, why not? 
  std::string ext = toUpperCaseFileExtension(filename);

  if(ext.substr(0,4) != "HIST")
    return 0;

  return 1;
}



int UTChemWellReader::FillOutputPortInformation(int port, vtkInformation* info)
{
  if(port == 1)
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
  else
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}



int UTChemWellReader::RequestData(
                                  vtkInformation* vtkNotUsed(request),
                                  vtkInformationVector** vtkNotUsed(inputVector),
                                  vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(1);
  vtkTable* table = vtkTable::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));


  assert(varCount == dataLabel.size());
  assert( data.size() == varCount * timeStepCount);
  if( !table || varCount != dataLabel.size() || data.size() != varCount * timeStepCount)
  	return 0; // Things have gone badly wrong.

  // creating empty arrays to hold points
  for (int i = 0; i < varCount; i++){
    vtkFloatArray * col = vtkFloatArray::New();
    col->SetName(dataLabel[i].c_str()); // adding data labels
    table->AddColumn(col);
    col->Delete();
  }

  table->SetNumberOfRows(timeStepCount);

  // adding data points to all row, col positions
  for (int row = 0; row < timeStepCount; row++)
    for (int col = 0; col < varCount; col++)
      table->SetValue(row, col, data[row*varCount+col]);

  // Visual information
  vtkInformation* outViz = outputVector->GetInformationObject(0);
  vtkPolyData* data = vtkPolyData::SafeDownCast(outViz->Get(vtkDataObject::DATA_OBJECT()));

  buildWell(data);

  return 1;
}


int UTChemWellReader::RequestInformation(
                                          vtkInformation *vtkNotUsed(request),
                                          vtkInformationVector **vtkNotUsed(inputVector),
                                          vtkInformationVector *outVec)
{
  if(!FileName)
    return 0;

  if( timeList.size()>0) {
    vtkErrorMacro(<<"Trying to reading file more than once!");
  } else {
	  int success = readFile(); // 0 if failed, 1 if successful

	  if(!success) {
		  vtkErrorMacro("File parsing failed");
		  return 0; // Fail
	  }
  }

  // Visual information
  vtkInformation* outViz = outVec->GetInformationObject(0);

  int nx = InputInfo->nx;
  int ny = InputInfo->ny;
  int nz = InputInfo->nz;

  int bnd[] = { 0, nx, 0, ny, 0, nz };

  outViz->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), bnd, 6);

  return 1; // Load file data as soon as we open it
}


// MVM: if this is used as an initializer/resetter, then this is
// a horrible name.
// Note this method is called by the constructor to reset our internal state
void UTChemWellReader::readHeader()
{
    headerCount = 4;
    phaseCount = 0;
    wellBlockCount = 0;
    componentCount = 0;
    timeStepCount = 0;
    varCount = 0;

    niaq = 0;
    nfld = 0;
    nsld = 0;

    wellId = 0;
    wellType = 0;

    wellName.clear();
    dataLabel.clear();
    data.clear();
}



// Reads a UTChem data file
int UTChemWellReader::parseLine(const char* c_str)
{
  // for HIST file format, see notes in the end of header file

  if (contains(c_str, "RUN NAME IS")){
    return parseAsWellHeader(c_str);
  }else if (contains(c_str, "HISTORY DATA FOR WELL")){
    return parseAsWellHistoryData(c_str);
  }else if (contains(c_str, "LIST OF VARIABLES ARE")){
    return parseAsWellVariables(c_str);
  }else{
    return parseAsWellTable(c_str);
  }
}



int UTChemWellReader::parseAsWellHeader(const char *c_str)
{
  // for now not interested in the header, just skip 4 lines

  for (int i = 0; i < headerCount; i++){
    const char* line = readNextLine(false);
  }

  return 1;
}



int UTChemWellReader::parseAsWellHistoryData(const char *c_str)
{
  char wellNameChars[100];
  int a=0, b=0;

  int read = sscanf(c_str, " HISTORY DATA FOR WELL: ID = %i IFLAG = %i NAME = %99s ", &a, &b, wellNameChars);
  wellNameChars[sizeof(wellNameChars)-1]='\0'; //Paranoia

  if (read != 3){
    vtkErrorMacro(<<"Unexpected history data format");
    return 0;
  }

  removeTrailingChar(wellNameChars, ' ');

  wellId = a;
  wellType = b;
  wellName = wellNameChars;

  return 1;
}



int UTChemWellReader::parseAsWellVariables(const char* c_str)
{
  if (wellType == 1 || wellType == 3)
    return parseAsInjectorVariable();
  else if(wellType == 2 || wellType ==4)
    return parseAsProducerVariable();
  else {
    vtkErrorMacro(<<"Unknown well Type."<<wellType<<": Expected 1,2,3 or 4.");
    return 0;
  }
}



int UTChemWellReader::parseAsWellTable(const char* c_str)
{
  std::string token;
  std::stringstream line(c_str);
  float f;
  std::stringstream ss;
  while (std::getline(line, token , ',')) {
	  ss.str(token);
	  ss >> f;
	  std::cout << "float data: " << f << std::endl;
	  data.push_back(f);
	  ss.clear();
  }
   
  return 1;
}



int UTChemWellReader::parseAsEndVarSection(const char* c_str)
{
  varCount = 0;
  int read = sscanf(c_str, " TOTAL NO. OF VARIABLES FOR %*s %*s %*s %*s %i ", &varCount);

  if (1!=read){
    vtkErrorMacro(<<"Unexpected variable count format");
    return 0;
  }

  return 1;
}


// Helper method to determine the number of entries
// e.g. 4- 6 would return 3
static int getVarRange(const char* c_str)
{
  assert(c_str);
  int start = 0;
  int end = 0;

  int read = sscanf(c_str, "%i- %i", &start, &end); // assuming 7- 9 : PHASE CUTS FOR EACH PHASE

  if (read) {
    return (end - start + 1);
  }else{
    return 0;
  }
}



int UTChemWellReader::parseAsProducerVariable()
{
    while (true) {

        const char* line = readNextLine(false);

        if (strlen(line) == 0) {
            continue; // skip blank lines
        }

        if (contains(line, "1-PV, 2-DAYS")) {
            dataLabel.push_back("Cumulative Pore Volume");
            dataLabel.push_back("Time");
            dataLabel.push_back("Total Production");
            dataLabel.push_back("Water/Oil Ratio");
            dataLabel.push_back("Cumulative Oil Recovery");
            dataLabel.push_back("Total Production Rate");
        }
		else if (contains(line, "PHASE CUTS FOR EACH PHASE")) {
            phaseCount = getVarRange(line);
            if (phaseCount<3 || phaseCount>4) {
				vtkErrorMacro(<<"Unexpected phase cuts format");
                return 0;
            }
            dataLabel.push_back("Phase Cut Water");
            dataLabel.push_back("Phase Cut Oil");
            dataLabel.push_back("Phase Cut Micoremultion");
            if (phaseCount > 3) {
                dataLabel.push_back("Phase Cut Gas");
            }
       }
       else if (contains(line, "WELLBORE PRESSURE OF EACH WELLBLOCK")) {
           if ((wellBlockCount = getVarRange(line)) < 1) {
               vtkErrorMacro(<<"Unexpected wellbore pressure format");
               return 0;
           }
           for (int i = 0; i < wellBlockCount; i++) {
              char label[100];
		      sprintf(label, "Wellbore pressure of block #%i", i+1);
              dataLabel.push_back(label);
           }
      }
      else if (contains(line, "WELLBORE TEMPERATURE")) { 
          dataLabel.push_back("Temperature");
      }
	  else if (contains(line, "PHASE AND TOTAL CONC. FOR")) {
          char componentPhase[200];

		  // Skip to "COMPONENT" (may return NULL)
          // The label we need is 13 characters after COMPONENT...
          char * component = strstr((char*)line, "COMPONENT");
          if (component == NULL || strlen(component) < 15 || component[12]!=' ') { 
              vtkErrorMacro(<<"Unexpected component format");
              return 0;
          } 
          component += 13; //assuming format "PHASE AND TOTAL CONC. FOR COMPONENT 1  WATER  "
          removeTrailingChar(component, ' ');

	      // see if we still have a reasonable component name
	      if (strlen(component) >100 || *component=='\0') {
		      return 0;
	      }
          for (int i = 0; i < 4; i++) {
              sprintf(componentPhase,  i == 3 ? "%s total ": "%s phase %d", component, i+1);
              dataLabel.push_back(componentPhase);
          }
          componentCount++;
      }
      else if (contains(line, "LOWER,UPPER, AND EFF. SALINITIES")) {
          dataLabel.push_back("Lower Effective Salinity");
          dataLabel.push_back("Upper Effective Salinity");
          dataLabel.push_back("Effective Salinity");
      }
	  else if (contains(line, "AQUEOUS PHASE CONC. OF BIO SPECIES")) {
		  std::string phaseLabel = "Aqueous phase conc. of bio species";
		  std::stringstream ss;
		  for (int i = 0; i < getVarRange(line); ++i) {
			  ss << phaseLabel << i;
		      dataLabel.push_back(ss.str());
			  ss.clear();
		  }
	  }
	  else if (contains(line, "CAQSP(KK) FOR KK=1,NIAQ")) {
          if ( (niaq = getVarRange(line)) < 1 ) {
              vtkErrorMacro(<<"Unexpected CAQSP format");
              return 0;
          }
          for (int i = 0; i < niaq; i++) {
              char label[100];
		      sprintf(label, "CAQSP#i", i+1);
              dataLabel.push_back(label);
          }
      }
      else if (contains(line, "CAQSP(KK) FOR KK=NIAQ+1,NFLD")) {
          if ( (nfld = niaq + getVarRange(line)) <= niaq ) {
              vtkErrorMacro(<<"Unexpected CAQSP format");
              return 0;
          }
          for (int i = niaq; i < nfld; i++) {
              char label[100];
		      sprintf(label, "CAQSP#%i", i+1);
              dataLabel.push_back(label);
          }
     }
	 else if (contains(line, "PSURF(L)")) {
		 dataLabel.push_back("PSURF#1");
		 dataLabel.push_back("PSURF#2");
		 dataLabel.push_back("PSURF#3");
         dataLabel.push_back("TSURF");
     }
     else if (contains(line, "CSLDT(KK),KK=1,NSLD")) {
        if ( (nsld = getVarRange(line)) < 1) {
            vtkErrorMacro(<<"Unexpected CSLDT format");
            return 0;
        }
        for (int i = 0; i < nsld; i++) {
            char label[100];
		    sprintf(label, "CSLDT#%i", i+1);
            dataLabel.push_back(label);
        }

    }
    else if (contains(line, "LOG(IFTMW), LOG(IFTMO)")) {
        dataLabel.push_back("LOG(IFTMW)");
        dataLabel.push_back("LOG(IFTMO)");
    }
    else if (contains(line, "TOTAL NO. OF VARIABLES")) {
        return parseAsEndVarSection(line);
    }
    else {
      vtkErrorMacro(<<"Unexpected producer variable format");
      return 0;
    }
  }
}



int UTChemWellReader::parseAsInjectorVariable()
{
  while (true){

    const char* line = readNextLine(false);

    if (strlen(line) == 0)
      continue; // skip blank lines

    if (contains(line, "1-PV, 2-DAYS")){

      dataLabel.push_back("Cumulative Pore Volume");
      dataLabel.push_back("Time");
      dataLabel.push_back("Total Injection");
      dataLabel.push_back("Total Injection Rate");

    }else if (contains(line, "WELLBORE PRESSURE FOR EACH WELLBLOCK")){

      if ((wellBlockCount = getVarRange(line)) < 0){
        vtkErrorMacro(<<"Unexpected wellbore pressure format");
        return 0;
      }

      for (int i = 0; i < wellBlockCount; i++){
        char label[100];

		sprintf(label,"Wellbore Pressure of block #%i", i+1);
        dataLabel.push_back(label);
      }

    }else if (contains(line, "PRESSURE DROP FOR EACH WELLBLOCK")){

      for (int i = 0; i < wellBlockCount; i++){
        char label[100];

		sprintf(label,"Pressure Drop of block #%i", i+1);
        dataLabel.push_back(label);
      }

    }else if (contains(line,"TOTAL NO. OF VARIABLES")){

      return parseAsEndVarSection(line);

    }else{

      vtkErrorMacro(<<"Unexpected injector variable format");
      return 0;
    }
  }
}



// overloaded function, verifying that the file read
// contains only valid data, after its read entirely
bool UTChemWellReader::validFileRead()
{
	bool divisible = false;

	if (varCount != 0) {
		if (data.size() % varCount == 0) {
			divisible = true;
		}
	}

  if (divisible) {
    timeStepCount = (int)data.size()/varCount;
    return 1;
  }

  return 0;
}

void UTChemWellReader::buildWell(vtkPolyData* data)
{
  vtkCellArray * lines = vtkCellArray::New();
  vtkPoints * points = vtkPoints::New();
  vtkPolyLine* line = vtkPolyLine::New();
  float** positions = InputInfo->getCellCenters();
  vtkIdType id = 0;

  int fileWellId = atoi(file_ext.substr(4,2).c_str());

  UTChemInputReader::WellData well = InputInfo->wellInfo[fileWellId];

  int numPts = well.ilast - well.ifirst + 1;
  vtkIdList* ptIds = line->GetPointIds();

  // Need to subtract 1 from UTChem indices since they appear to be
  // from [1, nx] ... [1, nz] while ours are [0, nx), etc.
  // From UTChem docs:
  //   "Possible Values: Between 1 and the number of gridblocks in the
  //    pertinent direction, inclusive"
  if(well.idir != 4){
    for(int i = 0 ; i < numPts ; ++i) {
      switch(well.idir) {
        default: // Ignore bad cases
          break;
        case 1: // Parallel to x-axis
          points->InsertNextPoint(positions[0][i], positions[1][well.iw - 1], positions[2][well.jw - 1]);
          ptIds->InsertNextId(id++);
          break;
        case 2: // Parallel to y-axis
          points->InsertNextPoint(positions[0][well.iw - 1], positions[1][i], positions[2][well.jw - 1]);
          ptIds->InsertNextId(id++);
          break;
        case 3: // Parallel to z-axis
          points->InsertNextPoint(positions[0][well.iw - 1], positions[1][well.jw - 1], positions[2][i]);
          ptIds->InsertNextId(id++);
          break;
      }
    }
  } else { // Deviated well
    std::vector<UTChemInputReader::WellData::DeviatedCoords>::iterator itt;
    for(itt = well.deviated.begin() ; itt != well.deviated.end() ; ++itt) {
      points->InsertNextPoint(positions[0][itt->i - 1], positions[1][itt->j - 1], positions[2][itt->k - 1]);
      ptIds->InsertNextId(id++);
    }
  }
    
  lines->InsertNextCell(line);
  line->Delete();

  //assert(lines->GetNumberOfCells() == InputInfo->wellInfo.size());// && points->GetNumberOfPoints() == totalPoints);

  data->SetPoints(points);
  data->SetLines(lines);

  lines->Delete();
  points->Delete();
}
