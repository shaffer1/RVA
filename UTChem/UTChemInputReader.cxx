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
#include "UTChemInputReader.h"
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
#include "vtkStructuredGrid.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"

#if 0
int UTChemInputReader::canReadFile(const char* filename)
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

UTChemInputReader::UTChemInputReader(const std::string& input)
  : readParams(0), gridObject(NULL), objectType(0), line(1), xdim(NULL), ydim(NULL), zdim(NULL),
  parseResult(UTChemInputReader::NO_FILE), points(NULL), top(NULL), cellVolume(NULL),
   N(0), dx1(1), dy1(1), dz1(1), iads(0), ibio(0), icap(0), ickl(0), icnm(0), icoord(0), 
	 icse(0), icumtm(0), icvis(0), icwm(0), idispc(0), idxyz(0), ieng(0), ifoamp(0), igas(0), 
	 ihystp(0), imes(0), imode(0), inoneq(0), ioutgms(0), ipalk(0), ipbio(0), ipcap(0), 
	 ipctot(0), iper(0), ipgel(0), iphse(0), ipobs(0), ippres(0), ipsat(0), iptemp(0), 
	 ireact(0), irkf(0), is3g(0), istop(0), itc(0), itreac(0), iunit(0), ivel(0), ng(0), 
	 ngc(0), no(0), noth(0), nta(0), ntw(0), nx(0), ny(0), nz(0), cellCenters(NULL),
	 tmax(0), compr(0), pstand(0), ipor1(0), ipermx(0), ipermy(0), ipermz(0), imod(0), 
	 itranz(0), intg(0), idepth(0), ipress(0), iswi(0), icwi(0)
{
  try {
    InputFile.open(input.c_str());
    if(InputFile.is_open()) {
      parseResult = readFile();

			if (idepth==4)
			{
				size_t found;
				std::string key ("INPUT");
				std::string input_copy (input);
				found = input_copy.rfind(key);
				input_copy.replace(found, key.length(), "TOP");
				top = new UTChemTopReader(input_copy.c_str(), nx, ny);
				setupSGridPoints();
			}
    }
    switch(parseResult) {
      case UTChemInputReader::FAIL_HEADER :
           vtkOutputWindowDisplayErrorText("Could not read INPUT file (perhaps invalid format?)\nFailed reading Reservoir Description");
           isValid = false;
    break;
      case UTChemInputReader::FAIL_OUTPUTOPTS :
           vtkOutputWindowDisplayErrorText("Could not read INPUT file (perhaps invalid format?)\nFailed reading Output Options");
           isValid = false;
     break;
      case UTChemInputReader::FAIL_WELLINFO :
        vtkOutputWindowDisplayErrorText("Could not read INPUT file (perhaps invalid format?)\nFailed reading well information");
        isValid = false;
     break;
      default:
        isValid = true;
     break;
    }
  } catch(const std::exception& e) {
    vtkOutputWindowDisplayErrorText(e.what()); // should not happen
    parseResult = UTChemInputReader::NO_FILE;
    isValid = false;
  } 
	if (parseResult != UTChemInputReader::NO_FILE)
	{
		determineGridType();
		calculateCellVolume();
	}
  try { InputFile.close(); } catch(...) {}
}

UTChemInputReader::~UTChemInputReader()
{
  if(InputFile.is_open())
    InputFile.close();
  if(gridObject != NULL)
    gridObject->Delete();
  if(xdim != NULL)
    xdim->Delete();
  if(ydim != NULL)
    ydim->Delete();
  if(zdim != NULL)
    zdim->Delete();
  if(cellCenters != NULL) {
    for(int i = 0 ; i < 3 ; ++i) // 3 = number of components (x,y,z)
      delete [] cellCenters[i];
    delete [] cellCenters;
  }
	if(points!=NULL)
		points->Delete();
	points=NULL;
	if (cellVolume!=NULL)
		cellVolume->Delete();

	delete top;
}

int UTChemInputReader::canReadFile() {

  return icoord != 2 && parseResult != NO_FILE && !(icoord == 4 && idxyz == 2);
}

void UTChemInputReader::skipLines(std::ifstream& InputFile, int numLines, int& lineCount)
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

void UTChemInputReader::readNextLine(std::ifstream& InputFile, std::string& str, int& lineCount)
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

char* UTChemInputReader::consumeProcessed(char* ptr)
{
  return (char*)skipNonWhiteSpace(skipWhiteSpace(ptr));
}

void UTChemInputReader::getCellCenter(int i, int j, int k, float * out)
{
	if(objectType==0)
	{
		out[0] = dx1*(i+0.5);
		out[1] = dy1*(j+0.5);
		out[2] = dz1*(k+0.5);
		return;
	}
	if(objectType==1 || objectType==2)
	{
		out[0] = (xdim->GetValue(i)+xdim->GetValue(i+1))/2;
		out[1] = (ydim->GetValue(j)+ydim->GetValue(j+1))/2;
		out[2] = (zdim->GetValue(k)+zdim->GetValue(k+1))/2;
	
		if(objectType==2)
			out[2] = out[2] + top->topdim[i+j*nx];

		return;
	}
	else
		vtkOutputWindowDisplayErrorText("Unsupported object type");
}

float** UTChemInputReader::getCellCenters()
{
  if(cellCenters == NULL) {
    int id = 0;
    cellCenters = new float*[3];
    cellCenters[0] = new float[nx];
    cellCenters[1] = new float[ny];
    cellCenters[2] = new float[nz];

    if(getObjectType() == 0) { // Image data
      int curr = 0;
      for(int i = 0 ; i < nx ; ++i) {
        int next = curr + dx1;
        cellCenters[0][i] = (curr + next) / 2;
        curr = next;
      }

      curr = 0;
      for(int i = 0 ; i < ny ; ++i) {
        int next = curr + dy1;
        cellCenters[1][i] = (curr + next) / 2;
        curr = next;
      }

      curr = 0;
      for(int i = 0 ; i < nz ; ++i) {
        int next = curr + dz1;
        cellCenters[2][i] = (curr + next) / 2;
        curr = next;
      }
    } else { // Rectilinear Grid
      int curr = 0;
      for(int i = 0 ; i < nx ; ++i) {
        int next = curr + xspace[i];
        cellCenters[0][i] = (curr + next) / 2;
        curr = next;
      }

      curr = 0;
      for(int i = 0 ; i < ny ; ++i) {
        int next = curr + yspace[i];
        cellCenters[1][i] = (curr + next) / 2;
        curr = next;
      }

      curr = 0;
      for(int i = 0 ; i < nz ; ++i) {
        int next = curr + zspace[i];
        cellCenters[2][i] = (curr + next) / 2;
        curr = next;
      }
    }
  }

  return cellCenters;
}

UTChemInputReader::ParseState UTChemInputReader::readFile()
{
  std::string str;
  UTChemInputReader::ParseState retval = UTChemInputReader::FAIL_HEADER; // See enum in .h

  // NOTE: The functions must be called in order (due to how the file is setup)
  // otherwise this _WILL_ break

  // Make sure we start at the beginning
  InputFile.seekg(0, std::ios_base::beg);

  // Read first set of variables
  if((retval = readResvDesc(str)) != UTChemInputReader::SUCCESS)
    return retval;

  // Read the next set of variables under the Output Options Section
  if((retval = readOutputOpts(str)) != UTChemInputReader::SUCCESS)
    return retval;

  // Read the next set of variables under the Reservoir Properties Section
  if((retval = readReservoirProperties(str)) != UTChemInputReader::SUCCESS)
    return retval;

  if((retval = readWellInformation(str)) != UTChemInputReader::SUCCESS)
    return retval;

  return UTChemInputReader::SUCCESS;
}

UTChemInputReader::ParseState UTChemInputReader::readResvDesc(std::string& str)
{
  skipLines(35); // Eat lines we don't care about
  readNextLine(str);

  // CC SIMULATION FLAGS
  // *---- IMODE IMES IDISPC ICWM ICAP IREACT IBIO ICOORD ITREAC ITC IGAS IENG 
  // ICOORD:
  // 1 - Cartesian coordinate system is used
  // 2 - Radial coordinate system is used
  // 3 - Cartesian coordinate system with variable-width gridblock is used (2-D cross section only)
  // 4 - Curvilinear grid definition of the X-Z cross section is used (2-D or 3-D)
  if(12 != sscanf(str.c_str(), " %d %d %d %d %d %d %d %d %d %d %d %d", &imode, &imes, &idispc, &icwm, &icap, &ireact, &ibio, &icoord, &itreac, &itc, &igas, &ieng))
    return UTChemInputReader::FAIL_HEADER;

  skipLines(3);
  readNextLine(str);

  // CC NO. OF GRIDBLOCKS,FLAG SPECIFIES CONSTANT OR VARIABLE GRID SIZE, UNIT
  // *----NX   NY  NZ  IDXYZ   IUNIT
  // IDXYZ - Flag indicating constant or variable grid size.
  // 0 - Constant grid size
  // 1 - Variable grid size on a regional basis
  // 2 - Variable grid size
  // Note: IDXYZ must be set equal to 2 if ICOORD=3
  // IUNIT - Flag indicating English or Metric units.
  // 0 - English unit
  // 1 - Metric unit
  if(5 != sscanf(str.c_str(), " %d %d %d %d %d", &nx, &ny, &nz, &idxyz, &iunit))
    return UTChemInputReader::FAIL_HEADER;

  skipLines(3);
  readNextLine(str);

  // Double checking
  if(idxyz == 1) {
    // CC  CONSTANT GRID BLOCK SIZE IN X, Y, AND Z
    // *----DX double check
    int skip = readDVar(str, xspace, nx);

    skipLines(skip);
    readNextLine(str);

    // CC  CONSTANT GRID BLOCK SIZE IN X, Y, AND Z
    // *----Dy double check  
    skip = readDVar(str, yspace, ny);

    skipLines(skip);
    readNextLine(str);

    // CC  CONSTANT GRID BLOCK SIZE IN X, Y, AND Z
    // *----Dz 
    skip = readDVar(str, zspace, nz);

    skipLines(skip);
    readNextLine(str);

    // Finally setup the coordinates
    setupRGridCoords();
  } else if(idxyz == 0) {
    // CC  CONSTANT GRID BLOCK SIZE IN X, Y, AND Z
    // *----DX1       DY1      DZ1
    if(3 != sscanf(str.c_str(), " %f %f %f", &dx1, &dy1, &dz1))
      return UTChemInputReader::FAIL_HEADER;

    skipLines(3);
    readNextLine(str);
  } else if(idxyz == 2 && icoord == 4) { // Variable grid (support assumes icoord == 4 [curvilinear])
                                         // But some logic may or may not work for others
    int numCross = ((nx + 1) * (nz + 1)) * 2;
    std::vector<double> crossSection; // This vector is stored flat
    crossSection.reserve(numCross);

    // From UTChem manual:
    // XCORD(I), ZCORD(I), for I=1, (NX+1)×(NZ+1) (This line is read only if ICOORD=4)
    // CC  VARIABLE GRID BLOCK SIZE IN X
    // *----DX(I)     DZ(I)
    int skip = readDVar(str, crossSection, numCross);

    // According to UTChem manual, we can now enjoy a fixed Y under this
    // configuration
    // CC  CONSTANT GRID BLOCK SIZE IN Y
    // *----DY 
    skipLines(skip);
    readNextLine(str);
    skip = readDVar(str, yspace, ny);

   /* // Calculate xspace
    for(int i = 0, k = 0 ; i < nx ; i++, k += 2) {
      if((i % nz == 0) && i != 0)
        k += 2; // Skip the end points on spacing
      xspace.push_back(crossSection[k + 2] - crossSection[k]);
    }

    // Calculate zspace
    for(int i = 0, k = 1 ; i < nz ; i++, k += 2) {
      if((i % nx == 0) && i != 0)
        k += 2; // Skip the end points on spacing
      zspace.push_back(crossSection[k + 2] - crossSection[k]);
    }*/

    setupSGridPointsCurv(crossSection);
    skipLines(skip);
    readNextLine(str);
  }

  // CC TOTAL NO. OF COMPONENTS, NO. OF TRACERS, NO. OF GEL COMPONENTS
  // *----N   NO  NTW NTA  NGC NG  NOTH
  if(7 != sscanf(str.c_str(), " %d %d %d %d %d %d %d", &N, &no, &ntw, &nta, &ngc, &ng, &noth))
    return UTChemInputReader::FAIL_HEADER;

  skipLines(3);

  // CC  NAME OF SPECIES
  // *---- SPNAME(I) FOR I=1,N 
  for(int i = 0 ; i < N ; ++i) {
    readNextLine(str);
    species.push_back(str);
  }

  skipLines(3);
  readNextLine(str);

  if(itreac) { // It seems there exists variables without comments if this is enabled
    // sscanf(str.c_str(), " %d %d %d", &itreac1, &itreac2, &itreac3); // If we want to read these
    skipLines(3);
    readNextLine(str);
  }

  // CC FLAG INDICATING IF THE COMPONENT IS INCLUDED IN CALCULATIONS OR NOT
  // *----ICF(KC) FOR KC=1,N
  if(N != readIVarInLine(N, str.c_str(), icf))
    return UTChemInputReader::FAIL_HEADER;

  return UTChemInputReader::SUCCESS;
}

UTChemInputReader::ParseState UTChemInputReader::readOutputOpts(std::string& str)
{
  skipLines(10);
  readNextLine(str);

  // CC FLAG FOR PV OR DAYS TO PRINT OR TO STOP THE RUN
  // *----ICUMTM  ISTOP  IOUTGMS  IS3G
  if(4 != sscanf(str.c_str(), " %d %d %d %d", &icumtm, &istop, &ioutgms, &is3g)) {
    // *----ICUMTM  ISTOP  IOUTGMS (in ex09)
    // HACK: Need to see why this really changes
    if(3 != sscanf(str.c_str(), " %d %d %d", &icumtm, &istop, &ioutgms))
      return UTChemInputReader::FAIL_OUTPUTOPTS;
  }

  skipLines(3);
  readNextLine(str);

  // CC FLAG INDICATING IF THE PROFILE OF KCTH COMPONENT SHOULD BE WRITTEN
  // *----IPRFLG(KC),KC=1,N
  if(N != readIVarInLine(N, str.c_str(), iprflag))
    return UTChemInputReader::FAIL_OUTPUTOPTS;

  skipLines(3);
  readNextLine(str);

  // CC FLAG FOR individual map files
  // *----IPPRES IPSAT IPCTOT IPBIO IPCAP IPGEL IPALK IPTEMP IPOBS 
  if(9 != sscanf(str.c_str(), " %d %d %d %d %d %d %d %d %d", &ippres, &ipsat, &ipctot, &ipbio, &ipcap, &ipgel, &ipalk, &iptemp, &ipobs))
    return UTChemInputReader::FAIL_OUTPUTOPTS;

  skipLines(3);
  readNextLine(str);

  // CC FLAG for individual output map files
  // *----ICKL IVIS IPER ICNM ICSE ihystp  ifoamp  inoneq
  if(8 != sscanf(str.c_str(), " %d %d %d %d %d %d %d %d", &ickl, &icvis, &iper, &icnm, &icse, &ihystp, &ifoamp, &inoneq))
    return UTChemInputReader::FAIL_OUTPUTOPTS;

  skipLines(3);
  readNextLine(str);

  // CC FLAG  for variables to PROF output file
  // *----Iads IVel Irkf Iphse 
  if(4 != sscanf(str.c_str(), " %d %d %d %d", &iads, &ivel, &irkf, &iphse))
    return UTChemInputReader::FAIL_OUTPUTOPTS;

  return UTChemInputReader::SUCCESS;
}

UTChemInputReader::ParseState UTChemInputReader::readReservoirProperties(std::string& str)
{
  skipLines(10);
  readNextLine(str);

	// CC MAX. SIMULATION TIME ( DAYS)
  // *---- TMAX 
  if(1 != sscanf(str.c_str(), " %d", &tmax))
    return UTChemInputReader::FAIL_RESERVOIRPROPERTIES;

  skipLines(3);
  readNextLine(str);

  // CC rock comp. 0.000004 1/kpa
	// CC ROCK COMPRESSIBILITY (1/KPA), STAND. PRESSURE(KPA)
  // *----COMPR        PSTAND
  if(2 != sscanf(str.c_str(), " %lg %lg", &compr, &pstand))
    return UTChemInputReader::FAIL_RESERVOIRPROPERTIES;

	skipLines(3);
  readNextLine(str);

  // CC FLAGS INDICATION CONSTANT OR VARIABLE POROSITY, X,Y,AND Z PERMEABILITY
  // *----IPOR1 IPERMX IPERMY IPERMZ  IMOD   ITRANZ   INTG
  if(7 != sscanf(str.c_str(), " %d %d %d %d %d %d %d", &ipor1, &ipermx, &ipermy, &ipermz, &imod, & itranz, &intg)) {
    // HACK: Need to investigate why fewer parameters
    // *----IPOR1 IPERMX IPERMY IPERMZ  IMOD (in ex09)
    if(5 != sscanf(str.c_str(), " %d %d %d %d %d", &ipor1, &ipermx, &ipermy, &ipermz, &imod))
      return UTChemInputReader::FAIL_RESERVOIRPROPERTIES;
  }

	/*int skip=3;
	if (ipor1!=4) skip+=4;
	if (ipermx!=4) skip+=4;
	if (ipermy!=4 && icoord!=2) skip+=4;
	if (ipermz!=4) skip+=4;*/

  // HACK: Looks increasingly variable - so let's hope
  // comment line exists? :S
  while(0 != str.compare(0, strlen("*----IDEPTH"), "*----IDEPTH")) {
    readNextLine(str);
  }
  //skipLines(1); // Skip this line
  readNextLine(str);

  // CC FLAG FOR CONSTANT OR VARIABLE DEPTH, PRESSURE,WATER SATURATION
  // *----IDEPTH  IPRESS  ISWI  icwi
  if(4 != sscanf(str.c_str(), " %d %d %d %d", &idepth, &ipress, &iswi, &icwi))
    return UTChemInputReader::FAIL_RESERVOIRPROPERTIES;

	return UTChemInputReader::SUCCESS;
}
 

UTChemInputReader::ParseState UTChemInputReader::readWellInformation(std::string& str)
{
  // Maybe necessary to rewind file in the future? For now it does not seem this way (as long as this is read last)
  // based on our sample datasets

  // Read everything over and find our well data
  while(!InputFile.eof()) {
    readNextLine(str);

    // CC
    // CC WELL ID,LOCATIONS,AND FLAG FOR SPECIFYING WELL TYPE, WELL RADIUS, SKIN
    // *----IDW   IW    JW    IFLAG    RW     SWELL  IDIR   IFIRST  ILAST  IPRF
    // NOTE: This is, perhaps, too primitive as these comments may not always exist
    //   but seems to be present in all of our sample data sets.
    if(str.find("*----IDW") != std::string::npos || str.find("*---- IDW") != std::string::npos) {
      readNextLine(str);

      int idw, iw, jw, iflag, idir, ifirst, ilast, iprf;
      float rw, swell;
      std::vector<WellData::DeviatedCoords> deviated;

      int check = sscanf(str.c_str(), " %d %d %d %d %g %g %d %d %d %d", &idw, &iw, &jw, &iflag, &rw, &swell, &idir, &ifirst, &ilast, &iprf);
      // HACK: FIXME. In ex09 this line is skipping an integer in the middle?
      // the iflags value is not in the string according to the debugger? How?
      // Fix this and remove check != 3 (only reason it is here is due to the fact
      // that these values are currently unused)
      if(check != 10 && check != 3) {
        return UTChemInputReader::FAIL_WELLINFO;
      }

      if(idir == 4) { // Deviated well
        // Skip until we find the information
        while(!InputFile.eof() && str.find("*----IW") == std::string::npos) readNextLine(str);

        readNextLine(str);

        while(!InputFile.eof() && str.find("CC") == std::string::npos && str.find("cc") == std::string::npos) {
          WellData::DeviatedCoords coords;
          if(3 != sscanf(str.c_str(), " %d %d %d", &coords.i, &coords.j, &coords.k))
            return UTChemInputReader::FAIL_WELLINFO;
          deviated.push_back(coords);
          readNextLine(str);
        }
      }

      WellData x = { idw, iw, jw, iflag, rw, swell, idir, ifirst, ilast, iprf, deviated };
      wellInfo[idw] = x;
    }
  }

  return UTChemInputReader::SUCCESS;
}

void UTChemInputReader::setupRGridCoords()
{
    xdim = vtkDoubleArray::New();
    ydim = vtkDoubleArray::New();
    zdim = vtkDoubleArray::New();
    int i = 0;
  // We are using CellData so we need nx+1 coordinates
    double xpos = 0, ypos = 0, zpos = 0;
    xdim->InsertNextValue(xpos);
    ydim->InsertNextValue(ypos);
    zdim->InsertNextValue(zpos);

     assert(xspace.size() == nx);
     assert(yspace.size() == ny);
     assert(zspace.size() == nz);
    for(i = 0 ; i < nx ; ++i) {
      xpos += xspace[i];
      xdim->InsertNextValue(xpos);
    }
    for(i = 0 ; i < ny ; ++i) {
      ypos += yspace[i];
      ydim->InsertNextValue(ypos);
    }
    for(i = 0 ; i < nz ; ++i) {
      zpos += zspace[i];
      zdim->InsertNextValue(zpos);
    }
}

void UTChemInputReader::setupSGridPointsCurv(const std::vector<double>& cross)
{
	int i=0;
	int j=0;
	int k=0;
	points = vtkPoints::New();
	for (k=0; k<= nz; k++)
	{
		//double zbase = zdim!=NULL ? zdim->GetValue(k) : dz1*k;
		for (j=0; j<= ny; j++)
		{
			///double y = ydim!=NULL ? ydim->GetValue(j) : dy1*i;

			for (i=0; i<= nx; i++)
			{
				/*double zAverage = zbase;

				if(top!=NULL && top->isValid) {
				int iminusone = std::max(i-1, 0);
				int jminusone = std::max(j-1, 0);
				int boundedi = std::min(i, nx-1);
				int boundedj = std::min(j, ny-1);
			// Calculate the averge of 4 top values
				// being careful that we don't go past the end
				double z00 = top->topdim[boundedi+boundedj*nx];
				double z01 = top->topdim[iminusone+boundedj*nx];
				double z10 = top->topdim[boundedi+jminusone*nx];
				double z11 = top->topdim[iminusone+jminusone*nx];

				  zAverage += (z00+z01+z10+z11)/4.0;
				}

				double x = xdim!=NULL ? xdim->GetValue(i) : dx1*i;*/
				points->InsertNextPoint(cross[2 * i], yspace[j % ny], cross[(2 * k) + 1]);
			}
		}
	}
}

void UTChemInputReader::setupSGridPoints()
{
	int i=0;
	int j=0;
	int k=0;
	points = vtkPoints::New();
	for (k=0; k<=nz; k++)
	{
		double zbase = zdim!=NULL ? zdim->GetValue(k) : dz1*k;
		for (j=0; j<=ny; j++)
		{
			double y = ydim!=NULL ? ydim->GetValue(j) : dy1*i;

			for (i=0; i<=nx; i++)
			{
				double zAverage = zbase;

				if(top!=NULL && top->isValid) {
				int iminusone = std::max(i-1, 0);
				int jminusone = std::max(j-1, 0);
				int boundedi = std::min(i, nx-1);
				int boundedj = std::min(j, ny-1);
			// Calculate the averge of 4 top values
				// being careful that we don't go past the end
				double z00 = top->topdim[boundedi+boundedj*nx];
				double z01 = top->topdim[iminusone+boundedj*nx];
				double z10 = top->topdim[boundedi+jminusone*nx];
				double z11 = top->topdim[iminusone+jminusone*nx];

				  zAverage += (z00+z01+z10+z11)/4.0;
				}

				double x = xdim!=NULL ? xdim->GetValue(i) : dx1*i;
				points->InsertNextPoint(x,y,zAverage);
			}
		}
	}
}

void UTChemInputReader::readNextLine(std::string& str)
{
  UTChemInputReader::readNextLine(InputFile, str, line);
}

void UTChemInputReader::skipLines(int numLines)
{
  UTChemInputReader::skipLines(InputFile, numLines, line);
}

// readIVarInLine
//  - This function extracts numVars number of ints from a line and stores them in a container
// * numVars = Number of variables to be read
// * str = Beginning of the line
// * container = Where the variables will be stored
// Returns number of variables successfully read
int UTChemInputReader::readIVarInLine(int numVars, const char* str, std::vector<int>& container)
{
  int i = 0, tmp = 0;
  char* ptr = (char*)str;
  for(i = 0 ; i < numVars && ptr != NULL ; ++i, ptr=consumeProcessed(ptr)) {
    if(1 != sscanf(ptr, " %d", &tmp))
      return i;
    container.push_back(tmp);
  }
  return i;
}

// readDVar
//   - Much like readIVarInLine except no known boundaries
// * str = Beginning of line
// * container = here variables are stored
// Returns void
int UTChemInputReader::readDVar(std::string& str, std::vector<double>& container, int numExpected)
{
  while(str.find("cc") == std::string::npos && str.find("CC") == std::string::npos) {
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
        assert(count>0 && val>0);
        assert(numExpected <0 || count + container.size() <= numExpected);
         
        if(numExpected >=0  && numExpected < container.size() + count) {
          throw std::exception("Too many values read");
        }
        for(int i = 0 ; i < count ; ++i)
          container.push_back(val);
      } else if(1 == sscanf(ptr, "%lg", &val)) {
        container.push_back(val);
      } else {
        throw std::exception("Unexpected line format");
      }

      // Consume what was just processed
      ptr=consumeProcessed(ptr);
    }
    readNextLine(str);
  }
  if(numExpected >=0 && container.size() != numExpected) {
    std::string err("Invalid number of entries in INPUT on line: ");
    err.append(convertXToY<int, std::string>(line));
    err.append(" - numExpected = ");
    err.append(convertXToY<int, std::string>(numExpected));
    err.append(" , size = ");
    err.append(convertXToY<int, std::string>(container.size()));
    throw std::exception(err.c_str());
  }
  return 2; // Default
}

vtkDataObject * UTChemInputReader::getObject(vtkInformation* info)
{
  assert(gridObject);
  assert(info);

  if(gridObject && info)
    gridObject->SetPipelineInformation(info);
  else {
    vtkOutputWindowDisplayErrorText("No Grid Object or no vtkInformation! - determineGridType not called?");
  }
  return gridObject;
}

void UTChemInputReader::determineGridType()
{
  if(gridObject)
    gridObject->Delete();
  gridObject = NULL;

	if (points)
	{
		objectType = 2;
		gridObject = vtkStructuredGrid::New();
	}
	else
  if((icoord > 1 && icoord < 5) || idxyz == 2) { // Using only rectilinear grids for all other types - this may not be correct
    objectType = 1;
    gridObject = vtkRectilinearGrid::New();
  } else { // icoord == 1 or by default
    objectType = 0;
    gridObject = vtkImageData::New();
  }
}

int UTChemInputReader::getObjectType() const
{
  return objectType;
}

void UTChemInputReader::calculateCellVolume()
{
	cellVolume = vtkFloatArray::New();
	cellVolume->SetName("Cell_Volume");
	if (!xspace.empty() && !yspace.empty() && !zspace.empty())
	{
		for (int k=0; k<zspace.size(); k++)
		{
			for (int j=0; j<yspace.size(); j++)
			{
				for (int i=0; i<xspace.size(); i++)
				{
					cellVolume->InsertNextValue(xspace[i]*yspace[j]*zspace[k]);
				}
			}
		}
	}
	else
	{
		double volume = (dx1)*(dy1)*(dz1);
		for (int i = 0; i<(nx*ny*nz); i++)
			cellVolume->InsertNextValue(volume);
	}
}