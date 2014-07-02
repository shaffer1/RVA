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
  vtkTypeRevisionMacro(UTChemFluxReader, UTChemAsciiReader);
  //void PrintSelf(ostream& os, vtkIndent indent);

  virtual int CanReadFile(const char*);
  //virtual int FillOutputPortInformation(int port, vtkInformation* info);

protected:
  UTChemFluxReader();
  ~UTChemFluxReader();

  // Useful functions:
  //  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  //  int ProcessRequest (public member)
  //  int RequestInformation - will be important for running in parallel (large data sets will do this?)
  //  int CanReadFile(const char*)
  // According to: http://www.itk.org/Wiki/Writing_ParaView_Readers
 /* virtual int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int RequestDataObject(vtkInformation*,vtkInformationVector**,vtkInformationVector*);
  virtual int ProcessRequest(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);*/

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
