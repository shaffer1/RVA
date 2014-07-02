/*=========================================================================

Program:   RVA
Module:    ZGlobalScale

Original author and copyright: Visual Technology Services Ltd.
ZGlobalScale is provided under an open source license - see Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.
=========================================================================*/

/**
 * @file       zScalePlugin.cpp
 * @date       October 17th, 2011
 *
 * @brief      Defines class for plugin startup handlers in which are executed Python macros.
 */

// SYSTEM INCLUDES
//
#include <QApplication>
#include <QMessageBox>
#include <QToolBar>
#include <QMainWindow>
#include <QLayout>
#include <QAction>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QTimer>
#include <QItemSelectionModel>
#include <pqPipelineModel.h>
#include <pqCoreUtilities.h>
#include <pqActiveObjects.h>
#include <pqSMAdaptor.h>
#include <pqSetName.h>
#include <pqView.h>
#include <pqApplicationCore.h>
#include <pqDataRepresentation.h>
#include <pqViewManager.h>
#include <pqPipelineSource.h>
#include <pqApplicationCore.h>
#include <pqServerManagerModel.h>
#include <pqPipelineBrowserWidget.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMPVRepresentationProxy.h>
#include <vtkSMProxy.h>
#include <vtkAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkSmartPointer.h>
#include <vtkSMViewProxy.h>
#include <vtkView.h>
#include <vtkTransform.h>
#include <vtk3DWidgetRepresentation.h>

// LOCAL INCLUDES
//
#include "zGlobalScale.h"

// constructor
zGlobalScale::zGlobalScale(QObject* ptrParent): QObject(ptrParent)
{
    // create new double spin box
    mptrZScaleWidget = new QDoubleSpinBox;

    // initialize z scale widget
    mptrZScaleWidget->setToolTip("Set New Global Z Scale");
    mptrZScaleWidget->setRange(-10000.0, 10000.0);
    mptrZScaleWidget->setSingleStep(0.5);
    mptrZScaleWidget->setValue(1.0);
    mptrZScaleWidget->setDecimals(5);
    mptrZScaleWidget->setPrefix("x");
}

// destructor
zGlobalScale::~zGlobalScale()
{
}

// callback for startup
void zGlobalScale::onStartup()
{
    // register timer which will initialize plugin
    QTimer::singleShot(1000, this, SLOT(InitializePlugin()));

    // connects state load / save signals with appropriate handlers
    connect(pqApplicationCore::instance(), SIGNAL(stateSaved(vtkPVXMLElement*)), this, SLOT(SaveState(vtkPVXMLElement*)));
    connect(pqApplicationCore::instance(), SIGNAL(aboutToLoadState(vtkPVXMLElement*)), this, SLOT(LoadState(vtkPVXMLElement*)));
}

// callback for shutdown
void zGlobalScale::onShutdown()
{
    // disconnects state load / save signals from appropriate handlers
    disconnect(pqApplicationCore::instance(), SIGNAL(stateSaved(vtkPVXMLElement*)), this, SLOT(SaveState(vtkPVXMLElement*)));
    disconnect(pqApplicationCore::instance(), SIGNAL(aboutToLoadState(vtkPVXMLElement*)), this, SLOT(LoadState(vtkPVXMLElement*)));
}

// event handler called whenever Z Scale + toolbar button is pressed
void zGlobalScale::ZScalePlus()
{
    // set new global z scale value
    mptrZScaleWidget->setValue(mptrZScaleWidget->value() + 0.5);
}

// event handler called whenever Z Scale - toolbar button is pressed
void zGlobalScale::ZScaleMinus()
{
    // set new global z scale value
    mptrZScaleWidget->setValue(mptrZScaleWidget->value() - 0.5);
}

// event handler called whenever Z Scale Step value is changed
void zGlobalScale::ZScaleStepValue(double dValue)
{
    /*// NOTE: processes all pipeline objects, except for named exclusion filter, here hardcoded list.
    QString python_script = QString("exclude_list=[\"WellGlyph\"]\n") +
                            QString("proxies = servermanager.ProxyManager().GetProxiesInGroup(\"sources\")\n") +
                            QString("for proxy in proxies:\n") +
                            QString("    px_name=proxy[0]\n") +
                            QString("    source=FindSource(px_name)\n") +
                            QString("    dp=GetDisplayProperties(source)\n") +
                            QString("    exi=0\n") +
                            QString("    for ex in exclude_list:\n") +
                            QString("        exi += px_name.count(ex)\n") +
                            QString("    if exi==0:\n") +
                            QString("        dp.Scale = [dp.Scale[0], dp.Scale[1], ") + QString::number(dValue) + QString("]\n");

    // execute above script
    ExecuteScript(python_script.toAscii().data());*/

    int i;

    // get reference of parent window
    QMainWindow* parent_widget = (QMainWindow*)pqCoreUtilities::mainWidget();

     // get reference to application's object
    pqApplicationCore* application = pqApplicationCore::instance();

     // get reference to server manager model's object
    pqServerManagerModel* server_manager_model = application->getServerManagerModel();

    // get all sources
    QList<pqPipelineSource*> list = server_manager_model->findItems<pqPipelineSource*>();

    // for all selections
    for(i = 0; i < list.size(); i++)
    {
        // if selected item is pipeline source, and its not instance of PVTI_OffshoreWellGlyph nor PVTI_WellGlyph, process it
        pqPipelineSource* source = list.at(i);
        if(source != NULL && QString("PVTI_WellGlyph").compare(source->getProxy()->GetXMLName()) != 0 && QString("PVTI_OffshoreWellGlyph").compare(source->getProxy()->GetXMLName()) != 0)
        {
            // get representation
            pqDataRepresentation* representation = source->getRepresentation(pqActiveObjects::instance().activeView());
            if(representation == NULL)
                continue;

            // set new scale values
            QList<QVariant> scale_values;
            scale_values << 1.0 << 1.0 << mptrZScaleWidget->value();

            // set new property's value
            pqSMAdaptor::setMultipleElementProperty(representation->getProxy()->GetProperty("Scale"), scale_values);

            // update underlying VTK objects
            representation->getProxy()->UpdateVTKObjects();
        }
    }

    // get currently active view and reset its display (if available)
    pqView* active_view = pqActiveObjects::instance().activeView();
    if(active_view != NULL)
    {

    active_view->resetDisplay();
      vtkSMViewProxy * smactive_view = active_view->getViewProxy();
      vtkView * vactive_view = smactive_view->GetClientSideView();
      if(vactive_view)
      {
        int num_repr = vactive_view->GetNumberOfRepresentations();
        vtkTransform * transform = vtkTransform::New();
        transform->Scale(1,1,mptrZScaleWidget->value());
        for (int i=0; i<num_repr; i++)
        {
           vtkDataRepresentation * repr = vactive_view->GetRepresentation(i);
           vtk3DWidgetRepresentation *widget =
           vtk3DWidgetRepresentation::SafeDownCast(repr);
           if ( widget )
           {
            //if the world scale has changed while the widget is active,

            //remove and re add the transform to get the widget to be

            //transformed properly
            widget->SetCustomWidgetTransform(NULL);
            widget->SetCustomWidgetTransform(transform);
           }
        }
        transform->Delete();
      }
  }
}

// slot routine called whenever user saves state
void zGlobalScale::SaveState(vtkPVXMLElement* ptrPVXmlElement)
{
    bool updated_only = true;

    // allocate root element for storing pvti parameters
    vtkSmartPointer<vtkPVXMLElement> pvti_root = ptrPVXmlElement->FindNestedElementByName("PVTI");
    if(pvti_root.GetPointer() == NULL)
    {
        // create new pvti root element
        pvti_root = vtkSmartPointer<vtkPVXMLElement>::New();

        // set root's element name
        pvti_root->SetName("RVA_ZGLOBALSCALE");

        // new element created
        updated_only = false;
    }

    // allocate new element
    vtkSmartPointer<vtkPVXMLElement> element = vtkSmartPointer<vtkPVXMLElement>::New();

    // set element's name
    element->SetName("GlobalZScale");

    // set element's value
    element->SetAttribute("Value", QString::number(mptrZScaleWidget->value()).toAscii().data());

    // add element to root
    pvti_root->AddNestedElement(element);

    // adds root element
    if(!updated_only)
        ptrPVXmlElement->AddNestedElement(pvti_root);
}

// slot routine called whenever user loads state
void zGlobalScale::LoadState(vtkPVXMLElement* ptrPVXmlElement)
{
    // get root PVTI element
    vtkSmartPointer<vtkPVXMLElement> pvti_root = ptrPVXmlElement->FindNestedElementByName("RVA_ZGLOBALSCALE");
    if(pvti_root.GetPointer() == NULL)
        return;

    // for all nested elements, get them and store their value to QSettings
    unsigned int i;
    for(i = 0; i < pvti_root->GetNumberOfNestedElements(); i++)
    {
        // get nested element
        vtkSmartPointer<vtkPVXMLElement> element = pvti_root->GetNestedElement(i);
        if(element.GetPointer() == NULL || 0 != strcmp("GlobalZScale", element->GetName()))
            continue;

        // get global z scale's value
        double global_z_scale = atof(element->GetAttribute("Value"));

        // set element's value
        mptrZScaleWidget->setValue(global_z_scale);
    }
}

// initialize - add toolbar and associate actions with appropriate behavior
void zGlobalScale::InitializePlugin()
{
    // create items icon object
    QIcon plus_icon(":/RVA/Icons/zGlobalScalePlus16.png");
    QIcon minus_icon(":/RVA/Icons/zGlobalScaleMinus16.png");

    // get reference of parent window
    QMainWindow* parent_widget = (QMainWindow*)pqCoreUtilities::mainWidget();
    if(parent_widget == NULL)
        QTimer::singleShot(1000, this, SLOT(InitializePlugin()));
    else
    {
        // create new toolbar
        QToolBar* toolbar = new QToolBar("Global Z Scale", parent_widget);
        toolbar->setObjectName("globalZScaleToolbar");

        // add new actions / widgets to toolbar
        QAction* plus_action = toolbar->addAction(plus_icon, "Scale Scene Along Z Axis In Positive Direction");
        QAction* minus_action = toolbar->addAction(minus_icon, "Scale Scene Along Z Axis In Negative Direction");

        // connect actions with event handlers
        connect(plus_action, SIGNAL(triggered()), this, SLOT(ZScalePlus()));
        connect(minus_action, SIGNAL(triggered()), this, SLOT(ZScaleMinus()));

        // add widgets to created toolbar
        toolbar->addWidget(new QLabel("Global Z Scale: "));
        toolbar->addWidget(mptrZScaleWidget);

        // connect widget's signal with event handler
        connect(mptrZScaleWidget, SIGNAL(valueChanged(double)), this, SLOT(ZScaleStepValue(double)));

        // add new toolbar to parent window
        parent_widget->addToolBar(toolbar);
    }
}
