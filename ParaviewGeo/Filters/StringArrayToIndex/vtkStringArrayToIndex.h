#ifndef _VTK_STRING_ARRAY_TO_INDEX_H
#define _VTK_STRING_ARRAY_TO_INDEX_H

#include "vtkPolyDataAlgorithm.h"
#include "vtkDataSet.h"

class VTK_EXPORT vtkStringArrayToIndex : public vtkPolyDataAlgorithm
{
public:
  static vtkStringArrayToIndex *New();
  vtkTypeRevisionMacro(vtkStringArrayToIndex,vtkPolyDataAlgorithm);
  
  // Description:
  // Get/Set the string property to be 
  // changed to an index
  vtkSetStringMacro(StrProp);
  vtkGetStringMacro(StrProp);

  vtkSetStringMacro(IndexName);
  vtkGetStringMacro(IndexName);


protected:
  vtkStringArrayToIndex();
  ~vtkStringArrayToIndex();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);


private:
  vtkStringArrayToIndex(const vtkStringArrayToIndex&);
  void operator = (const vtkStringArrayToIndex&);
  
  char* StrProp;
  char* IndexName;
  vtkDataSet* input;
  vtkDataSet* output;

};
#endif
