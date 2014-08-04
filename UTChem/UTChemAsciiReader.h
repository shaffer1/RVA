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

#ifndef __UTChemAsciiReader_h
#define __UTChemAsciiReader_h

#include <fstream>
#include <utility>
#include <vector>
#include <string>
#include <map>

#include "vtkAlgorithm.h"
#include "vtkDataObject.h"
#include "vtkFloatArray.h"
#include "vtkDataSet.h"

struct UTChemInputReader;

typedef  std::map<int,vtkFloatArray*> IntegerTovtkFloatArrayMap;
typedef  std::map<int,vtkFloatArray*>::iterator IntegerTovtkFloatArrayMap_it;


class VTK_EXPORT UTChemAsciiReader : public vtkAlgorithm {
public:
  vtkTypeMacro(UTChemAsciiReader, vtkAlgorithm);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual int CanReadFile(const char*)=0;
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  UTChemAsciiReader();
  virtual ~UTChemAsciiReader();

  // Useful functions:
  // According to: http://www.itk.org/Wiki/Writing_ParaView_Readers
  virtual int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int RequestDataObject(vtkInformation*,vtkInformationVector**,vtkInformationVector*);
  virtual int ProcessRequest(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);

  char* FileName;
protected:
  //BTX
  virtual int readFile();

// internal functions for readFile
  virtual int initializeStream();
  virtual void readHeader()=0; // throws exception if invalid nx,ny,nz
  virtual int parseLine(const char*)=0;// Returns 1 if line was eaten, 0 otherwise (failed). May also throw an std:ex if we choke on the line 
  virtual bool validFileRead(); // after file parsing is complete, checks that we have a data structure to display


  virtual  const char*fileExtensionToLabel(std::string&ext);
  static  std::string getInputFileFromFileName(const char* filename);
  virtual int readLayerValues(std::string name = "", bool absolutePhase = true); // reads NX*NY  numerical values
  virtual void setMeaningfulArrayName(vtkFloatArray* array,int thePhase, std::string name = "", bool absolutePhase = true);
  virtual void readNXNYnumericalValuesIntoArray(float*receivingArray);
  virtual unsigned findClosestTimeStep(double& reqTime);
  virtual int buildVTKObject(const unsigned& bestidx, vtkInformation* outInfo);
  virtual int buildImageData(vtkDataSet * dataSet);
  virtual int buildRGridData(vtkDataSet * dataSet);
	virtual int buildSGridData(vtkDataSet * dataSet);

  virtual void freeDataVectors(); // Called by destructor and when parsing fails

  void reloadInputFile(const char*filename);

  const char* readNextLine(bool mustBeNonEmpty);

  // Methods to add well VOI information to the object
  void setWellVOI(vtkDataObject* dataObj);
  void calculateWellVOI(float[6]);

  IntegerTovtkFloatArrayMap* currentTimeStep;
  std::vector<IntegerTovtkFloatArrayMap* > allData;

  std::vector<double> timeList; // must be double, as we pass the bare double[] to Paraview
  std::map<int,std::string> componentNames;
  std::map<std::string,int> reverseMap;

  int nx, ny, nz;
  vtkstd::string file_ext;
  std::ifstream stream;
  int line_num;
  double fileLength;

  int timestep;
  vtkDataObject * dataObj;

  UTChemInputReader * InputInfo;

  // Parsing state:
  std::string nextLine;
  int phase, layer;
  float inj;
  double time;
  float* oneGrid; // Current set of nx*ny*nz points
  char phaseName[100];

  private:
  UTChemAsciiReader(const UTChemAsciiReader&); // Not implemented.
  void operator=(const UTChemAsciiReader&); // Not implemented.

  //ETX
};
#endif
