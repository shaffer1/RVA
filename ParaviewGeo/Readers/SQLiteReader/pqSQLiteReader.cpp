#include "pqSQLiteReader.h"
#include <QComboBox>
#include <QWidget>
#include <QtDebug>
#include <QString>
#include <QStringList>
#include <QCheckBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QMapIterator>

#include "vtkSMProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMRenderViewProxy.h"


#include "pqRenderView.h"
#include "pqActiveView.h"
#include "pqView.h"
#include "pqSMAdaptor.h"
#include "pqPropertyManager.h"
#include "pqNamedWidgets.h"

//************************* non member function
bool isNumeric(std::string value)
{
	if( (value.at(0)!='-') &! isdigit(value.at(0)) )
	{
		return false;
	}

	for(unsigned int i=1; i<value.length(); i++)
	{
		if( (!isdigit(value[i])) &! (value[i] == '.') )
		{
			return false;
		}
	}
	return true;
}
//***********************************************************


//________________________________________________________________________
pqSQLiteReader::pqSQLiteReader(pqProxy* pxy, QWidget* p)
: pqLoadedFormObjectPanel(":/PVGReaders/pqSQLiteReader.ui", pxy, p)
{
	this->cb_tables = this->findChild<QComboBox*>("ActiveTable");
	this->cb_tablesType = this->findChild<QComboBox*>("ActiveTableType");
	this->propDW = this->findChild<QDockWidget*>("dwProps");
	this->paramDW = this->findChild<QDockWidget*>("dwParams");

	this->psfWidget = this->findChild<QWidget*>("psfWidget");
	this->psfWidget->setVisible(false);

	this->Px = this->findChild<QComboBox*>("Px");
	this->Py = this->findChild<QComboBox*>("Py");
	this->Pz = this->findChild<QComboBox*>("Pz");

	vtkSMStringVectorProperty *headers = vtkSMStringVectorProperty::SafeDownCast(
		this->proxy()->GetProperty("Headers"));

	this->serverSideProperties = vtkSMStringVectorProperty::SafeDownCast(
		this->proxy()->GetProperty("Properties"));
	if(!serverSideProperties)
	{
		qDebug() << "\"Properties\" property is not found on the server";
	}

	QString line(headers->GetElement(0));
	QStringList lineSplit;

	lineSplit = line.split("|", QString::SkipEmptyParts);

	QFlags<Qt::ItemFlag> flg=Qt::ItemIsUserCheckable |
		Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	foreach(QString table, lineSplit)
	{
		QStringList tSplit = table.split("::", QString::SkipEmptyParts);

		QString tableName = tSplit[0];

		if(this->propTableWidgets.contains(tableName))
		{
			throw ("Tables must have different names");
		}

		this->cb_tables->addItem(tableName);

		QStringList fSplit = tSplit[1].split(",");
	
		QTableWidget* tableWidget = new QTableWidget();
		tableWidget->setColumnCount(3);
		tableWidget->setRowCount(fSplit.size());

		QTableWidgetItem *item = new QTableWidgetItem("Name");
		tableWidget->setHorizontalHeaderItem(0, item);

		item = new QTableWidgetItem("Type");
		tableWidget->setHorizontalHeaderItem(1, item);

		item = new QTableWidgetItem("To import");
		tableWidget->setHorizontalHeaderItem(2, item);

		tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
		tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
		
		const char** properties = new const char*[fSplit.size()*3];
		char* elem;

		int row = 0;
		foreach(QString field, fSplit)
		{
			QString prop = field.left(field.lastIndexOf("("));
			QString type = field.right(field.length()-1 - field.lastIndexOf("("));
			type = type.remove(type.length()-1, 1);

			if(type == "int" || type == "float" || type == "double")
			{
				this->Px->addItem(prop);
				this->Py->addItem(prop);
				this->Pz->addItem(prop);
			}

			elem = new char[prop.length()+1];

			int count;
			for(count=0; count<prop.length();++count)
				elem[count] = prop[count].toAscii();
			elem[count] = '\0';
			properties[row*3] = elem;

			elem = new char[type.length()+1];
			for(count=0; count<type.length();++count)
				elem[count] = type[count].toAscii();
			elem[count] = '\0';
			properties[row*3 + 1] = elem;

			properties[row*3 + 2] = "0";

			QTableWidgetItem *item = new QTableWidgetItem(prop);
			item->setTextAlignment(Qt::AlignCenter);
			tableWidget->setItem(row, 0, item);

			item =  new QTableWidgetItem(type);
			item->setTextAlignment(Qt::AlignCenter);
			tableWidget->setItem(row, 1, item);

			item =  new QTableWidgetItem("");
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(flg);
			item->setCheckState(Qt::Unchecked);
			tableWidget->setItem(row, 2, item);

			++row;
		}
		this->propsMap[tableName] = properties;
		this->propTableWidgets[tableName] = tableWidget;
	}

	this->onTableSelectionChanged(0);
	this->onTableTypeSelectionChanged(0);
	this->updatePyPzIndexes(this->Px->currentIndex());

	// This means that a state is being loaded. Since we are using
	// a fancy hacked gui, we need to restore the gui state by ourselves.
	if(this->serverSideProperties->GetNumberOfElements())
		this->restoreGuiState();

	QObject::connect(this->cb_tables, SIGNAL(currentIndexChanged(int)), 
		this, SLOT(onTableSelectionChanged(int)));

	QObject::connect(this->cb_tablesType, SIGNAL(currentIndexChanged(int)), 
		this, SLOT(onTableTypeSelectionChanged(int)));

	QObject::connect(this->Px, SIGNAL(currentIndexChanged(int)), 
		this, SLOT(updatePyPzIndexes(int)));


	QMapIterator<QString, QTableWidget*> it(this->propTableWidgets);
	while (it.hasNext()) 
	{
		it.next();
		QObject::connect(it.value(), SIGNAL(itemChanged ( QTableWidgetItem*)),
			this, SLOT(onTableItemSelectionChanged( QTableWidgetItem*)));
	}

	this->QVTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	this->QVTKConnect->Connect(this->serverSideProperties, vtkCommand::ModifiedEvent,
		this, SLOT(updateTable()));

	this->linkServerManagerProperties();
}


//________________________________________________________________________
pqSQLiteReader::~pqSQLiteReader()
{
	QMapIterator<QString, const char**> it(this->propsMap);
	while (it.hasNext()) 
	{
		it.next();

		// the char* allocated for the current table is currently used
		// by the server side vtkSMStringVectorProperty and will be freed
		// automatically
		if(it.key() == this->cb_tables->currentText())
			continue;

		int nProps = this->propTableWidgets[it.key()]->rowCount();

		for(int i=0; i<nProps*3; ++i)
		{
			delete[] it.value()[i];
		}
		
	}
}


//________________________________________________________________________
void pqSQLiteReader::accept()
{
	int nProps = 3*this->propTableWidgets[this->cb_tables->currentText()]->rowCount();

	this->QVTKConnect->Disconnect();
	this->serverSideProperties->SetElements(nProps, 
		this->propsMap[this->cb_tables->currentText()]);
	this->QVTKConnect->Connect(this->serverSideProperties, vtkCommand::ModifiedEvent,
		this, SLOT(updateTable()));

	pqLoadedFormObjectPanel::accept();
}


//________________________________________________________________________
void pqSQLiteReader::linkServerManagerProperties()
{
	// parent class hooks up some of our widgets in the ui
	pqLoadedFormObjectPanel::linkServerManagerProperties();
}

//________________________________________________________________________
void pqSQLiteReader::onTableSelectionChanged(int index)
{
	if(this->cb_tables->count() == 0)
		return;

	QString activeTable = this->cb_tables->currentText();
	if(this->propTableWidgets.contains(activeTable))
	{
		this->propDW->setWindowTitle(activeTable + " properties");
		this->propDW->setWidget(this->propTableWidgets[activeTable]);
	}
	else
	{
		this->propDW->setWindowTitle("properties");
		this->propDW->setWidget(NULL);
	}
}

//________________________________________________________________________
void pqSQLiteReader::onTableTypeSelectionChanged(int index)
{
	if(this->cb_tables->count() == 0)
		return;

	QString activeType = this->cb_tablesType->currentText();
	if(activeType == "Point Set")
	{
		QTableWidget* activeWidget = this->propTableWidgets[this->cb_tables->currentText()];
		for(int i=0; i<activeWidget->rowCount(); ++i)
		{
			QString prop = activeWidget->item(i, 0)->text();
			QString type = activeWidget->item(i, 1)->text();
			if(type == "int" || type == "float" || type == "double")
			{
				this->Px->addItem(prop);
				this->Py->addItem(prop);
				this->Pz->addItem(prop);
			}
		}
		this->paramDW->setWindowTitle("Point Set Parameters");
		this->paramDW->setWidget(this->psfWidget);
	}
	else
	{
		this->paramDW->setWidget(NULL);
		this->paramDW->setWindowTitle("Parameters");
	}
}


//________________________________________________________________________
void pqSQLiteReader::updatePyPzIndexes(int index)
{
	if(this->Py->count() > index)
	{
		this->Py->setCurrentIndex(index+1);
		if(this->Pz->count() > index + 1)
			this->Pz->setCurrentIndex(index+2);
		else
			this->Pz->setCurrentIndex(index+1);
	}
}

//________________________________________________________________________
void pqSQLiteReader::onTableItemSelectionChanged( QTableWidgetItem* item)
{
	int row = item->row();
	int column = item->column();
	if(column != 2)
		return;

	bool checked = item->checkState() == Qt::Checked;

	const char** properties = this->propsMap[this->cb_tables->currentText()];
	if(checked)
		properties[row*3 + column] = "1";
	else
		properties[row*3 + column] = "0";

	this->referenceProxy()->setModifiedState(pqProxy::MODIFIED);
}

//________________________________________________________________________
void pqSQLiteReader::updateTable()
{
	QTableWidget* activeTable = this->propTableWidgets[this->cb_tables->currentText()];
	activeTable->blockSignals(true);

	for( unsigned int i=0; i<this->serverSideProperties->GetNumberOfElements()/3; ++i )
	{
		if( strcmp(this->serverSideProperties->GetElement(i*3 + 2), "1") == 0 )
		{
			activeTable->item(i, 2)->setCheckState(Qt::Checked);		
		}
		else
		{
			activeTable->item(i, 2)->setCheckState(Qt::Unchecked);
		}
	}
	activeTable->blockSignals(false);
	pqLoadedFormObjectPanel::accept();
}


//________________________________________________________________________
void pqSQLiteReader::restoreGuiState()
{
	QString text;

	text = vtkSMStringVectorProperty::SafeDownCast(
		this->proxy()->GetProperty("ActiveTable"))->GetElement(0);
	this->setComboBoxIndex(this->cb_tables, text);

	text = vtkSMStringVectorProperty::SafeDownCast(
		this->proxy()->GetProperty("ActiveTableType"))->GetElement(0);
	this->setComboBoxIndex(this->cb_tablesType, text);

	text = vtkSMStringVectorProperty::SafeDownCast(
		this->proxy()->GetProperty("Px"))->GetElement(0);
	this->setComboBoxIndex(this->Px, text);

	text = vtkSMStringVectorProperty::SafeDownCast(
		this->proxy()->GetProperty("Py"))->GetElement(0);
	this->setComboBoxIndex(this->Py, text);

	text = vtkSMStringVectorProperty::SafeDownCast(
		this->proxy()->GetProperty("Pz"))->GetElement(0);
	this->setComboBoxIndex(this->Pz, text);

	QTableWidget* activeTable = this->propTableWidgets[this->cb_tables->currentText()];
	const char** props = this->propsMap[this->cb_tables->currentText()];
	for(unsigned int i=0; i<serverSideProperties->GetNumberOfElements(); i+=3)
	{
		if(strcmp(this->serverSideProperties->GetElement(i+2), "1")==0)
		{
			props[i+2] = "1";
			activeTable->item(i/3,2)->setCheckState(Qt::Checked);
		}
	}

	pqLoadedFormObjectPanel::accept();
}



//________________________________________________________________________
void pqSQLiteReader::setComboBoxIndex(QComboBox* comboBox, QString &text)
{
	for(int i=0; i<comboBox->count(); i++)
	{
		if(comboBox->itemText(i) == text)
		{
			comboBox->setCurrentIndex(i);
			return;
		}
	}
}