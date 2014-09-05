/*=========================================================================

Program:   RVA
Module:    UTChemFluxReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Li, D McWherter

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __UTChemFluxReader_h
#define __UTChemFluxReader_h

#include "vtkImageAlgorithm.h"

#include "UTChemAsciiReader.h"

#include <vector>
#include <map>
#include <utility>
#include <fstream>


class VTK_EXPORT UTChemFluxReader : public UTChemAsciiReader {
public:
  static UTChemFluxReader* New();
  vtkTypeMacro(UTChemFluxReader, UTChemAsciiReader);

  virtual int CanReadFile(const char*);

protected:
  UTChemFluxReader();
  ~UTChemFluxReader();
  
private:
  //BTX
  virtual const char*fileExtensionToLabel(std::string&ext);

// internal functions for readFile and parseLine
  virtual void readHeader(); //  throws exception if invalid nx,ny,nz
  virtual int parseLine(const char* c_str);

  int parseAsLayerIdent(const char* c_str); // 1 for success
  int parseAsFluxHeader(const char* c_str);

  int readFluxDataTable();
  vtkFloatArray* getCurrentFloatArray();

  // parse state
  int directionXYZ; //0,1,2
  int fluxNameCount;
  std::map<std::string,int> fluxNameToIndex;

  //ETX
};

#endif // __UTChemFluxReader_h
