/*=========================================================================

Program:   RVA
Module:    ConnectedThreshold

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Li, D McWherter, R Reizner

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __ConnectedThresholdFilter_h
#define __ConnectedThresholdFilter_h

#include "vtkDataSetAlgorithm.h"
#include "vtkRectilinearGrid.h"
#include "vtkStdString.h"
#include "ConnectedThresholdWithCustomSourceFilter.h"
#include <vector>
#include <utility>

class vtkPolyData;

class ConnectedThresholdFilter : public ConnectedThresholdWithCustomSourceFilter
{
public:


  static ConnectedThresholdFilter *New();
  vtkTypeMacro(ConnectedThresholdFilter,ConnectedThresholdWithCustomSourceFilter);

protected: 
  ConnectedThresholdFilter();
  virtual ~ConnectedThresholdFilter();

private:
  ConnectedThresholdFilter(const ConnectedThresholdFilter&);  // Not implemented.
  void operator=(const ConnectedThresholdFilter&);  // Not implemented.

};


#endif


