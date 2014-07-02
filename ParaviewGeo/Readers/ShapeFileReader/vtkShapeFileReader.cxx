// .NAME vtkShapeFileReader.cxx
// Read Data Files (*.shp / *.shx) ESRI data files.
// POINT, LINE, POLYGON, MESH
// Without properties
#include "vtkShapeFileReader.h"
#include "shpread.h"
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <string>
#include <iostream>

#include "mgdecl.h"
#include "hpolygon.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkFloatArray.h"
#include "vtkStringList.h"
#include "vtkLongArray.h"
#include <vtkOutputWindow.h>

// define Gocad file type identifers.
#define POINTOBJ 1
#define LINEOBJ 2
#define FACEOBJ 3
#define MESHOBJ 4

vtkCxxRevisionMacro(vtkShapeFileReader, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkShapeFileReader);

// Constructor
vtkShapeFileReader::vtkShapeFileReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
};

// --------------------------------------
// Destructor
vtkShapeFileReader::~vtkShapeFileReader()
{
  this->SetFileName(0);
}

// --------------------------------------
void vtkShapeFileReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}

// --------------------------------------
int vtkShapeFileReader::RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector)
{
  // Make sure we have a file to read.
  if(!this->FileName)  {
    vtkErrorMacro("A FileName must be specified.");
    return 0;
  }
  if(strlen(this->FileName)==0)  {
    vtkErrorMacro("A NULL FileName.");
    return 0;
  }

	//check for the .shx & .shp our selves.
	vtkstd::string shxName = this->FileName;
	int shxpos = shxName.find_last_of(".");
	shxName.replace(shxpos,4,".shx");
  ifstream shp(this->FileName);
	ifstream shx( shxName.c_str() );
	
	bool ReadFailed = false;
	if ( !shx.good() )
		{
		shx.close();
		shxName.replace(shxpos,4,".SHX");
		shx.open( shxName.c_str() );
		if ( !shx.good() )
			{
			ReadFailed = true;			
			vtkErrorMacro("Could not find the .SHX or .shx file " );
			}		
		}

	if ( !shp.good() )
		{
		vtkErrorMacro("Could not find the .shp file" );
		}
	shp.close();
	shx.close();

	if ( ReadFailed )
		{
		return 0;
		}	
	
	
  

  vtkIdType* nodes = NULL;
  vtkIdType fnodes[4];
  vtkSmartPointer<vtkPoints> myPointsPtr = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> myCellsPtr = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkCellArray> faceCellsPtr = vtkSmartPointer<vtkCellArray>::New();
  vtkFloatArray* fap = NULL;
  int filetype = 0;
  HPolygon poly;
  int nPolygons = 0;
  double baseZ;
	vtkLongArray* fapLAP = vtkLongArray::New();

  // Get SHP Handle (SHPInfo struct pointer.
  SHPHandle shpih = SHPOpen(this->FileName, "rb"); 
	if ( shpih == NULL )
		{
		//incase the .shx is missing or the shape file reader failed		
		fapLAP->Delete();
		return 0;
		}

	SHPObject* shpop;


  switch (shpih->nShapeType)
  {
  case SHPT_NULL:
    vtkErrorMacro("NULL Object type." << this->FileName);
    SHPClose(shpih);
		fapLAP->Delete();
    return 0;
    break;
  case SHPT_POINT:
  case SHPT_POINTZ:
  case SHPT_POINTM:
  case SHPT_MULTIPOINT:
  case SHPT_MULTIPOINTZ:
  case SHPT_MULTIPOINTM:
    filetype = POINTOBJ;
    break;
  case SHPT_ARC:  // Polyline
  case SHPT_ARCZ:
  case SHPT_ARCM:
    filetype = LINEOBJ;
    break;
  case SHPT_POLYGON:
  case SHPT_POLYGONZ:
  case SHPT_POLYGONM:
    filetype = FACEOBJ;  // (default) form triangulated surfaces.
    // arbitrary limit set to avoid triangulation overload (e.g. ArcGIS\ArcTutor\Map\airport.mdb.shp)
    for (int ri0=0; ri0<shpih->nRecords; ri0++) {
      shpop = SHPReadObject(shpih,ri0);
      if (shpop->nVertices > 100)  { filetype = LINEOBJ; break; } // form polygon outline only.
    }
    break;
  case SHPT_MULTIPATCH:
    filetype = MESHOBJ;
    nPolygons = 0;
    for (int ri0=0; ri0<shpih->nRecords; ri0++) {
      shpop = SHPReadObject(shpih,ri0);
      if (shpop->nSHPType!=SHPP_TRISTRIP && shpop->nSHPType!=SHPP_TRIFAN)
        nPolygons += shpop->nParts;
    }
    if (nPolygons>0)  poly.mtabSize.resize(nPolygons);
    break;
  default:
    vtkErrorMacro("Unknown Object type." << this->FileName);
    SHPClose(shpih);
		fapLAP->Delete();
    return 0;
  }
  	
  char errbuff[80];
  int pid=0, pidBase, ei, ei0, si, pi, vi, nodecnt, polypart=0;

  for (int ri=0; ri<shpih->nRecords; ri++) {
    shpop = SHPReadObject(shpih,ri);  
    if (shpop->nSHPType != shpih->nShapeType && shpih->nShapeType != SHPT_MULTIPATCH) {
      sprintf(errbuff,"Object type (%d) not match overall type (%d), skipped record %d",
        (int)shpop->nSHPType, (int)shpih->nShapeType, ri+1);
      vtkErrorMacro("Error: " << errbuff);
      continue;  // skip it.
    }
    switch (filetype)
    {
    case POINTOBJ:
      if (nodes!=NULL)  delete nodes;
      nodes = new vtkIdType[shpop->nVertices];
      for (vi=0; vi<shpop->nVertices; vi++) {
          myPointsPtr->InsertPoint(pid,shpop->padfX[vi],shpop->padfY[vi],shpop->padfZ[vi]);
          nodes[vi] = pid++;
          // shpop->padfM[vi]);
      }  
      myCellsPtr->InsertNextCell(shpop->nVertices, nodes);
      delete nodes;
      nodes = NULL;
      break;
    case LINEOBJ:
      for (pi=0; pi<shpop->nParts; pi++) {
        si = shpop->panPartStart[pi];
        ei = (pi==shpop->nParts-1) ? shpop->nVertices : shpop->panPartStart[pi+1];
        if (nodes!=NULL)  delete nodes;
        nodes = new vtkIdType[ei-si];
        for (vi=si; vi<ei;vi++) {
          myPointsPtr->InsertPoint(pid,shpop->padfX[vi],shpop->padfY[vi],shpop->padfZ[vi]);
          nodes[vi-si] = pid++;
        }
        myCellsPtr->InsertNextCell(ei-si, nodes);
      }  
      break;
    case FACEOBJ:						
      poly.mtabSize.resize(shpop->nParts);  // HGRD
      baseZ = shpop->padfZ[0];  // default Z for polygon(s) of this record.
      for (pi=0; pi<shpop->nParts; pi++) {
        si = shpop->panPartStart[pi];
        ei = (pi==shpop->nParts-1) ? shpop->nVertices : shpop->panPartStart[pi+1];
        ei0 = ei;
        if (ei>si+200)  { ei=si+200; nodecnt=201; }
        else  { nodecnt = ei-si; }
        poly.mtabSize[pi]=nodecnt;
				polypart++;
        for (vi=si; vi<ei;vi++) {
          poly.mtabPnt.insert( poly.mtabPnt.end(),Vect2D(shpop->padfX[vi],shpop->padfY[vi]) );
        }
        if (ei != ei0)
          poly.mtabPnt.insert( poly.mtabPnt.end(),Vect2D(shpop->padfX[si],shpop->padfY[si]) );
      }
      pidBase = pid;
      // *** DO TRIANGULATION ON poly(s) for this record AND BUILD FACE CELLS ***
      poly.Triangulate();
      for (int pti=0; pti<(int)poly.mtabPnt.size(); pti++)
        myPointsPtr->InsertPoint(pid++,poly.mtabPnt[pti].X(),poly.mtabPnt[pti].Y(),baseZ);
      for (int tri=0; tri<(int)poly.mtabCell.size(); tri++) {
        for (int ni=0; ni<3; ni++)
          fnodes[ni] = pidBase + poly.mtabCell[tri].Index(ni);
        faceCellsPtr->InsertNextCell(3, fnodes);
      }
      break;
    case MESHOBJ:
      for (pi=0; pi<shpop->nParts; pi++) {
        if (shpop->nSHPType==SHPP_TRISTRIP || shpop->nSHPType==SHPP_TRIFAN) {
          pidBase = pid;
          si = shpop->panPartStart[pi];
          ei = (pi==shpop->nParts-1) ? shpop->nVertices : shpop->panPartStart[pi+1];
          for (vi=si; vi<ei;vi++) {
            myPointsPtr->InsertPoint(pid++,shpop->padfX[vi],shpop->padfY[vi],shpop->padfZ[vi]);
          }
          switch (shpop->nSHPType)
          {
          case SHPP_TRISTRIP:
            for (int sti=pidBase+2; sti<pid; sti++) {
              fnodes[0] = sti-2;
              fnodes[1] = sti-1;
              fnodes[2] = sti;
              faceCellsPtr->InsertNextCell(3, fnodes);
            }
            break;
          case SHPP_TRIFAN:
            for (int sti=pidBase+2; sti<pid; sti++) {
              fnodes[0] = pidBase;
              fnodes[1] = sti-1;
              fnodes[2] = sti;
              faceCellsPtr->InsertNextCell(3, fnodes);
            }
            break;
          }
        }
        else {  // all polygon (RING) cases
          // Combine all Rings before triangulation to produce correct holes.
          baseZ = shpop->padfZ[0];  // default Z for ALL polygon(s).
          for (pi=0; pi<shpop->nParts; pi++) {
            si = shpop->panPartStart[pi];
            ei = (pi==shpop->nParts-1) ? shpop->nVertices : shpop->panPartStart[pi+1];
            poly.mtabSize[polypart++]=ei-si;
            for (vi=si; vi<ei;vi++) {
              poly.mtabPnt.insert( poly.mtabPnt.end(),Vect2D(shpop->padfX[vi],shpop->padfY[vi]) );
            }
          }
        }
      }
      break;
    }
    SHPDestroyObject(shpop);
  }
  if (filetype==MESHOBJ && nPolygons>0) {
    // *** DO TRIANGULATION ON accumulated poly(s) AND BUILD FACE CELLS ***
    pidBase = pid;
    poly.Triangulate();
    for (int pti2=0; pti2<(int)poly.mtabPnt.size(); pti2++)
      myPointsPtr->InsertPoint(pid++,poly.mtabPnt[pti2].X(),poly.mtabPnt[pti2].Y(),baseZ);
    for (int tri2=0; tri2<(int)poly.mtabCell.size(); tri2++) {
      for (int ni=0; ni<3; ni++)
        fnodes[ni] = pidBase + poly.mtabCell[tri2].Index(ni);
      faceCellsPtr->InsertNextCell(3, fnodes);
    }
  }
  SHPClose(shpih);
  if (nodes!=NULL)  { delete nodes; nodes=NULL; }

	int propcnt = 0;
/***     WORK-IN-PROGRESS ***
	//  - - - - - - - - - - - Prepare to process DBF file.  - - - - - - - - - - -
	int ichar, ifldsz, labid, recordsz;
	int fldcnt, fldstart, fi, ti, recordcnt = 0;
	long startpos = 0x20;
	char fldtxt[40];
	float fdata, dbfdata[1000][40];
	char dbfname[128];
	char dbuff[1024];
  char label[40][12];
  char labty[40];
  int fldsz[40];
  int flddp[40];

	FILE* shpdb = fopen("D:\\temp\\shpdb.txt","w");
	strcpy(dbfname,this->FileName);
	strcpy(&dbfname[strlen(dbfname)-3],"dbf");  // replace 'shp' with 'dbf'
	fputs(dbfname,shpdb);
	fputs("\n",shpdb);
	FILE* dbffp = fopen(dbfname,"r");
	fprintf(shpdb,"dbffp = %d\n",(long)dbffp);
	fclose(shpdb);

	labid = 0;
	recordsz = 0;
	fldcnt = 0;

	while (true) {
		if (dbffp==NULL) {
			vtkErrorMacro("Unable to open dbf file: " << dbfname );
			break;
		}
		clearerr(dbffp);
		int result = fseek(dbffp,0x20,0);
		if (result!=0)  {
			vtkErrorMacro("Unable to seek to position 20H - " << strerror(errno));
			fclose(dbffp);
			break;
		}
		ichar = fgetc(dbffp);
		while (ichar != 0x0d) {  // test for end of field labels
			dbuff[0] = ichar;
			fread(&dbuff[1], 0x1f, 1, dbffp);  // read rest of label field
			strncpy(label[labid],dbuff,11);  // field name
			label[labid][11]='\00';         // terminate string
			labty[labid] = dbuff[11];      // field type
			if (dbuff[16]&0x80)
				ifldsz = 128 + (int)(dbuff[16]&0x7F);  // unsigned field size
			else
				ifldsz = (int)dbuff[16];  // field size
			fldsz[labid] = ifldsz;
			flddp[labid] = (int)dbuff[17];  // decimal places
			recordsz += ifldsz;  // record size = sum of field sizes
			labid++;
			ichar = fgetc(dbffp);
		}
		fldcnt = labid;
		while (fread(dbuff, recordsz+1, 1, dbffp) > 0) {
			fldstart=1;
			for (fi=0; fi<fldcnt; fi++) {
				if (labty[fi]=='C')  {
					fldstart += fldsz[fi];
					continue;  // skip text fields for now.
				}
				strncpy(fldtxt,&dbuff[fldstart],fldsz[fi]);
				fldtxt[fldsz[fi]] = '\00';

				if (labty[fi]=='C') {
					if (fldtxt[0]==' ')
						fldtxt[1] = '\00';
					else {
						for (ti=fldsz[fi]-1; ti>0; ti--) {
							if (fldtxt[ti]==' ')  fldtxt[ti]='\00';
							else  break;
						}
					}
					if (!bQuery) printf(",%s",fldtxt);
					csdata = CString(fldtxt);
					csp = new CString(csdata);
					data[recordcnt][fi].txtptr = csp;
				}
				else  {  // numeric

				ti=0;
					while (fldtxt[ti]==' ') ti++;
					fdata = (float)atof(&fldtxt[ti]);
					dbfdata[recordcnt][fi] = fdata;
				<*** } // end of 'else { // numeric' ***>
				fldstart += fldsz[fi];
			}
			recordcnt++;
		}
		fclose(dbffp);
	}
	// <<< Create property arrays and fill data from dbfdata >>>
	int prop_count = 0;
	for (int fldindx=0; fldindx<fldcnt; fldindx++) {
		if (labty[fi]=='C')  continue;  // skip textual fields for now.
		fap =vtkFloatArray::New();
		fapLAP->InsertNextValue((long)fap);
		((vtkDataArray*)fap)->SetName(label[fldindx]);
		for (int recindx=0; recindx<shpih->nRecords; recindx++) {
			((vtkDataArray*)fap)->InsertTuple(recindx, &dbfdata[recindx][fldindx]);
		}
		prop_count++;
	}
*** END WORK-IN-PROGRESS ***/

  // Store the points and cells in the output data object.
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  output->SetPoints(myPointsPtr);
 
  if (filetype==POINTOBJ)
    output->SetVerts(myCellsPtr);
  else if (filetype==LINEOBJ)
    output->SetLines(myCellsPtr);
  else if (filetype==FACEOBJ) {
    output->SetPolys(faceCellsPtr);  // HGRD triangulated processing
  }
  else  // MESHOBJ
    output->SetPolys(faceCellsPtr);
/***     'WORK_IN_PROGRESS ***
	for (int pi3=0; pi3<prop_count; pi3++)
		output->GetCellData()->AddArray(((vtkDataArray*)fapLAP->GetValue(pi3)));
*** END 'WORK_IN_PROGRESS ***/

  for (int pi=0; pi<propcnt; pi++)
    {
    ((vtkFloatArray*)fapLAP->GetValue(pi))->Delete();
    }
		fapLAP->Delete();
  return 1;
}
