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

#ifndef __UTChemConcReader_h
#define __UTChemConcReader_h

#include "vtkImageAlgorithm.h"

#include "UTChemAsciiReader.h"

#include <vector>
#include <map>
#include <utility>
#include <fstream>


class VTK_EXPORT UTChemConcReader : public UTChemAsciiReader {
public:
  static UTChemConcReader* New();
  vtkTypeRevisionMacro(UTChemConcReader, UTChemAsciiReader);

  virtual int CanReadFile(const char*);

protected:
  UTChemConcReader();
  ~UTChemConcReader();
  
private:
  //BTX

// internal functions for readFile
  void readHeader(); // throws exception if invalid nx,ny,nz
  int parseLine(const char* c_str);

  int parseAsATIMEline(const char* c_str); // 1 for success
  int parseAsAStandardPropertyline(const char* c_str);
  int parseAsACONCENTRATIONline(const char* c_str);
  int parseAsASATPHASEline(const char* c_str);
  int parseAsCOMPPHASEline(const char* c_str);
  int parseAsTEMPERATUREline(const char* c_str);
  int parseAsPERMEABILITYline(const char* c_str);
  int parseAsPOROSITYline(const char* c_str);
  int parseAsSURFACTANTline(const char* c_str);
  int parseAsPCONCENTRATIONline(const char* match, const char* c_str);
  //ETX
};

#endif // __UTChemConcReader_h
