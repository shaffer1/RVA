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

#include "UTChemInputReader.h"
#include "UTChemTopReader.h"
#include "RVA_Util.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"


UTChemInputReader::UTChemInputReader(const std::string& input)
  : gridObject(NULL), objectType(0), xdim(NULL), ydim(NULL), zdim(NULL),
  parseResult(UTChemInputReader::NO_FILE), points(NULL), top(NULL), cellVolume(NULL),
   N(0), r1(0), dx1(1), dy1(1), dz1(1), iads(0), ibio(0), icap(0), ickl(0), icnm(0), icoord(0), 
	 icse(0), icumtm(0), icvis(0), icwm(0), idispc(0), idxyz(0), ieng(0), ifoamp(0), igas(0), 
	 ihystp(0), imes(0), imode(0), inoneq(0), ioutgms(0), ipalk(0), ipbio(0), ipcap(0), 
	 ipctot(0), iper(0), ipgel(0), iphse(0), ipobs(0), ippres(0), ipsat(0), iptemp(0), 
	 ireact(0), irkf(0), is3g(0), istop(0), itc(0), itreac(0), iunit(0), ivel(0), ng(0), 
	 ngc(0), no(0), noth(0), nta(0), ntw(0), nx(0), ny(0), nz(0), cellCenters(NULL),
	 tmax(0), compr(0), pstand(0), ipor1(0), ipermx(0), ipermy(0), ipermz(0), imod(0), 
	 itranz(0), intg(0), idepth(0), ipress(0), iswi(0), icwi(0)
{
	InputFile.open(input.c_str());
	if (InputFile.is_open()) {
		parseResult = readFile();

		// MVM: according to 9.3 user guide idepth should be {0, 1, 2}
		// I'm not sure what this was trying to accomplish. 
		if (idepth==4)
		{
			size_t found;
			std::string key ("INPUT");
			std::string input_copy (input);
			found = input_copy.rfind(key);
			input_copy.replace(found, key.length(), "TOP");
			top = new UTChemTopReader(input_copy.c_str(), nx, ny);
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
		case UTChemInputReader::NO_FILE:
			vtkOutputWindowDisplayErrorText("No readable INPUT file in directory.");
			isValid = false;
			break;
		default:
			isValid = true;
			break;
	}
	
	if (parseResult != UTChemInputReader::NO_FILE)
	{
		determineGridType();
		calculateCellVolume();
	}
	
	InputFile.close(); 
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
  return parseResult != NO_FILE;
}

void UTChemInputReader::skipLines(int numLines)
{
  std::string tmp;

  for(int i = 0 ; i < numLines ; ++i) {
      std::getline(InputFile, tmp);
  }
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

  // Read Title and Reservoir Decription Data section 3.1
  if ((retval = readResvDesc()) != UTChemInputReader::SUCCESS) 
  {
    return retval;
  }

  // Read Output Option Data section 3.2
  if ((retval = readOutputOpts()) != UTChemInputReader::SUCCESS)
  {
    return retval;
  }

  // Read Reservoir Properties section 3.3
  if ((retval = readReservoirProperties()) != UTChemInputReader::SUCCESS)
  {
    return retval;
  }

  // Skip General Physical Property Data section 3.4
  // Skip Physical Property Data for Geochemical Options section 3.5
  // Skip Data for Biodegradation Option section 3.6

  // Read Recurrent Injection/Production Data Set section setion 3.7
  if((retval = readWellInformation()) != UTChemInputReader::SUCCESS) {
	  return retval;
  }

  return UTChemInputReader::SUCCESS;
}

UTChemInputReader::ParseState UTChemInputReader::readResvDesc()
{
	using std::getline;
  // Section #.#.# refers to section in UTCHEM User's Guide
	std::string str;
  std::stringstream ss;

  // Header
  skipLines(22);  

  // Section 3.1.1
  skipLines(3+1);

  // Section 3.1.2
  skipLines(3+3);
 
  // Section 3.1.3
  skipLines(3);   
  getline(InputFile, str);

  ss << str;
  ss >> imode >> imes >> idispc >> icwm 
	  >> icap >> ireact >> ibio >> icoord
	  >> itreac >> itc >> igas >> ieng;

  // Section 3.1.4
  skipLines(3); 
  getline(InputFile, str);

  ss.clear();
  ss.str(str);
  ss >> nx >> ny >> nz >> idxyz >> iunit;

  // Next set of sections depends on idxyz and icoord
  skipLines(3);  
  getline(InputFile, str); // might want to move this read inside the branching...
  
  switch (idxyz) {
	case 0:	
    	// Globally Constant	
		if (icoord == 1) {
			// Cartesian
  			// Section 3.1.6  
			ss.clear();
			ss.str(str);
			ss >> dx1 >> dy1 >> dz1;
		}
		else if (icoord == 2) {
			// Radial
			// Section 3.1.7 
			ss.clear();
			ss.str(str);
			ss >> r1 >> dx1 >> dz1;
		}
		else if (icoord == 4) {
			// Curvilinear
			// Sections 3.1.5 and 3.1.8
			readCurvilinearXZ(str, xspace, zspace);

			skipLines(2);
			getline(InputFile, str);

			ss.clear();
			ss.str(str);
			ss >> dy1;
			// use dy1 to populate yspace for later
			// note to self - I'm expanding the dys here...
			// need to be consistent about that!
			//for (int i = 0; i <= ny; i++) {
				//yspace.push_back(double(i * dy1));
			//}
			for (int i = 0; i < ny; i++) {
				yspace.push_back(dy1);
			}
			setupSGridCoords();
		}
		else {
			// unrecognized IDXYZ/ICOORD combination
			std::cerr << "Unrecognized IDXYZ/ICOORD combination.\n";
		}
		break;	
	case 1:
		// regionally variable
		if (icoord == 1) {
			// Cartesian
			// Section 3.1.9 
    		readRegionalCoords(str, xspace, nx);

			// Section 3.1.10
    		skipLines(3);
    		getline(InputFile, str);
    		readRegionalCoords(str, yspace, ny);

			// Section 3.1.11
    		skipLines(3);
    		getline(InputFile, str);
    		readRegionalCoords(str, zspace, nz);

    		// Finally setup the coordinates
    		setupRGridCoords();
		}
		else if (icoord == 2) {
			// Radial
			// Sections 3.1.12, 3.1.13, 3.1.14
			std::cerr << "Reading regionally variable radial grids unsupported.\n";
		}
		else if (icoord == 4) {
			// Curvilinear
			// Sections 3.1.5, 3.1.15
			readCurvilinearXZ(str, xspace, zspace);
			
			skipLines(2);
			getline(InputFile, str);
			readRegionalCoords(str, yspace, ny);
			setupSGridCoords();
		}
		else {
			// Unimplmented IDXYZ/ICOORD combo
		}
		break;	
	case 2:
		// globally variable
		if (icoord == 1) {
			// Cartesian
	  		// Section 3.1.16
	  		// NX delta x's
	  		std::stringstream ss;	
	  		ss.str(str);
			for (int i = 0; i < nx; i++)
			{
		  		double tmp;
		  		ss >> tmp;
		  		xspace.push_back(tmp);
			}

			// Section 3.1.17
			skipLines(3);
			getline(InputFile, str);
			ss.clear();
			ss.str(str);
			for (int i = 0; i < ny; i++) 
			{
				double tmp;
				ss >> tmp;
				yspace.push_back(tmp);
			}

			// Section 3.1.19
			skipLines(3);
			getline(InputFile, str);
			ss.clear();
			ss.str(str);
			for (int i = 0; i < nz; i++)
			{
				double tmp;
				ss >> tmp;
				zspace.push_back(tmp);
			}
			setupRGridCoords();
		}
		else if (icoord == 2) {
			// Radial
			std::cerr << "Globally variable radial files not supported.\n";
		}
		else if (icoord == 3) {
			// weird Cartesian
			std::cerr << "Current type of cartesian grid not supported.\n";
		}
		else if (icoord == 4) {
			// 20140624 -- seems to read fine, but later on something
			// happens to prevent the file from being read.
			// Curvilinear
	  		// Section 3.1.5, 3.1.17
      		readCurvilinearXZ(str, xspace, zspace);	

			// Section 3.1.17
    		skipLines(2);
    		getline(InputFile, str);
			std::cout << "3.1.17 str: " << str << "\n";
			ss.clear();
			ss.str(str);

			for (int i = 0; i < ny; i++) 
			{
				double tmp;
				ss >> tmp;
				yspace.push_back(tmp);
			}
		
			setupSGridCoords();
		}
		else {
			// unrecognized IDXYZ/ICOORD combo 
		}

		break;
	default:
		// unrecognized IDXYZ value
		break;
  }

  // User Guide section 3.1.23
  skipLines(3); 
  getline(InputFile, str);
  ss.clear();
  ss.str(str);
  ss >> N >> no >> ntw >> nta >> ngc >> ng >> noth; 
 
  // Section 3.1.24 - this will vary in size
  skipLines(3);
  for (int i = 0 ; i < N ; ++i) {
    getline(InputFile, str);
	species.push_back(str);
  }

  // Section 3.1.25 - only exists if NTW > 0 and ITREAC == 1?
  if (ntw > 0 && itreac == 1) {
	skipLines(3);
    getline(InputFile, str);
  }

  // Section 3.1.26
  skipLines(3);
  getline(InputFile, str);
  if (N != readIVarInLine(N, str.c_str(), icf)) {
    return UTChemInputReader::FAIL_HEADER;
  }

  return UTChemInputReader::SUCCESS;
}

UTChemInputReader::ParseState UTChemInputReader::readOutputOpts()
{
	std::string str;
	std::stringstream ss;

	// skipping section header
	skipLines(7);

	// Section 3.2.1
  	skipLines(3);
  getline(InputFile, str);
  ss.str(str);
  ss >> icumtm >> istop >> ioutgms;

  // Section 3.2.2
  // What is done with this section?
  skipLines(3);
  getline(InputFile,str);
  ss.clear();
  ss.str(str);
  int tmp;
  while (ss >> tmp)
	  ;

  // Section 3.2.3
  skipLines(3);
  getline(InputFile,str);
  ss.clear();
  ss.str(str);
  ss >> ippres >> ipsat >> ipctot >> ipbio >> ipcap >> ipgel >> ipalk >> iptemp >> ipobs;

  // Section 3.2.4
  skipLines(3);
  getline(InputFile,str);
  ss.clear();
  ss.str(str);
  ss >> ickl >> icvis >> iper >> icnm >> icse >> ihystp >> ifoamp >> inoneq;

  // Section 3.2.5
  skipLines(3);
  getline(InputFile,str);
  ss.clear();
  ss.str(str);
  ss >> iads >> ivel >> irkf >> iphse;

  // Section 3.2.6
  int nobs;
  if (ipobs == 1) {
	  skipLines(3);
	  getline(InputFile,str);
	  ss.clear();
	  ss.str(str);
	  ss >> nobs;
  }

  // Section 3.2.7
  if (ipobs == 1 && nobs > 0) 
  {
  	  skipLines(3);
	  getline(InputFile,str);
  }

  return UTChemInputReader::SUCCESS;
}

UTChemInputReader::ParseState UTChemInputReader::readReservoirProperties()
{
  std::string str;
  std::stringstream ss;
  // skip header
  skipLines(7);

  // Section 3.3.1
  skipLines(3);
  getline(InputFile,str);
  ss.str(str);
  ss >> tmax;

  // Section 3.3.2
  skipLines(3);
  getline(InputFile,str);
  ss.clear();
  ss.str(str);
  ss >> compr >> pstand;

  // Section 3.3.3
	skipLines(3);
  getline(InputFile,str);
  ss.clear();
  ss.str(str);
  ss >> ipor1 >> ipermx >> ipermy >> ipermz >> imod;

  // Following sections depend on previous line
  if (ipor1 == 0) {
	  // Section 3.3.4
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipor1 == 1) {
	  // Section 3.3.5 1,nz
	  skipLines(3);
	  getline(InputFile,str); 

  }
  else if (ipor1 == 2) {
	  // Section 3.3.6 1, nx*ny*nz
	  skipLines(3);
	  std::string tmp;
	  for (int i = 0; i < (nx * ny * nz); i++ )
	  {
		  InputFile >> tmp;
	  }
	  InputFile.ignore(1, '\n');
  }
  
  if (ipermx == 0) {
	  // Section 3.3.7
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipermx == 1) {
	  // Section 3.3.8 1,nz
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipermx == 2) {
	  // Section 3.3.9 1, nx*ny*nz
	  skipLines(3);
	  std::string tmp;
	  for (int i = 0; i < (nx * ny * nz) ; i++ )
	  {
		  InputFile >> tmp; 
	  }
	  InputFile.ignore(1, '\n');
  }

  if (ipermy == 0 && icoord != 2) {
	  // Section 3.3.10
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipermy == 1 && icoord != 2) {
      // Section 3.3.11 1, nz
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipermy == 2 && icoord != 2) {
	  // Section 3.3.12 1,nx*ny*nz
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipermy == 3 && icoord != 2) {
	  // Section 3.3.13
	  skipLines(3);
	  getline(InputFile,str);
  }

  if (ipermz == 0) {
	  // Section 3.3.14
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipermz == 1) {
	  // Section 3.3.15 1,nz
	  skipLines(3);
	  getline(InputFile,str);
  }
  else if (ipermz == 2) {
	  // Section 3.3.16 1,nx*ny*nz
	  skipLines(3);
	  std::string tmp;
	  for (int i = 0; i < (nx*ny*nz); i++)
	  {
		  InputFile >> tmp;
	  }
	  InputFile.ignore(1, '\n');
  }
  else if (ipermz == 3) {
	  // Section 3.3.17
	  skipLines(3);
	  getline(InputFile,str);
  }
  
  // Section 3.3.18
  skipLines(3);
  getline(InputFile,str);
  ss.clear();
  ss.str(str);
  ss >> idepth >> ipress >> iswi >> icwi;

  return UTChemInputReader::SUCCESS;
}
 

UTChemInputReader::ParseState UTChemInputReader::readWellInformation()
{
  std::string str;
  // Section 3.7
  std::stringstream ss;
  // MVM: seems to hunt for info in sections 3.7.6.a and ?  
  while(!InputFile.eof()) {
    getline(InputFile,str);

	// MVM: The real problem is that there are multiple sections that start with "IDW"
	// 3.7.6.a, what we want, and 3.7.2* where according to the User Guide it should be
	// "ID" but is really "IDW".
	// What we really want here is a regexp matching

    // CC
    // CC WELL ID,LOCATIONS,AND FLAG FOR SPECIFYING WELL TYPE, WELL RADIUS, SKIN
    // *----IDW   IW    JW    IFLAG    RW     SWELL  IDIR   IFIRST  ILAST  IPRF
	
    if(str.find("*----IDW") != std::string::npos || str.find("*---- IDW") != std::string::npos) {
      getline(InputFile,str);
	  ss.clear();
      ss.str(str);
      int idw, iw, jw, iflag, idir, kfirst, klast, iprf;
      float rw, swell;
      std::vector<WellData::DeviatedCoords> deviated;

	  if (!(ss >> idw >> iw >> jw >> iflag >> rw >> swell >> idir >> kfirst >> klast >> iprf)) {
		// MVM: This probably is not an error in reading, but rather that IDW is being
		// used as an identifier in a different section.
		
		// This will have to be completely redone!
		//return UTChemInputReader::FAIL_WELLINFO;
	  }
      
      if(idir == 4) { // Deviated well
        // Skip until we find the information
        while(!InputFile.eof() && str.find("*----IW") == std::string::npos) getline(InputFile,str);

        getline(InputFile,str);

        while(!InputFile.eof() && str.find("CC") == std::string::npos && str.find("cc") == std::string::npos) {
          WellData::DeviatedCoords coords;
          ss.clear();
		  ss.str(str);
		  if (!(ss >> coords.i >> coords.j >> coords.k)) {
			  return UTChemInputReader::FAIL_WELLINFO;
		  } 
          
		  deviated.push_back(coords);
          getline(InputFile,str);
        }
      }

      WellData x = { idw, iw, jw, iflag, rw, swell, idir, kfirst, klast, iprf, deviated };
      wellInfo[idw] = x;
    }
  }
  return UTChemInputReader::SUCCESS;
}

void UTChemInputReader::setupRGridCoords()
{
	// MVM: change to use smartpointers
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

    for (i = 0; i < nx; ++i) {
      xpos += xspace[i];
      xdim->InsertNextValue(xpos);
    }

    for (i = 0; i < ny; ++i) {
      ypos += yspace[i];
      ydim->InsertNextValue(ypos);
    }

    for (i = 0; i < nz; ++i) {
      zpos += zspace[i];
      zdim->InsertNextValue(zpos);
    }
}

void UTChemInputReader::setupSGridCoords()
{
	// change to smartpointer
	points = vtkPoints::New();

	// note: the way the coords are given in the file is the
	// equivalent to x-z-y fastest, but VTK expects x-y-z	

	// change this - UTChem gives deltas, but VTK wants absolutes
	// need to convert the dys to absolutes, x-z-y ordering messes up
	// the obvious way to do this!
	std::vector<double> y;
	double ypos = 0.0;
	y.push_back(ypos);
	
	for (int i = 1; i < ny+1; i++) {
		ypos += yspace[i-1];
		y.push_back(ypos);
	}

	for (int k = 0; k <= nz; k++) {
		for (int j = 0; j <= ny; j++) {
			for (int i = 0; i <= nx; i++) {
				points->InsertNextPoint(xspace[i], y.at(j), zspace[k]);

			}
		}
	}
}

// MVM: change to return void or elide
int UTChemInputReader::readIVarInLine(int numVars, const char* str, std::vector<int>& container)
{
  std::stringstream ss;
  ss.str(str);
  int item;
  int i = 0;
  while (ss >> item) 
  {
	  container.push_back(item);
	  i++;
  }
  return i;
}

void UTChemInputReader::readRegionalCoords(std::string& str, std::vector<double>& container, int numExpected)
{
  int begin = -1;
  int end = -1;
  double delta = 0.0;

  std::stringstream ss;

  // MVM: ugly, needs to be revisited, not sure I like how the first line is
  // sent in...
  // MVM: also change to read in num items not lines
  while (true) 
  {
	ss.clear();
	ss.str(str);
	ss >> begin >> end >> delta;
	for (int i = begin - 1; i < end; i++) 
	{
		container.push_back(delta);
	}	
  
	if (end == numExpected) 
	{
		break;
	}
	getline(InputFile,str);
  }
  std::cout << "container size: " << container.size() << std::endl;
  for (int i = 0; i < container.size(); i++) {
	  std::cout << container[i] << std::endl;
  }
}

void UTChemInputReader::readCurvilinearXZ(std::string& str, std::vector<double>& xcoords, std::vector<double>& zcoords)
{
	// Reads section 3.1.5 which is used for all three types of curvilinear files.
	// This is (nx+1)*(nz+1) lines of x,z coordinate pairs.
	std::stringstream ss;
	double x, z;
	int lines = (nx+1)*(nz+1);
	for (int i = 0; i < lines; ++i) {
		ss.clear();
		ss.str(str);
		ss >> x >> z;
		xcoords.push_back(x);
		zcoords.push_back(z);
		getline(InputFile,str);
	}
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
  if (gridObject) {
    gridObject->Delete();
  }
  gridObject = NULL;

  // Note that IDXYZ 2, ICOORD 3 is not handled, it
  // is probably going to be a vtkUnstructuredGrid.
  if (points)
  {
	// the Curvilinear types
    objectType = 2;
    gridObject = vtkStructuredGrid::New();
  }
  //else if (idxyz == 0 && (icoord == 1 || icoord == 2)) { 
  else if (idxyz == 0 && icoord == 1) {
  // constant Cartesian or Radial
    objectType = 0;
    gridObject = vtkImageData::New();
  } else { 
	// variable Cartesian or Radial
	objectType = 1;
    gridObject = vtkRectilinearGrid::New();
  } 
  
}

int UTChemInputReader::getObjectType() const
{
  return objectType;
}

// MVM: why not use the PV filter to calc cell volumes?
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
