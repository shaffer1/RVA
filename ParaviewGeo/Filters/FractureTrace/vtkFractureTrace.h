#ifndef _VTK_FRACTURE_TRACE_H
#define _VTK_FRACTURE_TRACE_H

#include "vtkPolyDataAlgorithm.h"
#include "vtkPolyData.h"

#include "vtkAppendPolyData.h"

class VTK_EXPORT vtkFractureTrace : public vtkPolyDataAlgorithm
{
public:
  static vtkFractureTrace *New();
  vtkTypeRevisionMacro(vtkFractureTrace,vtkPolyDataAlgorithm);


protected:
  vtkFractureTrace();
  ~vtkFractureTrace();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
	virtual int FillInputPortInformation(int port, vtkInformation *info);


private:

  vtkFractureTrace(const vtkFractureTrace&);
  void operator = (const vtkFractureTrace&);

  void computeFractureCenter(vtkIdType npts, const vtkIdType* pts, double* center);
  double computeNormalLength(vtkIdType npts, const vtkIdType* pts);
  int addArrow(const double* center, const double* V, double length, int prop);
  void createMatrix( const double *direction, const double *center, double matrix[16] );

  // finds the intersection between a line and a line segment. Returns false if there is no
  // such intersection.
  // pt: point on the line
  // V: direction of the line
  // p1: first point on the segment
  // p2: second point on the segment
  // t: The parameter t corresponding to the intersection point. This parameter is associated
  // with the parameteric equation of the line.
  bool line_segIntersection(const double* pt, const double* V, const double* p1, 
	  const double* p2, double& t);

  // find the two intersecting point between the line defined with the 
  // point pt and the direction V, and the segment of the cell defined by
	// npts and pts
  bool intersectWithCell(const double* pt, const double* V, 
	  vtkIdType npts, const vtkIdType* pts, double &t1, double &t2);


  vtkPolyData* input;
  vtkPoints* inPoints;
  vtkPolyData* output;
	vtkCellArray* outCells;
  vtkAppendPolyData* append;
};
#endif
