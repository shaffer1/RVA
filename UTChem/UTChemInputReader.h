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

#ifndef __UTChemInputReader_h
#define __UTChemInputReader_h

#include <fstream>
#include <utility>
#include <vector>
#include <string>
#include <map>

#include "vtkObject.h"

class vtkDataObject;
class vtkInformation;
class vtkDoubleArray;
class vtkPoints;
class vtkFloatArray;
class UTChemTopReader;

struct UTChemInputReader
{
	typedef enum {NO_FILE=0,FAIL_HEADER=1,FAIL_OUTPUTOPTS =2, FAIL_WELLINFO =3, FAIL_RESERVOIRPROPERTIES =4, SUCCESS=256} ParseState;

	UTChemInputReader::ParseState parseResult;
	UTChemInputReader(const std::string& input);
	~UTChemInputReader();

	// Well data information
	struct WellData
	{
	struct DeviatedCoords { int i, j, k; };
	int idw, iw, jw, iflag;
	float rw, swell;
	int idir, ifirst, ilast, iprf;
	std::vector<DeviatedCoords> deviated; // Holds values for deviated wells
	};

	// Giant list of public variables for use later
	int imode, imes, idispc, icwm, icap, ireact, ibio, icoord, itreac, itc, igas, ieng;
	int nx, ny, nz, idxyz, iunit;
	float r1, dx1, dy1, dz1;
	int N, no, ntw, nta, ngc, ng, noth;
	int icumtm, istop, ioutgms, is3g;
	int ippres, ipsat, ipctot, ipbio, ipcap, ipgel, ipalk, iptemp, ipobs;
	int ickl, icvis, iper, icnm, icse, ihystp, ifoamp, inoneq;
	int iads, ivel, irkf, iphse;
	int tmax;
	double compr, pstand;
	int ipor1, ipermx, ipermy, ipermz, imod, itranz, intg;
	int idepth, ipress, iswi, icwi;
	std::vector<std::string> species;
	std::vector<int> icf, iprflag;
	std::vector<double> xspace, yspace, zspace;
	vtkDoubleArray * xdim, * ydim, * zdim;
	vtkFloatArray * cellVolume;
	vtkPoints * points;
	std::map<int, WellData> wellInfo;

	vtkDataObject * getObject(vtkInformation*);
	int getObjectType() const;

	int canReadFile();
	float** getCellCenters();
	void getCellCenter(int i, int j, int k, float * out);

	UTChemTopReader * top;

private:
	ParseState readFile();

	// Keep things clean
	ParseState readResvDesc(std::string& str);
	ParseState readOutputOpts(std::string& str);
	ParseState readReservoirProperties(std::string& str);
	ParseState readWellInformation(std::string& str);

	// Helper functions
	void skipLines(int numLines);
	void readNextLine(std::string& str);
	int readIVarInLine(int numVars, const char* str, std::vector<int>& container);
	void readRegionalCoords(std::string& str, std::vector<double>& container, int numExpected=-1);
	void readCurvilinearXZ(std::string&, std::vector<double>&, std::vector<double>&);

	static char* consumeProcessed(char* ptr);

	// Extra setup functions
	void determineGridType();
	void setupRGridCoords();
	void setupSGridCoords();
	void calculateCellVolume();

	std::ifstream InputFile;
	int readParams;
	int objectType; // 0 = Image Data, 1 = Rectilinear Grid
	bool isValid;
	vtkDataObject * gridObject;
	float ** cellCenters;
};

#endif /* __UTChemInputReader_h */
