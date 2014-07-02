/*
   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*=========================================================================
MIRARCO MINING INNOVATION
Author: Nehme Bilal (nehmebilal@gmail.com)
Description:
			This filter is used to compute the volume of an unstructured grid
			input. The input must have a property to identify each closed volume.
			Each closed volume is also assumed to be formed by tetrahedras.
			This filter is usually used togother with "Volumique Tetrahedralization"
			filter. The later will allow you to tetrahedralize a set of 3D non-intersecting 
			closed surfaces. If each closed surface is not identified by an id,
			"Volumique Tetrahedralization" filter can assign these id's if the corresponding
			check box is checked in the user interface.
===========================================================================*/

#ifndef __vtkComputeVolumes_h
#define __vtkComputeVolumes_h

#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPolyData.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkDoubleArray.h"


class VTK_EXPORT vtkComputeVolumes : public vtkUnstructuredGridAlgorithm
{
public:
	static vtkComputeVolumes *New(); 
  vtkTypeRevisionMacro(vtkComputeVolumes,vtkUnstructuredGridAlgorithm);

	void copyPoint(double *src, double *dest);
	double tetrahedronVolume(double *p1, double *p2, double *p3, double *p4); 

	char* GetVolumesArray()
	{
		return VolumesArray;
	}

protected:
  vtkComputeVolumes();
  ~vtkComputeVolumes();
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

  int computeVolumes(vtkUnstructuredGrid* input, vtkUnstructuredGrid* output);

	char *VolumesArray;

private:
  vtkComputeVolumes(const vtkComputeVolumes&);  // Not implemented.
  void operator=(const vtkComputeVolumes&);  // Not implemented.



};

#endif
