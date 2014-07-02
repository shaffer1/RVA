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

#include "ConnectedThresholdWithCustomSourceGUIPanel.h"

#include <cassert>

#include <QComboBox>
#include <QLineEdit>
#include <QSlider>

#include "pqDoubleRangeWidget.h"
#include "pqSMAdaptor.h"
#include "vtkSMProperty.h"
#include "vtkSMProxy.h"
#include "ui_pqConnectedThresholdWithCustomSourcePanel.h"

class ConnectedThresholdWithCustomSourceGUIPanel::pqUI : public Ui::ThresholdPanel { };

ConnectedThresholdWithCustomSourceGUIPanel::ConnectedThresholdWithCustomSourceGUIPanel(pqProxy* pxy, QWidget* p) :
pqNamedObjectPanel(pxy, p)
{
  this->UI = new pqUI;
  this->UI->setupUi(this);

  this->linkServerManagerProperties();

  QObject::connect(this->UI->ThresholdBetween_0, SIGNAL(valueEdited(double)),
    this, SLOT(lowerChanged(double)));
  QObject::connect(this->UI->ThresholdBetween_1, SIGNAL(valueEdited(double)),
    this, SLOT(upperChanged(double)));
  QObject::connect(this->UI->ThresholdBetween_2, SIGNAL(valueEdited(double)),
    this, SLOT(lowerChanged(double)));
  QObject::connect(this->UI->ThresholdBetween_3, SIGNAL(valueEdited(double)),
    this, SLOT(upperChanged(double)));

  QObject::connect(this->findChild<QComboBox*>("SelectInputScalars"),
    SIGNAL(activated(int)), this, SLOT(variableChanged()),
    Qt::QueuedConnection);

  QObject::connect(this->findChild<QComboBox*>("SelectInputScalars2"),
    SIGNAL(activated(int)), this, SLOT(variableChanged()),
    Qt::QueuedConnection);

  //setup the proper tab order. This process requires us to find
  //the correct child widgets of the threshold objects. Without these
  //setting the tab breaks down
  QSlider *Slider0 = this->UI->ThresholdBetween_0->findChild<QSlider*>("Slider");
  QLineEdit *LineEdit0 = this->UI->ThresholdBetween_0->findChild<QLineEdit*>("LineEdit");

  QSlider *Slider1 = this->UI->ThresholdBetween_1->findChild<QSlider*>("Slider");
  QLineEdit *LineEdit1 = this->UI->ThresholdBetween_1->findChild<QLineEdit*>("LineEdit");

  QSlider *Slider3 = this->UI->ThresholdBetween_3->findChild<QSlider*>("Slider");
  QLineEdit *LineEdit3 = this->UI->ThresholdBetween_3->findChild<QLineEdit*>("LineEdit");

  QSlider *Slider2 = this->UI->ThresholdBetween_2->findChild<QSlider*>("Slider");
  QLineEdit *LineEdit2 = this->UI->ThresholdBetween_2->findChild<QLineEdit*>("LineEdit");


  //now setup the correct order
  QWidget::setTabOrder(this->UI->SelectInputScalars, Slider0);
  QWidget::setTabOrder(Slider0, LineEdit0);
  QWidget::setTabOrder(LineEdit0, Slider1);
  QWidget::setTabOrder(Slider1, LineEdit1);
  QWidget::setTabOrder(LineEdit1, this->UI->SelectInputScalars2);
  QWidget::setTabOrder(this->UI->SelectInputScalars2, Slider3);
  QWidget::setTabOrder(Slider2, LineEdit3);
  QWidget::setTabOrder(LineEdit2, Slider3);
  QWidget::setTabOrder(Slider3, LineEdit3);

}

ConnectedThresholdWithCustomSourceGUIPanel::~ConnectedThresholdWithCustomSourceGUIPanel()
{
  delete this->UI;
}

void ConnectedThresholdWithCustomSourceGUIPanel::lowerChanged(double val)
{
  pqDoubleRangeWidget* snd = dynamic_cast<pqDoubleRangeWidget*>(QObject::sender());

  assert(snd);

  if(snd->value() < val) {
    snd->setValue(val);
  }
}

void ConnectedThresholdWithCustomSourceGUIPanel::upperChanged(double val)
{
  pqDoubleRangeWidget* snd = dynamic_cast<pqDoubleRangeWidget*>(QObject::sender());

  assert(snd);

  if(snd->value() > val) {
    snd->setValue(val);
  }
}

void ConnectedThresholdWithCustomSourceGUIPanel::variableChanged()
{
  QObject* snd = QObject::sender();

  // Only these two should be calling this method
  assert(snd == this->UI->SelectInputScalars || snd == this->UI->SelectInputScalars2);

  // when the user changes the variable, adjust the ranges on the ThresholdBetween
  vtkSMProperty* prop = NULL;
  if(snd == this->UI->SelectInputScalars)
    prop = this->proxy()->GetProperty("ThresholdBetween");
  else if(snd == this->UI->SelectInputScalars2)
    prop = this->proxy()->GetProperty("ThresholdBetween2");

  assert(prop);

  QList<QVariant> range = pqSMAdaptor::getElementPropertyDomain(prop);

  if(range.size() == 2 && range[0].isValid() && range[1].isValid())
  {
    if(snd == this->UI->SelectInputScalars) {
      this->UI->ThresholdBetween_0->setValue(range[0].toDouble());
      this->UI->ThresholdBetween_1->setValue(range[1].toDouble());
    } else if(snd == this->UI->SelectInputScalars2) {
      // Always set these to maximum (i.e. "disable" them)
      this->UI->ThresholdBetween_2->setValue(range[1].toDouble());
      this->UI->ThresholdBetween_3->setValue(range[1].toDouble());
    }
  }
}
