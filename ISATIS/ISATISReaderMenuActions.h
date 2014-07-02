/*=========================================================================

Program:   RVA
Module:    ISATISReader

Copyright (c) University of Illinois at Urbana-Champaign (UIUC)
Original Authors: L Angrave, J Duggirala, D McWherter, U Yadav

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __ISATISReaderMenuActions_h
#define __ISATISReaderMenuActions_h

#include <QActionGroup>

class ISATISReaderMenuActions : public QActionGroup {
  Q_OBJECT
public:
  ISATISReaderMenuActions(QObject* p);
  virtual ~ISATISReaderMenuActions();
  bool eventFilter(QObject * object, QEvent * event);
  inline void SetTitle();

public slots:
  void onAction(QAction* a);
  void openLink(const QString &k);
  void AboutBox();
};

#endif

