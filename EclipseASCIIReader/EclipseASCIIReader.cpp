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

#include <sstream>

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
        vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
    // MVM: Eclipse is an unstructured grid because the cells may be 'slipped', 
    // as along a fault, and not meet corner-to-corner. 
    // Therefore, setting the extent doesn't make sense. But this may need
    // to be revisited if parallel reading is desired.
    // There isn't any other useful meta data that can be set at this point, therefore
    // the very minimal function, a la $VTK_SRC/IO/Geometry/vtkProStarReader

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
    // eclipse2vtk.py layout
    // open file
    // loop line by line
    //     check for comments
    //     check for section header
    //        skip section or read section
    //
    //  most sections are just read into arrays, but ZCORN
    //  has to create the actual cells.

    ifstream grdecl(this->FileName);
    if (!grdecl) 
    {
        vtkErrorMacro("Error opening file " << this->FileName);
        return 0;
    }

    return 1;
}
