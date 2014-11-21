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

#include <sstream>
#include <cassert>

#include "ISATISGUIPanel.h"
#include "ISATISReaderSource.h"

#include "vtkSMProperty.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkStdString.h"

#include "pqApplicationCore.h"
#include "pqSettings.h"

#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>


static vtkStdString toValue(QComboBox*box)
{
  assert(box);
  return box &&  box->currentIndex() >0 ? vtkStdString(box->currentText().toStdString().c_str()) : "";
}

static vtkStdString toValue(QLineEdit*edit)
{
  assert(edit);
  return vtkStdString(edit->text().toStdString().c_str());
}

static void updateComboFromStringArray(QComboBox* box, vtkStringArray* array)
{

  assert(box && array);
  int num = array->GetNumberOfValues();

  box->clear();
  box->setEnabled(num>0);

  if(num >0) {
    box->addItem("Select ...");
    for(int i=0; i<num; ++i)
      box->addItem(QString(array->GetValue(i)));
  }
}

static int vtkStdStringToInt(vtkStdString & string)
{
  int ret = 0;
  std::stringstream stream;
  stream << string;
  stream >> ret;
  return ret;
}

void ISATISGUIPanel::UpdateStudyCombo()
{
  vtkStringArray* studies = Reader->GetStudyNames();

  DirectoryListCombo->clear();
  DirectoryListCombo->setEnabled(false);
  FileListCombo->clear();
  FileListCombo->setEnabled(false);

  updateComboFromStringArray(StudyListCombo,studies);
}

// This method is called when the study is changed. We need to get the study and then populate the directory list
void ISATISGUIPanel::UpdateDirectoryCombo()
{
  vtkStdString study = toValue(StudyListCombo);
  assert(Reader && study);

  // Update
  vtkSMStringVectorProperty * studyprop = vtkSMStringVectorProperty::SafeDownCast(this->proxy()->GetProperty(" GTX Study"));
  studyprop->SetElement(0, study.c_str());

  vtkSMStringVectorProperty * getst = vtkSMStringVectorProperty::SafeDownCast(this->proxy()->GetProperty("GetGTXStudies"));

  vtkStringArray* directories = Reader->GetDirectoryNames();

  FileListCombo->clear();
  FileListCombo->setEnabled(false);

  updateComboFromStringArray(DirectoryListCombo,directories);
}

void ISATISGUIPanel::UpdateFileListCombo()
{
  assert(Reader && DirectoryListCombo && FileListCombo);
  vtkStdString directory = toValue(DirectoryListCombo);

  // Update
  vtkSMStringVectorProperty * dirprop = vtkSMStringVectorProperty::SafeDownCast(this->proxy()->GetProperty(" GTX Directory"));
  vtkSMStringVectorProperty * x = vtkSMStringVectorProperty::SafeDownCast(this->proxy()->GetProperty("GetFileNames"));
  dirprop->SetElement(0, directory.c_str());

  vtkStringArray* files = Reader->GetFileNames();
  updateComboFromStringArray(FileListCombo, files);
}

void ISATISGUIPanel::UpdatePreferences()
{
  assert(Reader); // grep: Currently dependent on Reader - this is not condusive to true server/client architecture

  pqApplicationCore * core = pqApplicationCore::instance();
  pqSettings * settings = core->settings();

  settings->setValue(QString("ISATISReader_GTXHost"), QVariant(Reader->GTXServerHost));
  settings->setValue(QString("ISATISReader_GTXPort"), QVariant(Reader->GTXServerPort));
  settings->setValue(QString("ISATISReader_GTXPath"), QVariant(Reader->GTXServerPath));
}

void ISATISGUIPanel::CheckPreferences()
{
  // grep: Currently dependent on Reader - this is not condusive to true server/client architecture
  assert(HostEdit && PortEdit && PathEdit && Reader);

  pqApplicationCore * core = pqApplicationCore::instance();
  pqSettings * settings = core->settings();

  // Retrieve preferences
  QVariant host(settings->value(QString("ISATISReader_GTXHost"), QVariant("")));
  QVariant port(settings->value(QString("ISATISReader_GTXPort"), QVariant(0)));
  QVariant path(settings->value(QString("ISATISReader_GTXPath"), QVariant("")));

  // Set appropriately
  Reader->GTXServerHost = host.toString().toStdString();
  Reader->GTXServerPort = port.toInt();
  Reader->GTXServerPath = path.toString().toStdString();

  // Update line-edit boxes with data
  HostEdit->setText(host.toString());
  PortEdit->setText(port.toString());
  PathEdit->setText(path.toString());
}

void ISATISGUIPanel::SetFile()
{
  vtkStdString name =  toValue( FileListCombo);

  // Update
  vtkSMStringVectorProperty * fileprop = vtkSMStringVectorProperty::SafeDownCast(this->proxy()->GetProperty(" GTX File Name"));
  fileprop->SetElement(0, name.c_str());

  Reader->SetupClient(); // Force the server to recognize the filename is set

  Reader->ServerStatus = ItemAvailable;

  // Update the whole pipeline
  this->proxy()->UpdateVTKObjects();
}

void ISATISGUIPanel::ForceReload()
{
  assert(HostEdit && PortEdit && PathEdit);
  vtkStdString host(toValue(HostEdit)), port(toValue(PortEdit)), path(toValue(PathEdit));
  Reader->SetGTXServerHost(host);
  Reader->SetGTXServerPath(path);
  Reader->SetGTXServerPort(vtkStdStringToInt(port));

  Reader->StatusMessage = "Attempting to connect...";
  UpdateStatusMessage();

  Reader->Connect(true);
  UpdateStudyCombo();
  UpdateStatusMessage();
  UpdatePreferences();
}

void ISATISGUIPanel::UpdateStatusMessage()
{
  assert(StatusLabel);
  StatusLabel->setText(QString(Reader->StatusMessage));
  QApplication::sendPostedEvents(); // Force the status text to update immediately
}

ISATISGUIPanel::ISATISGUIPanel(pqProxy* pxy, QWidget *q)
  : pqAutoGeneratedObjectPanel(pxy, q), Reader(0), HostEdit(0), PortEdit(0), PathEdit(0), StudyListCombo(0), DirectoryListCombo(0), FileListCombo(0),
    RefreshButton(0), StatusLabel(0)
{
  this->Reader = ISATISReaderSource::SafeDownCast(this->proxy()->GetClientSideObject());

  assert(this->Reader); // We should have an object

  // Update study list
  SetupTextBoxes();
  CheckPreferences();
  SetupComboBoxes();

  // Combo box signal/slots
  QObject::connect(StudyListCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateDirectoryCombo()));
  QObject::connect(DirectoryListCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateFileListCombo()));
  QObject::connect(FileListCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFile()));
  QObject::connect(LengthUnitsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(SetLengthUnit()));

  // Connect signals/slots to update - this removes any need for a "reconnect" button
  QObject::connect(HostEdit, SIGNAL(returnPressed()), this, SLOT(ForceReload()));
  QObject::connect(PortEdit, SIGNAL(returnPressed()), this, SLOT(ForceReload()));
  QObject::connect(PathEdit, SIGNAL(returnPressed()), this, SLOT(ForceReload()));

  StatusLabel = this->findChild<QLineEdit*>(" Placeholder");
  StatusLabel->setEnabled(0);
//  StatusLabel = new QLabel(this);
  StatusLabel->setToolTip("Server connection status");
//  StatusLabel->setWordWrap(true);


  RefreshButton = new QPushButton(QIcon(":/RVA/IsatisIcons/120px-RefreshIcon.png"), trUtf8("R&efresh"), this);
  RefreshButton->setToolTip(QString("Connect or reconnect to the specified GTX server"));
  RefreshButton->setStatusTip(QString("Connect or reconnect to the specified GTX server"));

  // Now we add the Status and Refresh button just above GTX Study. 
  // Note we are overwriting a placeholder property that we defined in sources.xml
  int idx = this->layout()->indexOf(PathEdit)-1;
  QGridLayout * gl = dynamic_cast<QGridLayout*>(this->layout());

  if(gl && idx>0) {
    gl->addWidget(RefreshButton,idx,0);
    gl->addWidget(StatusLabel ,idx,1);
  } else { // Paraview changed it's layout - just append our items (base class does not have addAt/insert method)
    this->layout()->addWidget(RefreshButton);
    this->layout()->addWidget(StatusLabel);
  }
  QObject::connect(RefreshButton, SIGNAL(pressed()), this, SLOT(ForceReload()));

  UpdateStatusMessage();

  // Perhaps UpdateSelfAndAllInputs instead?
  this->proxy()->GetProperty(" GTX Study")->SetImmediateUpdate(1);
  this->proxy()->GetProperty(" GTX Directory")->SetImmediateUpdate(1);
  this->proxy()->GetProperty(" GTX File Name")->SetImmediateUpdate(1);
}

ISATISGUIPanel::~ISATISGUIPanel()
{
  delete RefreshButton;
  delete StatusLabel;
  RefreshButton = 0;
  StatusLabel   = 0;
}

void ISATISGUIPanel::SetupTextBoxes()
{
  this->HostEdit = this->findChild<QLineEdit*>(" GTX Server Host");
  this->PortEdit = this->findChild<QLineEdit*>(" GTX Server Port");
  this->PathEdit = this->findChild<QLineEdit*>(" GTX Server Path");

  assert(HostEdit && PortEdit && PathEdit);
}

void ISATISGUIPanel::SetupComboBoxes()
{

  this->StudyListCombo = this->findChild<QComboBox*>(" GTX Study");
  this->DirectoryListCombo  = this->findChild<QComboBox*>(" GTX Directory");
  this->FileListCombo = this->findChild<QComboBox*>(" GTX File Name");
  this->LengthUnitsCombo =  this->findChild<QComboBox*>(" Length Units");
  assert(StudyListCombo && DirectoryListCombo && FileListCombo && this->LengthUnitsCombo);
  UpdateStudyCombo();

  InitializeLengthUnitsCombo();
}

#define LENGTH(x) ( sizeof(x) / sizeof( x[0] )  )

void ISATISGUIPanel::InitializeLengthUnitsCombo()
{
  // See
  const char* authorizedUnits[]= {
    "micr (Micron)","mm (Millimeter)","cm (Centimeter)","dm (Decimeter)","m (Meter)",
    "dam (Decameter)","hm (Hectometer)","km (Kilometer)",
    "in (Inch)","ft (Foot)","yd (Yard)","mile (Mile)","nmil (Nautical Mile)",
    "dakm (10 Kilometers)","hkm (100 Kilometers)","kkm (1000 Kilometers)"
  };


  assert(LengthUnitsCombo);

  LengthUnitsCombo->clear();
  LengthUnitsCombo->addItem(QString("Use variable settings"));
  for(int i=0; i< LENGTH(authorizedUnits); ++i)
    LengthUnitsCombo -> addItem( QString(authorizedUnits[i]));
}

void ISATISGUIPanel::SetLengthUnit()
{
  assert(LengthUnitsCombo);
  Reader->SetGTXLengthUnit( toValue(LengthUnitsCombo) );
}
