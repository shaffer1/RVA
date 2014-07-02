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

#ifndef _ConnectedThresholdGUIPanel_h
#define _ConnectedThresholdGUIPanel_h

#include "pqNamedObjectPanel.h"
#include "pqComponentsExport.h"
class pqDoubleRangeWidget;
//PQCOMPONENTS_EXPORT 
class ConnectedThresholdGUIPanel :
  public pqNamedObjectPanel
{
  Q_OBJECT
public:
  /// constructor
  ConnectedThresholdGUIPanel(pqProxy* proxy, QWidget* p = NULL);
  /// destructor
  virtual ~ConnectedThresholdGUIPanel();

protected slots:
  void lowerChanged(double);
  void upperChanged(double);
  void variableChanged();

protected:
  class pqUI;
  pqUI* UI;

};

#endif

