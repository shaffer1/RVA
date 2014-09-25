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


/* HIST file notes:

For producer wells:

Always on: (appeared in every HIST file)

	1-PV

		- cumulative pore volume

	2-DAYS

		- time in days

	3-TOTAL PROD. IN [FT3, M3, STB]

		- cumulative production [ft^3, m^3, STB]

	4-W/O RATIO

		- water oil ratio

	5- CUM. OIL REC.

		- cumulative oil recovery

	6-TOTAL PROD. RATE IN [FT3/D, M3/D, STB/D]

		- total production rate [ft^3/day, m^3/day, STB/day]

	10- 12 : WELLBORE PRESSURE OF EACH WELLBLOCK,[PSI, KPA]

	10- 10 : WELLBORE PRESSURE OF EACH WELLBLOCK,[PSI, KPA]

		- wellbore pressure for each well block, variable numbering

	11- 14 : PHASE AND TOTAL CONC. FOR COMPONENT 1  Water

  15- 18 : PHASE AND TOTAL CONC. FOR COMPONENT 2  Oil

  19- 22 : PHASE AND TOTAL CONC. FOR COMPONENT 3  Surf.

  23- 26 : PHASE AND TOTAL CONC. FOR COMPONENT 4  Polymer

  27- 30 : PHASE AND TOTAL CONC. FOR COMPONENT 5  Chloride

  31- 34 : PHASE AND TOTAL CONC. FOR COMPONENT 6  Calcium

  35- 38 : PHASE AND TOTAL CONC. FOR COMPONENT 9  MG

  39- 42 : PHASE AND TOTAL CONC. FOR COMPONENT10  CO3

  43- 46 : PHASE AND TOTAL CONC. FOR COMPONENT11  Na

  47- 50 : PHASE AND TOTAL CONC. FOR COMPONENT12  Hydr.

  51- 54 : PHASE AND TOTAL CONC. FOR COMPONENT13  Acid

		- phase and total concentration for component N, if ICF(I) = 1,

		  variable numbering, variable range and variable components,

		  total concentration is always present regardless of number

		  of phases, therefore if ICF(I) = 1, the there will be at least

		  2 lines dedicated to component I


On if IGAS = 0: (appeared in every HIST file so far)

	7- 9 : PHASE CUTS FOR EACH PHASE

		- water cut, oil cut, microemulsion cut


On if IGAS = 1: (no examples, pure speculation)

	7- 10 : PHASE CUTS FOR EACH PHASE

		- water cut, oil cut, microemulsion cut, gas cut,

		  format may vary, have not seen any sample


On if IENG = 1:

	13- 14 : WELLBORE TEMPERATURE (DEGREE [F, C])

	22 : WELLBORE TEMPERATURE (DEGREE [F, C])

		- well bore temperature for each well, variable numbering


On if IREACT > 1, or ICF(3) = 1:

	51- 53 : LOWER,UPPER, AND EFF. SALINITIES

		- lower effective salinity, upper effective salinity, effective salinity


On if IREACT > 1:

	54- 59 : CAQSP(KK) FOR KK=1,NIAQ

		- independent species concentration, number of lines is NIAQ ??location??

		  unit is mole/liter of water variable numbering


	if IRSPS > 0:

		60- 67 : CAQSP(KK) FOR KK=NIAQ+1,NFLD

			- dependent species concentration, number of lines is NFLD-NIAQ ??location??

			  unit is mole/liter of water, variable numbering


	if IREACT = 3:

		68- 71 : PSURF(L),L=1,3,TSURF

			- phase concentration of injected + generated surfactant

			  total concentration of injected + generated surfactant

			  variable numbering


	if NSLD > 0:

		82- 85 : CSLDT(KK),KK=1,NSLD

			- concentration of solid components, unit is mole/liter of water

			  number of lines is NSLD ??location??


	if ICNM > 0:

		72- 73 : LOG(IFTMW), LOG(IFTMO)

			- log_10 of interfacial tension between water/micromulsion and oil/micromulsion

			  XIFT1, XIFT2 with unit dyne/cm in documentation, inconsistent with example

			  variable numbering



For injector wells:

Always on: (appeared in every HIST file)

	1-PV

		- cumulative pore volume

	2-DAYS

		- time in days

	3-TOTAL INJ IN [FT3, M3, STB]

		- cumulative injection [ft^3, m^3, STB]

	4- TOTAL INJ. RATE IN [FT3/D, M3/D, STB/D]

		- total injection rate [ft^3/day, m^3/day, STB/day]

	5- 16 : WELLBORE PRESSURE FOR EACH WELLBLOCK, [PSI, KPA]

	5-  5 : WELLBORE PRESSURE FOR EACH WELLBLOCK, [PSI, KPA]

		- wellbore pressure for each well block, variable range, always start with 5


On when there is only one producer and injecter, or when NG > 0, NY = 1, NZ = 1

	17- 28 : PRESSURE DROP FOR EACH WELLBLOCK, [PSI, KPA]

	6-  6 : PRESSURE DROP FOR EACH WELLBLOCK, [PSI, KPA]

		- pressure drop between the wells when there is only one producer and injecter

		  or pressure drop between the pressure tabs, when NG > 0, NY = 1, NZ = 1



IFLAG:

	1 - injection well rate constrained

	2 - production well pressure constrained (only when ICOORD = 1 or 3)

	3 - injection well pressure constrained (only when ICOORD = 1 or 3)

	4 - production well rate constrained
*/
