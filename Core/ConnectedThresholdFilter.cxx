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
#include "ConnectedThresholdFilter.h"

#include <cassert>

#include "vtkCellData.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(ConnectedThresholdFilter);


//----------------------------------------------------------------------------
ConnectedThresholdFilter::ConnectedThresholdFilter() 
{
  this->SetDebug(1);    
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);

}

ConnectedThresholdFilter::~ConnectedThresholdFilter() 
{
}