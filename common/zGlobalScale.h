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
 * @file       zScalePlugin.h
 * @date       October 17th, 2011
 *
 * @brief      Declares class for plugin startup / shutdown handlers in which are executed Python macros.
 */

#if !defined(PQ_PLUGIN_STARTER_H)
#define PQ_PLUGIN_STARTER_H

// SYSTEM INCLUDES
//
#include <QObject>
#include <vtkPVXMLElement.h>

// USED TYPES
//
class QDoubleSpinBox;

/**
 * @class zGlobalScale
 * @brief Defines startup / shutdown plugin handlers
 * 
 * Class encapsulates handlers called on plugin startup and shutdown (loading / unloading) in which are also executed Python macros.
 * 
 * @date October 17th, 2011
 * @version 1.0
 */
class zGlobalScale: public QObject
{

    Q_OBJECT

    //////////////////// PUBLIC ////////////////////

public:

    //==================== EVENT HANDLERS ====================

    /**
     * @brief Constructor
     * Routine is called at the beginning of object's lifecycle
     *
     * @param ptrParent Parent Qt object for plugin starter.
     */
    zGlobalScale(QObject* ptrParent = 0);

    /**
     * @brief Destructor
     * Routine is called at the end of object's lifecycle
     */
    ~zGlobalScale();

    //==================== EVENT HANDLERS ====================

    /**
     * @brief Startup callback
     * Routine is called automatically when plugin is loaded
     */
    void onStartup();

    /**
     * @brief Shutdown callback
     * Routine is called automatically when plugin is unloaded
     */
    void onShutdown();

    //////////////////// PRIVATE SLOTS ////////////////////

private slots:

    // initialize - add toolbar and associate actions with appropriate behavior
    void InitializePlugin();

    // event handler called whenever Z Scale + toolbar button is pressed
    void ZScalePlus();

    // event handler called whenever Z Scale - toolbar button is pressed
    void ZScaleMinus();

    // event handler called whenever Z Scale Step value is changed
    void ZScaleStepValue(double dValue);

    // slot routine called whenever user saves state
    void SaveState(vtkPVXMLElement* ptrPVXmlElement);

    // slot routine called whenever user loads state
    void LoadState(vtkPVXMLElement* ptrPVXmlElement);

    //////////////////// PRIVATE ////////////////////

private:

    //==================== HELPER OPERATIONS ====================

    // executes passed ParaView Python script
    void ExecuteScript(const char* ptrScript);

    //==================== PROPERTIES ====================

    // global z scale widget
    QDoubleSpinBox* mptrZScaleWidget;
};

#endif // PQ_PLUGIN_STARTER_H
