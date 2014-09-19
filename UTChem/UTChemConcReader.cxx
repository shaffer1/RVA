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
#include "UTChemConcReader.h"
#include "UTChemInputReader.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cassert>
#include <limits>
#include <sstream>
#include <stdexcept>
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkStructuredGrid.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkFloatArray.h"
#include "vtkDataObject.h"
#include "vtkCellData.h"
#include "vtkImageData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkRectilinearGrid.h"

#include <RVA_Util.h>




// --------------------------------------------------------
vtkStandardNewMacro(UTChemConcReader);

UTChemConcReader::UTChemConcReader()
{
}

UTChemConcReader::~UTChemConcReader()
{
}

// Supported File Types
// Returns empty string for non-supported types
static const char* fileExtensionToLabel(std::string&ext)
{
  if(ext== "CONCP" || ext== "COMP_AQ" || ext== "COMP_ME" || ext== "COMP_OIL") return "Concentration";
  if(ext== "SATP") return "Saturation";
  if(ext== "PRESP") return "Pressure";
  if(ext== "VISC") return "Viscosity";
  if(ext== "TEMPP") return "Temperature";
  if(ext== "PERM") return "Permeability";
  if(ext== "ALKP") return "Alkaline";
  return "";
}

// Standard VTK file reader support
int UTChemConcReader::CanReadFile(const char* filename)
{
  if (!filename) { 
	  return 0;
  }

  std::string ext = getFileExtension(filename);

  if (strlen(fileExtensionToLabel(ext)) == 0) 
  { 
	return 0;
  }
  try {
	  // MVM: again, why reloading?
	  reloadInputFile(filename);
    return InputInfo->canReadFile();
  } catch (std::exception& e) {
    return 0;
  }
}

/* throws an exception if nx,ny,nz if valid dimensions could not be extracted from header */
void UTChemConcReader::readHeader()
{
  // get the dimensions and ignore other lines
  //  NX =           20  NY =           17  NZ =            6
  //RUN NAME IS :EX51
  //RUN # EX51 (BASED ON HILLAFB  SEAR TEST)
  //PHASE II SURF. INJ
  //USING UTCHEM 9.0
  std::string oneLine;
  for(int i = 0; i < 5 ; ++i) { // Naive approach - should be on 5th line
    std::getline(stream, oneLine);
    line_num++;
  }
  nx = ny = nz = -1;
  int read = sscanf(oneLine.c_str(), "  NX =           %d  NY =           %d  NZ =            %d", &nx, &ny, &nz);
  if(nx <= 0 || ny <= 0 || nz <= 0 || read != 3) {
    vtkErrorMacro(<<"Failed to read NX,NY,NZ data from line:'"<<oneLine<<"'")
	throw std::runtime_error("Failed to read nx,ny,nz dimensions at head of the data file");
  }
  if(nx != InputInfo->nx || ny != InputInfo->ny || nz != InputInfo->nz) {
	throw std::runtime_error("INPUT file inconsistent with nx,ny,nz dimensions in data file");
  }

}

/* Returns 1 if line was eaten, 0 otherwise. Does not but could throw an std:ex if we choke on the line */
int UTChemConcReader::parseAsATIMEline(const char* c_str)
{
  if( 2 != sscanf(c_str, "TIME = %lg DAYS PORE VOLUMES INJ. =  %f", &time, &inj))
    return 0; // Failed
  timestep++;
  this->currentTimeStep = new IntegerTovtkFloatArrayMap();
  allData.push_back(this->currentTimeStep);
  timeList.push_back(time);

  double filePosition = (double)stream.tellg();
  if(filePosition>0 && fileLength>0)
    this->UpdateProgress( filePosition / fileLength);

  return 1;
}
/* Returns 1 if line was eaten, 0 otherwise. Does not but could throw an std:ex if we choke on the line */
int UTChemConcReader::parseAsASATPHASEline(const char* c_str)
{
  if(2 != sscanf(c_str, "SAT. OF PHASE            %d  IN LAYER            %d", &phase, &layer))
    return 0;
  return readLayerValues(); // OK
}
/* Returns 1 if line was eaten, 0 otherwise. Does not but could throw an std:ex if we choke on the line */
int UTChemConcReader::parseAsAStandardPropertyline(const char* c_str)
{
  //PRESSURE (PSI) OF PHASE            1  IN LAYER            1
  //PRESSURE (KPA) OF PHASE            1  IN LAYER            1
  //VISCOSITY (MPA.S) OF PHASE            1  IN LAYER            1
  //VISCOSITY (CP) OF PHASE            1  IN LAYER            1
  char measure[100];
  char unit[100];
  if(4 !=  sscanf(c_str, "%99s %99s OF PHASE            %d  IN LAYER            %d",measure,unit, &phase, &layer))
    return 0;
  measure[sizeof(measure)-1] = unit[sizeof(unit)-1]='\0'; // paranoia for too large strings
  if(strlen(unit) <2 || unit[0] != '(' || unit[strlen(unit)-1] != ')') {
    vtkErrorMacro(<<"Expected to find (unit) but got '"<<unit<<"'")
  }
  return readLayerValues();
}

/* Returns 1 if line was eaten, 0 otherwise. Does not but could throw an std:ex if we choke on the line */
int UTChemConcReader::parseAsCOMPPHASEline(const char* c_str)
{
  char compName[10];
  char* str = compName+sizeof(compName)-2;
  memset(compName, 0, sizeof(compName));
  // CONC. OF COMP. NO.  1:WATER    IN OLEIC PHASE IN LAYER   1
  // CONC. OF COMP. NO.  1:WATER    IN AQUEOUS PHASE IN LAYER   1
  // CONC. OF COMP. NO.  1:WATER    IN MICROEMULSION PHASE IN LAYER   1
  if(4 != sscanf(c_str, "CONC. OF COMP. NO.  %d:%9c IN %99s PHASE IN LAYER   %d", &phase, compName, phaseName, &layer))
    return 0;
  phaseName[sizeof(phaseName)-1] = '\0'; // Reissuing paranoia from earlier
  while(compName != str && *(str-1) == ' ') str--;
  *str = '\0';
  if(componentNames.count(phase) == 0)
    componentNames[phase] = compName;
  return readLayerValues();
}


/* Returns 1 if line was eaten, 0 otherwise. Does not but could throw an std:ex if we choke on the line */
int UTChemConcReader::parseAsACONCENTRATIONline(const char* c_str)
{
  //TOTAL FLUID CONC. OF COMP. NO.  1:WATER    IN LAYER   1
  // note Components can have spaces in their names!
  if (1 != sscanf(c_str,"TOTAL FLUID CONC. OF COMP. NO.  %d",&phase)) {
    return 0;
  }

  const char* inLayer_str= strstr(c_str,"IN LAYER");
  
  if (!inLayer_str) {
	  return 0;
  }

  if (1 != sscanf(inLayer_str,"IN LAYER %d",&layer)) {
    return 0;
  }
  
  if (componentNames.count(phase)  == 0) {

    //move inLayer_str until we hit the last letter of the name.
    for (inLayer_str--; *inLayer_str==' '; inLayer_str--)
		;
    
	// move c_str forwards until we hit the colon just prior to the start of the name
    while (*c_str && *c_str != ':') {
		c_str ++;
	}

    // Name is now after : to *inLayer_str
    std::string name;
    
	do {
      name += (* (++c_str));
    } while (c_str < inLayer_str);

    vtkDebugMacro(<<"Extracted name:'"<< name<<"' for component "<<phase)
    componentNames[phase] = name;
  }

  return readLayerValues();
}

// Nearly the same as the standard concentration line except components may have collisions
// this method uses the reverse map to keep track of these collisions
int UTChemConcReader::parseAsPCONCENTRATIONline(const char* match, const char* c_str)
{
  char type[20];

  //FLUID CONCENTRATION OF  1: HYDROGEN ION                    IN LAYER   1
  //ADSORBED CONCENTRATION OF  2: SORBED SODIUM ION               IN LAYER   2
  //SOLID CONCENTRATION OF  3: CALCIUM HYDROXIDE (SOLID)       IN LAYER   1
  // note: Components can have spaces in their names!
  if(2 != sscanf(c_str,match,&type,&phase))
    return 0;

  const char* inLayer_str= strstr(c_str,"IN LAYER");
  if(!inLayer_str) return 0;
  if(1 != sscanf(inLayer_str,"IN LAYER %d",&layer))
    return 0;

  // Get name
  //move inLayer_str until we hit the last letter of the name.
  for(inLayer_str--; *inLayer_str==' '; inLayer_str--);
  // move c_str forwards until we hit the colon just prior to the start of the name
  while(*c_str && *c_str != ':') c_str ++;

  // Name is now after : to *inLayer_str
  std::string name;
  do {
    name +=(* (++c_str));
  } while(c_str < inLayer_str);

  name += " (";
  name += type;
  name += ")";

  // Since there are possible collisions, we need to use the reverse map
  if(reverseMap.count(name)  == 0) {
    vtkDebugMacro(<<"Extracted name:'"<< name<<"' for component "<<phase)
    reverseMap[name] = reverseMap.size();
  }

  phase = reverseMap[name];
  if(componentNames.count(phase) == 0)
    componentNames[phase] = name;

  return readLayerValues(NULL, false);//(char*)name.c_str());
}

int UTChemConcReader::parseAsTEMPERATUREline(const char* c_str)
{
  // TEMPERATURE (F) IN LAYER            1
  if(1 != sscanf(c_str,"TEMPERATURE (%*c) IN LAYER  %d",&layer))
    return 0;
  phase = 0;
  // MVM: gives warning because of deprecated conversion from string constant to char *
  return readLayerValues("Temperature");
}

int UTChemConcReader::parseAsPERMEABILITYline(const char* c_str)
{
  // Simulate time to get a valid array
  if(this->currentTimeStep == NULL)
    parseAsATIMEline("TIME = 0.0 DAYS PORE VOLUMES INJ. =  0.0"); // Hack to read in timeless data
  // X-PERMEABILITY (MD) IN LAYER            1
  if(1 != sscanf(c_str,"X-PERMEABILITY (%*c%*c) IN LAYER  %d",&layer))
    return 0;
  phase = 0;
  //MVM: deprecated conversion from string constant to char *
  return readLayerValues("Permeability");
}

int UTChemConcReader::parseAsPOROSITYline(const char* c_str)
{
  // Simulate time to get a valid array
  if(this->currentTimeStep == NULL)
    parseAsATIMEline("TIME = 0.0 DAYS PORE VOLUMES INJ. =  0.0"); // Hack to read in timeless data
  // POROSITY IN LAYER            1
  if(1 != sscanf(c_str,"POROSITY IN LAYER  %d",&layer))
    return 0;
  phase = 1;
  //MVM: deprecated conversion from string constant to char *
  return readLayerValues("Porosity");
}

int UTChemConcReader::parseAsSURFACTANTline(const char* c_str)
{
  char name[60] = "Surfactant_";
  char comp[30];
  //TOTAL SURF.(SOAP+INJ) CONC. IN LAYER            1
  //TOTAL SURF.(GEN+INJ) CONC. IN LAYER            1
  //MVM: &comp is char*[30] not char *
  if(2 != sscanf(c_str, "TOTAL SURF.%29s CONC. IN LAYER %d", comp, &layer))
    return 0;
  strncat(name, comp, sizeof(name) - strlen(name));
  if(reverseMap.count(name) == 0)
    reverseMap[name] = reverseMap.size();
  phase = reverseMap[name];
  return readLayerValues(name);
}

// Reads a UTChem data file (.CONC / .VISC etc )
int UTChemConcReader::parseLine(const char* c_str) {
    if( startsWith(c_str,"TIME ") )
        return parseAsATIMEline(c_str);
    else if(startsWith(c_str, "SAT. OF PHASE"))
        return parseAsASATPHASEline(c_str) ;
    else if(startsWith(c_str, "PRESSURE") ||startsWith(c_str, "VISCOSITY" ) )
        return parseAsAStandardPropertyline(c_str);
    else if(startsWith(c_str, "TOTAL FLUID CONC. OF COMP. NO."))
        return parseAsACONCENTRATIONline(c_str);
    else if(startsWith(c_str, "CONC. OF COMP. NO."))
        return parseAsCOMPPHASEline(c_str);
    else if(startsWith(c_str, "TEMPERATURE"))
        return parseAsTEMPERATUREline(c_str);
    else if(startsWith(c_str, "X-PERMEABILITY"))
        return parseAsPERMEABILITYline(c_str);
    else if(startsWith(c_str, "POROSITY"))
        return parseAsPOROSITYline(c_str);
    else if(startsWith(c_str, "FLUID CONCENTRATION OF") || startsWith(c_str, "ADSORBED CONCENTRATION OF") || startsWith(c_str, "SOLID CONCENTRATION OF"))
        return parseAsPCONCENTRATIONline("%19s CONCENTRATION OF  %d", c_str);
    else if(startsWith(c_str, "TOTAL SURF."))
        return parseAsSURFACTANTline(c_str);
   
   	return 0; //could not consume this line
}
