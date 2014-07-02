/*=========================================================================

Program:   RVA
Module:    ConnectedThresholdWithCustomSource

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Li, D McWherter, R Reizner

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef _ConnectedThresholdWithCustomSourceGUIPanel_h
#define _ConnectedThresholdWithCustomSourceGUIPanel_h

#include "pqNamedObjectPanel.h"
#include "pqComponentsExport.h"
class pqDoubleRangeWidget;
//PQCOMPONENTS_EXPORT 
class ConnectedThresholdWithCustomSourceGUIPanel :
  public pqNamedObjectPanel
{
  Q_OBJECT
public:
  /// constructor
  ConnectedThresholdWithCustomSourceGUIPanel(pqProxy* proxy, QWidget* p = NULL);
  /// destructor
  virtual ~ConnectedThresholdWithCustomSourceGUIPanel();

protected slots:
  void lowerChanged(double);
  void upperChanged(double);
  void variableChanged();

protected:
  class pqUI;
  pqUI* UI;

};

#endif

