// ZMapPlus GRID file reader. 20141022
// Mark Van Moer, NCSA

#ifndef __ZMapPlusReader_h
#define __ZMapPlusReader_h

#include "vtkImageAlgorithm.h"
#include "vtkSmartPointer.h"
#include <string>

class VTK_IO_EXPORT ZMapPlusReader : public vtkImageAlgorithm
{
public:
    static ZMapPlusReader *New();
    vtkTypeMacro(ZMapPlusReader, vtkImageAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

protected:
    ZMapPlusReader();
    ~ZMapPlusReader();
   
    char* FileName;

    virtual int RequestData(vtkInformation* request,
            vtkInformationVector** inputVector,
            vtkInformationVector* outputVector);

private:
    ZMapPlusReader(const ZMapPlusReader&); // Not implemented.
    void operator=(const ZMapPlusReader&); // Not implemented.
};

#endif
