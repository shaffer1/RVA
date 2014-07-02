/*=========================================================================

Program:   RVA
Module:    Core

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Duggirala, D McWherter, U Yadav

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __Core_h
#define __Core_h

#include <QActionGroup>

#include "GeolocationPanel.h"

class CoreMenuActions : public QActionGroup {
  Q_OBJECT
public:
  CoreMenuActions(QObject* p);
  virtual ~CoreMenuActions();
  bool eventFilter(QObject * object, QEvent * event);
  inline void SetTitle();

public slots:
  void onAction(QAction* a);
  void openLink(const QString &k);
  void AboutBox();

private:
  // Other menu actions
  GeolocationPanel geoPanel;
};

#endif

