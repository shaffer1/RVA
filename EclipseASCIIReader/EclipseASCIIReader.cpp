// Eclipse ASCII grdecl file reader.
#include "EclipseASCIIReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataObject.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkHexahedron.h"

#include <string>
#include <sstream>
#include <valarray>

vtkStandardNewMacro(EclipseASCIIReader);

EclipseASCIIReader::EclipseASCIIReader()
{
    this->FileName = 0;
    this->SetNumberOfInputPorts(0);
    this->filepos = 0;
}

EclipseASCIIReader::~EclipseASCIIReader()
{
    this->SetFileName(0);
}

void EclipseASCIIReader::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
    os << indent << "FileName: "
        << (this->FileName ? this->FileName : "(none)") << "\n";
}

int EclipseASCIIReader::RequestInformation(vtkInformation* vtkNotUsed(request),
        vtkInformationVector** vtkNotUsed(inputVector), 
        vtkInformationVector* outputVector)
{
    // MVM: Eclipse is an unstructured grid because the cells may be 'slipped', 
    // as along a fault, and not meet corner-to-corner. 
    // Therefore, setting the extent doesn't make sense. But this may need
    // to be revisited if parallel reading is desired.
    // There isn't any other useful meta data that can be set at this point, 
    // therefore the very minimal function, 
    // a la $VTK_SRC/IO/Geometry/vtkProStarReader

    if (!this->FileName)
    {
        vtkErrorMacro(<< "A FileName must be specified.");
        return 0;
    }
    return 1;
}

int EclipseASCIIReader::RequestData(vtkInformation* vtkNotUsed(request),
        vtkInformationVector** vtkNotUsed(inputVector),
        vtkInformationVector* outputVector)
{
    if (!this->FileName)
    {
        vtkErrorMacro(<< "A FileName must be specified.");
        return 0;
    }

    vtkInformation *outInfo = outputVector->GetInformationObject(0);
    vtkUnstructuredGrid* output = vtkUnstructuredGrid::SafeDownCast(
            outInfo->Get(vtkDataObject::DATA_OBJECT()));

    //
    //file reading goes here, call methods
    //
    if (!this->ReadGrid(output)) 
    {
        return 0;
    }

    return 1;
}

int EclipseASCIIReader::ReadGrid(vtkUnstructuredGrid* output) 
{
    ifstream grdecl(this->FileName);
    if (!grdecl) 
    {
        vtkErrorMacro("Error opening file " << this->FileName);
        return 0;
    }
    vtkDebugMacro("Reading Eclipse ASCII grdecl file: " << this->FileName);

    std::string line;
    std::stringstream ss;

    int xdim, ydim, zdim;
    std::vector<float> xcoords;
    std::vector<float> ycoords;

    while (getline(grdecl,line)) 
    {
        // MVM: would really like these to be /^PINCH/ etc.
        if (line.find("PINCH")
                || line.find("MAPUNITS")
                || line.find("MAPAXES")
                || line.find("GRIDUNIT")
                || line.find("COORDSYS"))
        {
            // skip the following section 
            while (true) 
            {
                getline(grdecl,line);
                if (line.find("/")) 
                {
                    break;
                }
            }   
        } 
        else if (line.find("SPECGRID")) {
            // grab x,y,z dims
            // These are the cell-based dims, not
            // the point-based.
            getline(grdecl, line);
            ss.str(line);
            ss >> xdim >> ydim >> zdim;
            ss.clear();
            this->CreateVTKCells(output, xdim, ydim, zdim);
        }
        else if (line.find("COORD")) {
            // be sure this doesn't clash with COORDSYS
            // read coord section
            // grab specific values from it
            // in eclipse2vtk.py I then do some slicing, because
            // this is a weird section. use valarray?
            this->ReadCOORDSection(grdecl, xcoords, ycoords, xdim, ydim);
        }
        else if (line.find("ZCORN")) {
            // read zcorn section
            // create the ug cells and points
        }
        else if (line.find("ACTNUM")) {
            // read scalars
        }
        else if (line.find("EQLNUM")) {
        }
        else if (line.find("SATNUM")) {
        }
        else if (line.find("FIPNUM")) {
        }
        else if (line.find("PERMX")) {
        }
        else if (line.find("PERMY")) {
        }
        else if (line.find("PERMZ")) {
        }
        else if (line.find("PORO")) {
        }
        else {
            // unhandled comment, scalar type, or other section
        }
    }

    grdecl.close();
    return 1;
}

// COORDS is a weird section, could write a special
// method just for that
void EclipseASCIIReader::ReadCOORDSection(ifstream& f, 
        std::vector<float>& xcoords, std::vector<float>& ycoords, int xdim, int ydim)
{
    // COORDS is a sequence of x,y,z triples that describe the
    // top and bottom X-Y 'guidelines' of the grid. The Z's are never used in
    // the grid itself, those come from the ZCORN section.
    // The x's and y's are duplicated top and bottom. It appears that they
    // don't ever deviate from each other, but this has not been verified.
    // Therefore, for the purposes of VTK, only a particular subset of the
    // section is needed.
    //
    // x0 y0 ztop x0 y0 zbot x1 y0 ztop x1 y0 zbot x2 y0 ztop x2 y0 zbot
    // ...
    // xn ym ztop xn ym zbot /
    // 
    // We just need unique x0-xn and y0-ym, which we can get by pushing
    // the first n x's and every nth y.
    
    // Note: I don't think COORD will ever have compression. 
    std::string line;
    std::stringstream ss;
    float x, y, z;
    int i = 0;
    int j = 0;

    while (true) 
    {
        getline(f, line);
        ss.str(line);
        while (ss >> x >> y >> z) 
        {
            if (i < xdim) {
                xcoords.push_back(x);
                i++;
            }
            if (0 == xdim % j) {
                ycoords.push_back(y);
                j++;
            }
        } 
        if (line.find("/")) 
        {
            break;
        }
        ss.clear();
    }
    if (xcoords.size() != xdim || ycoords.size() != ydim) {
        vtkErrorMacro(<<"COORD vectors != dims\n");
    }
}

void EclipseASCIIReader::CreateVTKCells(vtkUnstructuredGrid* ug, int x, int y, int z)
{
    // Index pattern for zeroeth cell.
    int cellZeroPattern[] = {0, 1, 2*x, 2*x+1, 4*x*y, 4*x*y+1, 4*x*y+2*x, 4*x*y+2*x+1};

    for (int k = 0; k < z; k++) {
       for (int j = 0; j < y; j++) {
          for (int i = 0; i < x; i++) {
             vtkSmartPointer<vtkHexahedron> cell = vtkSmartPointer<vtkHexahedron>::New();
             std::vector<int> pattern;
             for (int n = 0; n < 8; n++) {
                pattern.push_back(n+(2*i)+(4*j*x)+(8*k*x*y));
             }
             // Note that VTK ordering doesn't match grdecl ordering.
             cell->GetPointIds()->SetId(0, pattern[0]);
             cell->GetPointIds()->SetId(1, pattern[1]);
             cell->GetPointIds()->SetId(2, pattern[3]);
             cell->GetPointIds()->SetId(3, pattern[2]);
             cell->GetPointIds()->SetId(4, pattern[4]);
             cell->GetPointIds()->SetId(5, pattern[5]);
             cell->GetPointIds()->SetId(6, pattern[7]);
             cell->GetPointIds()->SetId(7, pattern[6]);
             ug->InsertNextCell(cell->GetCellType(), cell->GetPointIds());
          }
       }
    } 
}
