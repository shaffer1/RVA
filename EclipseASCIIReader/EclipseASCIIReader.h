// Eclipse ASCII grdecl reader. 20141210
// Mark Van Moer, NCSA

#ifndef __EclipseASCIIReader_h
#define __EclipseASCIIReader_h

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include <string>

class VTK_IO_EXPORT EclipseASCIIReader : public vtkUnstructuredGridAlgorithm
{
public:
    static EclipseASCIIReader *New();
    vtkTypeMacro(EclipseASCIIReader, vtkUnstructuredGridAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

protected:
    EclipseASCIIReader();
    ~EclipseASCIIReader();

    char* FileName;
    int filepos; // for skipping file sections.
    int ReadGrid(vtkUnstructuredGrid*);

    virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
    virtual int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

private:
    EclipseASCIIReader(const EclipseASCIIReader&); // Not implemented.
    void operator=(const EclipseASCIIReader&); // Not implemented.
};
#endif
