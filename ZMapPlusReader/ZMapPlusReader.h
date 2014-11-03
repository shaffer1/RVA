// ZMapPlus GRID file reader. 20141022
// Mark Van Moer, NCSA

#ifndef __ZMapPlusReader_h
#define __ZMapPlusReader_h

#include "vtkImageAlgorithm.h"
#include "vtkSmartPointer.h"
#include <string>

class VTK_EXPORT ZMapPlusReader : public vtkImageAlgorithm
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
   
    float NullValue;

    int NumberOfColumns;
    int NumberOfRows;
    float Xmin;
    float Xmax;
    float Ymin;
    float Ymax;    

    int pos; // for skipping header

    int ReadHeader();

    virtual int RequestData(vtkInformation*, vtkInformationVector **, vtkInformationVector *); 
    virtual int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

    

private:
    ZMapPlusReader(const ZMapPlusReader&); // Not implemented.
    void operator=(const ZMapPlusReader&); // Not implemented.
};

#endif
