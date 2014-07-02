/*=========================================================================

Program:   RVA
Module:    ConnectedThreshold

Copyright (c) University of Illinois at Urbana-Champaign (UIUC).
Portions of this file are derived from Paraview source code.
Original Authors: L Angrave, J Li, D McWherter, R Reizner 

All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __UI_ConnectedThresholdGUIPanel_H
#define __UI_ConnectedThresholdGUIPanel_H

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpacerItem>

#include "pqDoubleRangeWidget.h"

QT_BEGIN_NAMESPACE

// This code was initially created by Qt's generator from a UI file
// but has been modified, so should only be edited directly now.
class Ui_ThresholdPanel
{
public:
  QGridLayout *gridLayout;
  QHBoxLayout *hboxLayout;
  QHBoxLayout *hboxLayout1;
  QHBoxLayout *hboxLayout2;
  QHBoxLayout *hboxLayout3;
  QHBoxLayout *hboxLayout4;
  QHBoxLayout* modeLayout;
  QSpacerItem *spacerItem;
  pqDoubleRangeWidget *ThresholdBetween_0;
  pqDoubleRangeWidget *ThresholdBetween_1;
  pqDoubleRangeWidget *ThresholdBetween_2;
  pqDoubleRangeWidget *ThresholdBetween_3;
  QLabel *label_1;
  QLabel *label_2;
  QLabel *label_3;
  QLabel *label_4;
  QLabel *label_5;
  QLabel *label_6;
  QComboBox *SelectInputScalars;
  QComboBox *SelectInputScalars2;
  QCheckBox *InsideOut;
  QCheckBox *InsideOut2;
  QLineEdit* ResultArrayName;
  QLabel* FieldLabel;
  QLabel* modeLabel;
  QComboBox* modeBox;

  void setupUi(QWidget *ThresholdPanel)
  {
    if (ThresholdPanel->objectName().isEmpty())
      ThresholdPanel->setObjectName(QString::fromUtf8("ThresholdPanel"));
    ThresholdPanel->resize(244, 302);
    gridLayout = new QGridLayout(ThresholdPanel);
#ifndef Q_OS_MAC
    gridLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    gridLayout->setContentsMargins(9, 9, 9, 9);
#endif
    // The spacer makes things nice and compact
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Field layout
    ResultArrayName = new QLineEdit(ThresholdPanel);
    ResultArrayName->setObjectName(QString::fromUtf8("ResultArrayName"));
    FieldLabel = new QLabel(ThresholdPanel);
    FieldLabel->setText("Result Array Name");

    // Inside out box
    InsideOut = new QCheckBox(ThresholdPanel);
    InsideOut->setObjectName(QString::fromUtf8("InsideOut"));
    InsideOut2 = new QCheckBox(ThresholdPanel);
    InsideOut2->setObjectName(QString::fromUtf8("InsideOut2"));

    hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout->setSpacing(6);
#endif
    hboxLayout->setContentsMargins(0, 0, 0, 0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    ThresholdBetween_1 = new pqDoubleRangeWidget(ThresholdPanel);
    ThresholdBetween_1->setObjectName(QString::fromUtf8("ThresholdBetween_1"));

    hboxLayout->addWidget(ThresholdBetween_1);

    hboxLayout3 = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout3->setSpacing(6);
#endif
    hboxLayout3->setContentsMargins(0, 0, 0, 0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    ThresholdBetween_3 = new pqDoubleRangeWidget(ThresholdPanel);
    ThresholdBetween_3->setObjectName(QString::fromUtf8("ThresholdBetween2_1"));

    hboxLayout3->addWidget(ThresholdBetween_3);

    label_4 = new QLabel(ThresholdPanel);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setWordWrap(true);

    label_3 = new QLabel(ThresholdPanel);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setWordWrap(true);

    label_6 = new QLabel(ThresholdPanel);
    label_6->setObjectName(QString::fromUtf8("label_6"));
    label_6->setWordWrap(true);

    label_5 = new QLabel(ThresholdPanel);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setWordWrap(true);

    hboxLayout1 = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout1->setSpacing(6);
#endif
    hboxLayout1->setContentsMargins(0, 0, 0, 0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    ThresholdBetween_0 = new pqDoubleRangeWidget(ThresholdPanel);
    ThresholdBetween_0->setObjectName(QString::fromUtf8("ThresholdBetween_0"));

    hboxLayout1->addWidget(ThresholdBetween_0);

    SelectInputScalars = new QComboBox(ThresholdPanel);
    SelectInputScalars->setObjectName(QString::fromUtf8("SelectInputScalars"));

    SelectInputScalars2 = new QComboBox(ThresholdPanel);
    SelectInputScalars2->setObjectName(QString::fromUtf8("SelectInputScalars2"));

    label_2 = new QLabel(ThresholdPanel);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    label_1 = new QLabel(ThresholdPanel);
    label_1->setObjectName(QString::fromUtf8("label_1"));

    hboxLayout4 = new QHBoxLayout;
    ThresholdBetween_2 = new pqDoubleRangeWidget(ThresholdPanel);
    ThresholdBetween_2->setObjectName(QString::fromUtf8("ThresholdBetween2_0"));
    hboxLayout4->addWidget(ThresholdBetween_2);

    // Mode GUI stuff
    modeLabel = new QLabel(ThresholdPanel);
    modeLabel->setObjectName(QString::fromUtf8("ModeLabel"));
    modeLayout = new QHBoxLayout;
    modeBox = new QComboBox(ThresholdPanel);
    modeBox->setObjectName(QString::fromUtf8("FilterMode"));
    modeLayout->addWidget(modeBox);

    // Arrange things all right here
    gridLayout->addWidget(label_2, 0, 0, 1, 1);
    gridLayout->addWidget(SelectInputScalars, 0, 1, 1, 1);

    gridLayout->addWidget(label_3, 1, 0, 1, 1);
    gridLayout->addLayout(hboxLayout1, 1, 1, 1, 1);

    gridLayout->addWidget(label_4, 2, 0, 1, 1);
    gridLayout->addLayout(hboxLayout, 2, 1, 1, 1);
    gridLayout->addWidget(InsideOut, 3, 0, 2, 2);

    // Skip row 4 to maintain proper spacing/visualizing

    gridLayout->addWidget(modeLabel, 5, 0, 1, 1);
    gridLayout->addLayout(modeLayout, 5, 1, 1, 1);

    gridLayout->addWidget(label_1, 6, 0, 1, 1);
    gridLayout->addWidget(SelectInputScalars2, 6, 1, 1, 1);

    gridLayout->addWidget(label_5, 7, 0, 1, 1);
    gridLayout->addLayout(hboxLayout4, 7, 1, 1, 1);

    gridLayout->addWidget(label_6, 8, 0, 1, 1);
    gridLayout->addLayout(hboxLayout3, 8, 1, 1, 1);

    gridLayout->addWidget(InsideOut2, 9, 0, 2, 2);   
    gridLayout->addWidget(FieldLabel, 11, 0, 1, 1);
    gridLayout->addWidget(ResultArrayName, 11, 1, 1, 1);


    // Skip row 11 to maintain proper spacing/visualizing

    gridLayout->addItem(spacerItem, 12, 0, 1, 2); // This keeps things tight (must be below the last item)

    retranslateUi(ThresholdPanel);

    QMetaObject::connectSlotsByName(ThresholdPanel);
  } // setupUi

  void retranslateUi(QWidget *ThresholdPanel)
  {
    ThresholdPanel->setWindowTitle(QApplication::translate("ThresholdPanel", "Form", 0, QApplication::UnicodeUTF8));
    InsideOut->setText(QApplication::translate("ThresholdPanel", "Exclude Range (Inverse)", 0, QApplication::UnicodeUTF8));
    InsideOut2->setText(QApplication::translate("ThresholdPanel", "Exclude Range (Inverse)", 0, QApplication::UnicodeUTF8));
    ResultArrayName->setText(QApplication::translate("ThresholdPanel", "Result Array Name", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("ThresholdPanel", "Upper Threshold 2", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("ThresholdPanel", "Lower Threshold 2", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("ThresholdPanel", "Upper Threshold 1", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("ThresholdPanel", "Lower Threshold 1", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("ThresholdPanel", "Scalars 1", 0, QApplication::UnicodeUTF8));
    label_1->setText(QApplication::translate("ThresholdPanel", "Scalars 2", 0, QApplication::UnicodeUTF8));
    modeLabel->setText(QApplication::translate("ThresholdPanel", "Filter Mode", 0, QApplication::UnicodeUTF8));
  } // retranslateUi

};

namespace Ui {
  class ThresholdPanel: public Ui_ThresholdPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // __UI_ConnectedThresholdGUIPanel_H
