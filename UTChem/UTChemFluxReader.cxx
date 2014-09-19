/*=========================================================================

Program:   RVA
Module:    UTChemFluxReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Duggirala, D McWherter, U Yadav

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "UTChemFluxReader.h"
#include "UTChemInputReader.h"
#include <cstdlib>
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
vtkStandardNewMacro(UTChemFluxReader);

UTChemFluxReader::UTChemFluxReader() : fluxNameCount(0),directionXYZ(0)
{
}

UTChemFluxReader::~UTChemFluxReader()
{
}

int UTChemFluxReader::CanReadFile(const char* filename)
{
	// MVM: trying to imagine a scenario where this would 
	// ever be called 1) without a filename and 2) for anything
	// but PROF files.
	if (!filename) 
	{
	  return 0;
	}
  
	std::string ext = getFileExtension(filename);
	if (ext != "PROF") 
	{
		return 0;
	}
 
   	reloadInputFile(filename);
	if (InputInfo->ivel) {
		return InputInfo->canReadFile();
	}
	else {
		vtkOutputWindowDisplayErrorText("No phase vel. in .PROF file, IVEL equals 0.");
		return 0;
	}
}

const char* UTChemFluxReader::fileExtensionToLabel(std::string&ext) {
	if(ext == "PROF") return "Flux";
	return "";
}

/* throws an exception if nx,ny,nz if valid dimensions could not be extracted from header */
void UTChemFluxReader::readHeader()
{
	// get the dimensions 
	nx=InputInfo->nx;
	ny=InputInfo->ny;
	nz=InputInfo->nz; 
	// reset our parsing properties here:
    directionXYZ=-1; //0,1,2
    fluxNameCount=0;
    fluxNameToIndex.clear();
}

// Reads a UTChem data file (.CONC / .VISC etc )
int UTChemFluxReader::parseLine(const char* c_str) {

	if(contains(c_str,"D E T A I L S   O F   L A Y E R   N U M B E R"))
		return parseAsLayerIdent(c_str); // 1 for success

	if(contains(c_str,"PHASE X-FLUX") 
		|| contains(c_str,"PHASE Y-FLUX") 
		|| contains(c_str,"PHASE Z-FLUX"))
		return parseAsFluxHeader(c_str);

	return 1; // Just ignore lines that we don't understand
}


int UTChemFluxReader::parseAsLayerIdent(const char* c_str){ // 1 for success
	int layerIndex =0;
	
	// MVM: yikes
	int parsedLayerIndex= 1==sscanf(c_str,"D E T A I L S   O F   L A Y E R   N U M B E R     %d",&layerIndex);
	
	if(!parsedLayerIndex || layerIndex<1 || layerIndex > nz) {
		throw std::runtime_error("Failed to read layer number");
	}
	this->layer = layerIndex - 1;
	return 1; // consumed
}
int UTChemFluxReader::parseAsFluxHeader(const char* c_str){
	char fluxUnits[256];
	char directionChar;

	if( 3!=sscanf(c_str,"%99s PHASE %c-FLUX  (%255s",this->phaseName,&directionChar,fluxUnits)) {
		vtkErrorMacro(<<"Could not parse PHASE FLUX :'"<<nextLine<<"'")
			throw std::runtime_error("Failed to read phase flux heading");

	}
	this->phaseName[sizeof(phaseName)-1]=0; // Paranoia
	fluxUnits[sizeof(fluxUnits)-1]=0;
	
	removeTrailingChar(fluxUnits,')');

	// MVM: is this really doing char arithmetic?!
	this->directionXYZ = directionChar-'X';
	if(!*phaseName || !*fluxUnits || directionXYZ<0 || directionXYZ>2) {
		vtkErrorMacro(<<"Unexpected PHASE FLUX values :'"<<nextLine<<"'")
		throw std::runtime_error("Unexpected values found in phase flux heading");
	}
	std::string phaseNameString(phaseName);
	if( ! this->fluxNameToIndex.count(phaseNameString) ){
		++this->fluxNameCount;
		fluxNameToIndex[phaseNameString] = this->fluxNameCount;	
	}
	phase = fluxNameToIndex[phaseNameString];

	readFluxDataTable();
	return 1;
}

int UTChemFluxReader::readFluxDataTable() { // 1 for success
	std::stringstream ss;
	
	vtkFloatArray* array = getCurrentFloatArray();

	int numColsParsedEarlier = 0;

	// What about reading this number of values directly?
	size_t remainNumValues = nx * ny;
	while (numColsParsedEarlier < nx) {

		std::vector<int> indices;
		const char* c_str = readNextLine(true); 
		ss.str(c_str);
		std::string str;
		ss >> str;
		if (str != "J/I=") {
			std::cerr << "Not a table header row: " << str << std::endl;
		}
		int column;
		while (ss >> column) {
			indices.push_back(column);
		}
		ss.clear();
		
	    // MVM: naming - why would one expect 0 columns?	
		int numColsExpected = 0;

		// MVM: read ny rows of actual data.
		for (int j = 0; j < ny; j++) {

			// MVM: read first line of data
			c_str = readNextLine(true);
			ss.str(c_str);
		
		    // MVM: first col contains the J index	
			int jthIndex;
			ss >> jthIndex;

			if (jthIndex < 1 || jthIndex > ny || jthIndex != j+1) {
				throw std::runtime_error("Invalid j index");
			}

			vtkIdType idx = this->layer*nx*ny + j*nx + numColsParsedEarlier;

			// sanity check: paranoia because SetComponent performs no bounds checking
			if ( !array || idx < 0 
				|| (long)idx + (nx-numColsParsedEarlier) > (long)array->GetNumberOfTuples() 
				|| directionXYZ < 0 || directionXYZ >= (int)array->GetNumberOfComponents())
			{
				throw std::runtime_error("Invalid internal state");
			}
		
			std::vector<int>::const_iterator it;
			int columnCount = 0;
			float f;
			for (it = indices.begin(); it != indices.end(); ++it) {
				ss >> f;
				array->SetComponent(idx, directionXYZ, f);
				columnCount++;
				remainNumValues--;
				idx++;
			}
			ss.clear();

			// sanity check that all remaining rows also have a reasonable column count - 
			if (j == 0) {
				numColsExpected = columnCount;
				if (columnCount < 1) {
					throw std::runtime_error("Expected at least one data column");
				}
			}
			else if (columnCount != numColsExpected) {
				throw std::runtime_error("Incorrect number of columns in J/I data");
			}
		} // for all rows of data
		indices.clear();
		numColsParsedEarlier += numColsExpected;
	} // while
	if (remainNumValues != 0) {
		throw std::runtime_error("Incorrect number of values read");
	}
	return 1; // Success
}

vtkFloatArray* UTChemFluxReader::getCurrentFloatArray() {
	assert(nx>0 && ny >0 && nz>0);
	assert(layer>=0 && layer<nz);
// PROF files do not include a timestep info for the flux data!
// So we assume that if we see the same phasename/flux/layer that it's the next time step
// Assume files always write X,Y,Z order
  bool startNextTimeStep = phase==1 && layer ==0 && directionXYZ ==0;  
  if(startNextTimeStep) {
	  time = timestep++;
	  this->currentTimeStep = new IntegerTovtkFloatArrayMap();
	  allData.push_back(this->currentTimeStep);
	  timeList.push_back(time);
  }

  unsigned arraySize = nx*ny*nz;

  if(currentTimeStep == NULL || arraySize==0) {
	throw std::runtime_error("Can't readLayerValues when there's no current time step (or proper dimensions)");
  }
  if(phase<0) {
	throw std::runtime_error("Negative phase values are unsupported");
  }

  if( ! currentTimeStep->count(phase)) {
    vtkFloatArray* floatArray = vtkFloatArray::New();
	floatArray->SetNumberOfComponents(3);
    floatArray->SetNumberOfTuples(arraySize);
	for(int xyz=floatArray->GetNumberOfComponents() -1; xyz>=0;xyz--)
	    floatArray->FillComponent(xyz,std::numeric_limits<float>::quiet_NaN() );
    setMeaningfulArrayName(floatArray, phase);
    (*currentTimeStep)[phase]=floatArray;
  }
  vtkFloatArray* result  = (*currentTimeStep)[phase];
  oneGrid = result->GetPointer(0);
  assert(oneGrid);
  return result;
}
