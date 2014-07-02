#ifndef _pqODBCReader_h
#define _pqODBCReader_h

#include "pqLoadedFormObjectPanel.h"
#include "pqComponentsExport.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkSmartPointer.h"

#include <QMap>
#include <QTableWidget>
#include <QDockWidget>

class QComboBox;
class vtkSMStringVectorProperty;

class pqODBCReader : public pqLoadedFormObjectPanel {
  Q_OBJECT
public:
  /// constructor
  pqODBCReader(pqProxy* proxy, QWidget* p = NULL);
  /// destructor
  ~pqODBCReader();

   virtual void accept();

	void restoreGuiState();
	void setComboBoxIndex(QComboBox* comboBox, QString &text);

	protected slots:
		void onTableSelectionChanged(int index);
		void onTableTypeSelectionChanged(int index);
		void updatePyPzIndexes(int);
		void onTableItemSelectionChanged( QTableWidgetItem*);
		void updateTable();
    
protected:
  /// populate widgets with properties from the server manager
  virtual void linkServerManagerProperties();

  QComboBox* cb_tables;
  QComboBox* cb_tablesType;
  QDockWidget* propDW;
  QDockWidget* paramDW;
  QWidget* psfWidget;

  QComboBox* Px;
  QComboBox* Py;
  QComboBox* Pz;

  QMap < QString, QTableWidget* > propTableWidgets; 

  QMap<QString, const char**> propsMap;
  vtkSMStringVectorProperty *serverSideProperties;
  vtkSmartPointer<vtkEventQtSlotConnect> QVTKConnect;
};

#endif

