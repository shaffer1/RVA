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

#pragma warning( once: 4996 )
#define _CRT_SECURE_NO_WARNINGS (1)
#include "UTChemTopReader.h"
#include "RVA_Util.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkRectilinearGrid.h"

#if 0
int UTChemTopReader::canReadFile(const char* filename)
{
  if(filename == NULL)
    return 0;

  int vals[12];
  int ret = 1;
  std::ifstream input(filename);
  int unused = 0;
  std::string str;

  skipLines(input, 35, unused); // Eat lines we don't care about
  readNextLine(input, str, unused);

  // CC SIMULATION FLAGS
  // *---- IMODE IMES IDISPC ICWM ICAP IREACT IBIO ICOORD ITREAC ITC IGAS IENG 
  // ICOORD:
  // 1 - Cartesian coordinate system is used
  // 2 - Radial coordinate system is used
  // 3 - Cartesian coordinate system with variable-width gridblock is used (2-D cross section only)
  // 4 - Curvilinear grid definition of the X-Z cross section is used (2-D or 3-D)
  if(12 != sscanf(str.c_str(), " %d %d %d %d %d %d %d %d %d %d %d %d", &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &vals[6], &vals[7], &vals[8], &vals[9], &vals[10], &vals[11]))
    ret = 0; // Bad formatting

  // ICOORD is value number 7 in the array
  if(vals[7] == 2)
    ret = 0; // Radial not supported

  input.close();
  return 1 & ret;
}
#endif



void UTChemTopReader::skipLines(std::ifstream& InputFile, int numLines, int& lineCount)
{
  std::string tmp;

  if(!InputFile.is_open())
    return;

  for(int i = 0 ; i < numLines ; ++i) {
    try {
      std::getline(InputFile, tmp);
      lineCount++;
    } catch(...) {
      return;
    }
  }
}


void UTChemTopReader::readNextLine(std::ifstream& InputFile, std::string& str, int& lineCount)
{
  if(!InputFile.is_open())
    return;

  try {
    std::getline(InputFile, str);
    lineCount++;
  } catch(...) {
    return; // Exception caught
  }
}


char* UTChemTopReader::consumeProcessed(char* ptr)
{
  return (char*)skipNonWhiteSpace(skipWhiteSpace(ptr));
}

UTChemTopReader::UTChemTopReader(const std::string& input, int nx, int ny)
  : line(1), isValid(false),
  parseResult(UTChemTopReader::NO_FILE)
{
	this->nx = nx;
	this->ny = ny;
  try {
    InputFile.open(input.c_str());
    if(InputFile.is_open()) {
      parseResult = readFile();
    }

  } catch(const std::exception& e) {
    vtkOutputWindowDisplayErrorText(e.what()); // should not happen
    parseResult = UTChemTopReader::NO_FILE;
    isValid = false;
		try { InputFile.close(); } catch(...) {}
		throw e;
  } 



	try { InputFile.close(); } catch(...) {}


}


UTChemTopReader::~UTChemTopReader()
{
	try {
		if(InputFile.is_open())
			InputFile.close();
	} catch(...) {}
}


UTChemTopReader::ParseState UTChemTopReader::readFile()
{
  std::string str;
  UTChemTopReader::ParseState retval = UTChemTopReader::FAIL_HEADER; // See enum in .h

  if((retval = readDVar(str, topdim, nx*ny)) != UTChemTopReader::SUCCESS)
    return retval;

	isValid = (topdim.size() >= nx*ny);

  return UTChemTopReader::SUCCESS;
}


void UTChemTopReader::readNextLine(std::string& str)
{
  UTChemTopReader::readNextLine(InputFile, str, line);
}


void UTChemTopReader::skipLines(int numLines)
{
  UTChemTopReader::skipLines(InputFile, numLines, line);
}


UTChemTopReader::ParseState UTChemTopReader::readDVar(std::string& str, std::vector<double>& container, int numExpected)
{
	skipLines(1);
	bool ignoreTooManyValues = false;
  while(!InputFile.eof()) {
		readNextLine(str);
    int count;
    double val;
    char* ptr = (char*)str.c_str();

    while(ptr != NULL && *ptr != '\0' && *ptr != '\n' && *ptr != '\r') {
      ptr = (char*)skipWhiteSpace(ptr);
      
      if(*ptr == '\0' || *ptr == '\n' || *ptr == '\r')
        break; // Just in case there was trailing whitespace
      
      count = 0;
      val = 0;
      if(2 == sscanf(ptr, "%d*%lg", &count, &val)) {
				ignoreTooManyValues = true; // TOP file definitions are lazy - they may have too many values
        assert(count>0 && val>0);
        //assert(numExpected <0 || count + container.size() <= numExpected);
         
        //if(numExpected >=0  && numExpected < container.size() + count) {
        //  throw std::exception("Too many values read");
        //}
        for(int i = 0 ; i < count ; ++i)
          container.push_back(val);
      } else if(1 == sscanf(ptr, "%lg", &val)) {
        container.push_back(val);
      } else {
        //throw std::exception("Unexpected line format");
		throw std::runtime_error("Unexpected line format");
      }

      // Consume what was just processed
      ptr=consumeProcessed(ptr);
    }
    
  }
  if(numExpected >=0 && container.size() < numExpected) {
    //throw std::exception("Insufficient TOP values read");
	throw std::runtime_error("Insufficient TOP values read");
  }
	if(!ignoreTooManyValues && numExpected >=0 && container.size() != numExpected) {
		//throw std::exception("Too many TOP values read");
		throw std::runtime_error("Too many TOP values read");
	}

  return UTChemTopReader::SUCCESS; // Default
}
