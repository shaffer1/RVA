
#ifndef __vtkAnnotateVolume_h
#define __vtkAnnotateVolume_h

#include "vtkTableAlgorithm.h"


class VTK_EXPORT vtkAnnotateVolume : public vtkTableAlgorithm
{
  public:
    static vtkAnnotateVolume* New();
    vtkTypeRevisionMacro(vtkAnnotateVolume, vtkTableAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the text string to generate in the output.
    vtkSetStringMacro(Format);
    vtkGetStringMacro(Format);
    
  protected:
    vtkAnnotateVolume();
    ~vtkAnnotateVolume();

    
    virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
    virtual int FillInputPortInformation(int port, vtkInformation* info);
    char* Format;
    int    AllScalars;
    int    AttributeMode;
    int    ComponentMode;
    int    SelectedComponent;
  
  private:
    vtkAnnotateVolume(const vtkAnnotateVolume&); // Not implemented
    void operator=(const vtkAnnotateVolume&); // Not implemented
};

#endif

