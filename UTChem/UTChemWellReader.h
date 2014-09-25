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

#ifndef __UTChemWellReader_h
#define __UTChemWellReader_h

#include "vtkImageAlgorithm.h"
#include "UTChemAsciiReader.h"
#include "vtkPolyData.h"

#include <vector>
#include <string>

class VTK_EXPORT UTChemWellReader : public UTChemAsciiReader {
public:
  static UTChemWellReader* New();
  virtual int CanReadFile(const char*);
  vtkTypeMacro(UTChemWellReader, UTChemAsciiReader);
protected:
  UTChemWellReader();
  virtual ~UTChemWellReader();

  bool validFileRead();

  int FillOutputPortInformation(
        int port,
        vtkInformation* info);

  int RequestInformation (
        vtkInformation * request,
        vtkInformationVector** inputVector,
        vtkInformationVector *outputVector);

  int RequestData(
        vtkInformation* vtkNotUsed( request ),
        vtkInformationVector** vtkNotUsed(inputVector),
        vtkInformationVector* outputVector);

private:
  //BTX
  UTChemWellReader(const UTChemWellReader&); // Not implemented.
  void operator=(const UTChemWellReader&); // Not implemented.


  // internal functions for readFile and parseLine
  // Warning - readHeader is called by the constructor
  virtual void readHeader();// just resets our internal vars back to zero...
  virtual int parseLine(const char* c_str);
  virtual void buildWell(vtkPolyData*);

  // ----------------------------------------------

  int parseAsWellHeader(const char* c_str);

  int parseAsWellHistoryData(const char* c_str);
  int parseAsWellVariables(const char* c_str);
  int parseAsWellTable(const char* c_str);

  int parseAsProducerVariable();
  int parseAsInjectorVariable();
  int parseAsEndVarSection(const char* c_str);
  // ----------------------------------------------

  int headerCount; // number of lines in header that we skip
  int phaseCount; // water, oil, microemulsion, gas
  int wellBlockCount; // Spatial Geometry of well
  int componentCount; // component: water, oil, etc
  int timeStepCount; // Number of timesteps read
  int varCount; // number of data points per timestep

  // three column index related flags mentioned in HIST directly but not explained in the UTChem manual
  // We use these to count number of data points.
  int niaq; // see example data at the end of this file
  int nfld; //
  int nsld; // # of solid components

  int wellId;
  int wellType; // same as IFLAG, see notes in the end of the file
  std::string wellName;

  std::vector<std::string> dataLabel; // name of data
  std::vector<float> data; // data, read in sequence of appearance

  //ETX
};

#endif // __UTChemWellReader_h
