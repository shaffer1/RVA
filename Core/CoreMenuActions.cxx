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

#include "RVA_Build.h"
#include "RVA_Util.h"
#include "CoreMenuActions.h"

#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqObjectBuilder.h"
#include "pqPythonDialog.h"
#include "pqPythonManager.h"
#include "pqPythonShell.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqUndoStack.h"
#include "vtkPVPluginsInformation.h"
#include "pqPluginManager.h"

#include <QActionGroup>
#include <QApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QStyle>
#include <QTabWidget>
#include <QUrl>
#include <QMessageBox>

#include <QMenuBar>
#include <QList>
#include <QToolBar>

#define ABOUT_INFO	\
"Reservoir Visualization and Analysis (RVA) is a DoE-funded (DE-FE0005961) project jointly developed by the \
<a href='http://www.isgs.illinois.edu'>Illinois State Geological Survey</a> and the <a href='http://www.cs.illinois.edu'> \
Computer Science Department</a> at the University of Illinois at Urbana-Champaign (UIUC). \
<br /><br />Project Geologists: D Keefer, J Damico. \
<br />Software Developers: L Angrave, J Duggirala, J Li, D McWherter, R Reizner, E Shaffer, U Yadav and R Yeh. \
<br />Contact Us: rva-support@cs.illinois.edu \
<br /><br />We acknowledge the kind support of <a href='http://www.geovariances.com/en#RVA'>Geovariances</a>. \
<br /><br />We acknowledge the support of Visual Technology Services Ltd. for their Z Global Scale contribution. \
<br /><br />We acknowledge the assistance and support of Prof. Mojdeh Delshad, Department of Petroleum and Geosystems Engineering, University of Texas at Austin. \
<br /><br />UTChem (<a href='http://www.cpge.utexas.edu/utchem/'>http://www.cpge.utexas.edu/utchem/</a>) and example data is copyright University of Texas at Austin. \
<br /><br />This plugin is Copyright © 2011-2012 University of Illinois at Urbana-Champaign (UIUC). Portions (Z scale functionality) copyright Visual Technology Services Ltd. \
All Rights Reserved. See Legal Information for details."

#define LEGAL_INFO \
  "Copyright © 2011-2012 University of Illinois at Urbana-Champaign. Portions (Z scale functionality) copyright Visual Technology Services Ltd. All rights reserved.<br /> \
<br /><br />Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: \
<br /><br />Redistributions of source code must retain the above copyright notice,this list of conditions and the following disclaimer. \
<br /><br />Redistributions in binary form must reproduce the above copyright notice,this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. \
<br /><br />Neither the name of the University of Illinois nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission. \
<br /><br />Modified source versions must be plainly marked as such, and must not be misrepresented as being the original software. \
<br /><br />THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. \
<br /><br /> \
<br /><br />Geovariances® is a registered trademark of Geovariances, France. All other trademarks and copyrights are the property of their respective owners. \
<br /><br />ParaViewGeo Copyright © 2012, Objectivity Inc. All rights reserved."

//-----------------------------------------------------------------------------
CoreMenuActions::CoreMenuActions(QObject* p) : QActionGroup(p), geoPanel(this)
{
  //bool rvaLoaded = false;

  /*
  QWidget* main = pqCoreUtilities::mainWidget();
  QMainWindow* mainWindow = static_cast<QMainWindow*>(main);

  QMenuBar* mbar = mainWindow->menuBar();
  QList<QMenu*>& menus = mbar->findChildren<QMenu*>();
  QList<QToolBar*> toolbars = mainWindow->findChildren<QToolBar*>();

  for(QList<QMenu*>::iterator it = menus.begin() ; it != menus.end() ; ++it) {
    QMenu* tmp = static_cast<QMenu*>(*it);
    QString title(tmp->title());
    if(title.compare("&RVA") == 0)
      rvaLoaded = true;
  }*/

  
  QIcon importIcon(":/RVA/Icons/120px-P_geology.png");
  QIcon aboutIcon(":/RVA/Icons/118px-Circle_question_mark.png");
  QAction * Separator;

  QAction * Geolocation = new QAction("&Geolocation", this);
  Geolocation->setData("OMIT");
  this->addAction(Geolocation);

  Separator = new QAction(this);
  Separator->setSeparator(true);
  
  QAction * Import = new QAction(importIcon, "&Import ISATIS Data", this);
  Import->setData("ISATISReaderSource");
  Import->setVisible(true);
  this->addAction(Import);

  Separator = new QAction(this);
  Separator->setSeparator(true);
  
  QAction* VisualizeMacro = new QAction("&Visualize Flow", this);
  VisualizeMacro->setData("PythonMacro");

  QAction*ContextViewMacro = new QAction("&Context View", this);
  ContextViewMacro->setData("PythonMacro");

  // Add a menu separator
  Separator = new QAction(this);
  Separator->setSeparator(true);

  QAction * About = new QAction(aboutIcon,"&About...", this);
  About->setData("OMIT");
  this->addAction(About);

  QObject::connect(About, SIGNAL(triggered(bool)), this, SLOT(AboutBox()));
  QObject::connect(Geolocation, SIGNAL(triggered(bool)), &geoPanel, SLOT(openDialog()));
  QObject::connect(this, SIGNAL(triggered(QAction*)), this, SLOT(onAction(QAction*)));
  
  SetTitle(); // Should work at any given loading time unless auto-loaded
  QApplication::instance()->installEventFilter(this);
}

//-----------------------------------------------------------------------------
CoreMenuActions::~CoreMenuActions()
{
}


//-----------------------------------------------------------------------------
void CoreMenuActions::onAction(QAction* a)
{
  if(a->data().toString().toStdString() == "OMIT")
    return;

  if(a->data().toString().toStdString() == "PythonMacro") {
    pqPythonManager* mgr = qobject_cast<pqPythonManager*>(pqApplicationCore::instance()->manager("PYTHON_MANAGER"));
    pqPythonDialog* dlg = mgr->pythonShellDialog();
    dlg->shell()->releaseControl();
    dlg->shell()->makeCurrent();

    if(a->text() == "&Visualize Flow") { // Tubify macro
      dlg->runString("from RVAMacros import Tubify");
      dlg->runString("Tubify.Tubify()");
    }

    if(a->text() == "&Context View") { // ContextView macro
      dlg->runString("from RVAMacros import ContextView");
      dlg->runString("ContextView.ContextView()");
    }

    dlg->shell()->releaseControl();
    return;
  }

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqServerManagerModel* sm = core->getServerManagerModel();
  pqUndoStack* stack = core->getUndoStack();

  vtkPVPluginsInformation * plugins = core->getPluginManager()->loadedExtensions(core->getActiveServer(),false);

  int numPlugins = plugins->GetNumberOfPlugins();
  bool ISATISFound = false;

  for (int i = 0; i < numPlugins; i++){
    std::string name (plugins->GetPluginName(i));
    if (name.compare("RVA_ISATIS_Plugin") == 0 && plugins->GetPluginLoaded(i)){
        ISATISFound = true;
    }
  }

  if (!ISATISFound){
    QMessageBox msgBox;
    msgBox.setText("To import ISATIS data, please load RVA_ISATIS_Plugin first!");
    msgBox.exec();
    return;
  }

  /// Check that we are connect to some server (either builtin or remote).
  if (sm->getNumberOfItems<pqServer*>()) {
    // just create it on the first server connection
    pqServer* s = sm->getItemAtIndex<pqServer*>(0);
    QString source_type = a->data().toString();
    // make this operation undo-able if undo is enabled
    if(stack) {
      stack->beginUndoSet(QString("Create %1").arg(source_type));
    }
    builder->createSource("sources", source_type.toAscii().data(), s);

    if(stack) {
      stack->endUndoSet();
    }
  }
}

//-----------------------------------------------------------------------------
void CoreMenuActions::AboutBox()
{

  QDialog msgBox;
  QWidget* Parent = msgBox.window();

  QGridLayout * AboutLayout  = new QGridLayout(Parent);
  QLabel * AboutImage        = new QLabel(Parent);
  QTabWidget * Tabwidget     = new QTabWidget(Parent);
  QWidget * Tab1             = new QWidget(Tabwidget);
  QWidget * Tab2             = new QWidget(Tabwidget);
  QGridLayout * Tab1Layout   = new QGridLayout(Tab1);
  QGridLayout * Tab2Layout   = new QGridLayout(Tab2);
  QScrollArea * Tab1scrollArea = new QScrollArea(Tab1);
  QScrollArea * Tab2scrollArea = new QScrollArea(Tab2);
  QLabel * Tab1Contents = new QLabel(Tab1scrollArea);
  QLabel * Tab2Contents = new QLabel(Tab2scrollArea);
  QDialogButtonBox * buttonBox    = new QDialogButtonBox(QDialogButtonBox::Ok,Qt::Horizontal,Parent);

  msgBox.setFixedSize(500,500);
  msgBox.setWindowTitle("RVA : About");

  QPixmap file(":/RVA/Icons/uiuc_logo.gif",0,0);
  AboutImage->setPixmap(file);
  AboutImage->setAlignment(Qt::AlignCenter);

  QFont font("Cursive",10,QFont::Normal);

  Tab1scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  Tab1scrollArea->setBackgroundRole(QPalette::Light);
  Tab1scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  Tab1scrollArea->setWidgetResizable(true);
  Tab1Contents->setText(ABOUT_INFO);
  Tab1Contents->setWordWrap(true);
  Tab1Contents->setFont(font);
  Tab1Contents->setMargin(5);
  Tab1Contents->setAlignment(Qt::AlignLeft);
  Tab1scrollArea->setWidget(Tab1Contents);
  Tab1Layout->addWidget(Tab1scrollArea);
  Tabwidget->addTab(Tab1,"General Information");
  Tab1->setLayout(Tab1Layout);

  Tab2scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  Tab2scrollArea->setBackgroundRole(QPalette::Light);
  Tab2scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  Tab2scrollArea->setWidgetResizable(true);
  Tab2Contents->setText(LEGAL_INFO);
  Tab2Contents->setWordWrap(true);
  Tab2Contents->setMargin(5);
  Tab2Contents->setFont(font);
  Tab2scrollArea->setWidget(Tab2Contents);
  Tab2Layout->addWidget(Tab2scrollArea);
  Tabwidget->addTab(Tab2,"Legal Information");
  Tab2->setLayout(Tab2Layout);

  connect(Tab1Contents, SIGNAL(linkActivated(const QString &)), this, SLOT(openLink(const QString &)));
  connect(buttonBox, SIGNAL(accepted()), &msgBox, SLOT(accept()));

  AboutLayout->addWidget(AboutImage);
  AboutLayout->addWidget(Tabwidget);
  AboutLayout->addWidget(buttonBox);
  msgBox.setLayout(AboutLayout);
  msgBox.exec();
}

//-----------------------------------------------------------------------------
void CoreMenuActions::openLink(const QString &k)
{
  QUrl q(k);
  QDesktopServices::openUrl(q);
}

//-----------------------------------------------------------------------------

bool CoreMenuActions::eventFilter(QObject * obj, QEvent* event)
{
  if(event->type() == QEvent::WindowTitleChange && obj == pqCoreUtilities::mainWidget()) {
    SetTitle();
    return false;
  }
  return QActionGroup::eventFilter(obj, event);
}

void CoreMenuActions::SetTitle()
{
  QWidget * mainWindow = pqCoreUtilities::mainWidget();

  if(mainWindow == NULL)
    return;

  QString title(mainWindow->windowTitle());

  if(title.contains("RVA")) // Avoid an infinite loop
    return;

  title.append(" - RVA @ UIUC (build ");
  title.append(RVA_BUILD_NUMBER);
  title.append(")");

  mainWindow->setWindowTitle(title);
}

