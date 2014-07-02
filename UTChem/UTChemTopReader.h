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

#ifndef __UTChemTopReader_h
#define __UTChemTopReader_h

#include <fstream>
#include <utility>
#include <vector>
#include <string>
#include <map>

#include "vtkObject.h"

class vtkDataObject;
class vtkInformation;
class vtkDoubleArray;

class UTChemTopReader
{
public:
  typedef enum {NO_FILE=0,FAIL_HEADER=1,FAIL_OUTPUTOPTS =2, FAIL_WELLINFO =3,SUCCESS=256} ParseState;

  UTChemTopReader::ParseState parseResult;
  UTChemTopReader(const std::string& input, int nx, int ny);
  ~UTChemTopReader();


  // Giant list of public variables for use later
  int nx, ny;
  std::vector<double> topdim;
  bool isValid;


private:
  ParseState readFile();

  // Helper functions
  void skipLines(int numLines);
  void readNextLine(std::string& str);
  UTChemTopReader::ParseState readDVar(std::string& str, std::vector<double>& container, int numExpected=-1);

  // Static helpers
  static void skipLines(std::ifstream& InputFile, int numLines, int& lineCount);
  static void readNextLine(std::ifstream& InputFile, std::string& line, int& lineCount);
  static char* consumeProcessed(char* ptr);

  std::ifstream InputFile;
  int line;
};

#endif /* __UTChemTopReader_h */
