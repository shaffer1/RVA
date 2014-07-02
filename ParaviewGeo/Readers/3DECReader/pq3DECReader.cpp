#include "pq3DECReader.h"

#include <pqFileDialog.h>
#include <QToolButton>
#include <QLineEdit>

//________________________________________________________________________
pq3DECReader::pq3DECReader(pqProxy* pxy, QWidget* p)
: pqLoadedFormObjectPanel(":/PVGReaders/3DECReader/pq3DECReader.ui", pxy, p)
{
	this->pointsFile  = this->findChild<QLineEdit*>("PointsFile");
	this->dispFile = this->findChild<QLineEdit*>("DisplacementVectorsFile");
	this->scalarsFile = this->findChild<QLineEdit*>("ZonesScalarsFile");
	this->tensorsFile = this->findChild<QLineEdit*>("ZonesTensorsFile");

	QToolButton* browsePoints = this->findChild<QToolButton*>("BrowsePointsFile");
	QToolButton* browseDisp = this->findChild<QToolButton*>("BrowseDispFile");
	QToolButton* browseScalars = this->findChild<QToolButton*>("BrowseScalarsFile");
	QToolButton* browseTensors = this->findChild<QToolButton*>("BrowseTensorsFile");

	QObject::connect(browsePoints, SIGNAL(clicked()), this, SLOT(selectPointsFile()));
	QObject::connect(browseDisp, SIGNAL(clicked()), this, SLOT(selectDispFile()));
	QObject::connect(browseScalars, SIGNAL(clicked()), this, SLOT(selectScalarsFile()));
	QObject::connect(browseTensors, SIGNAL(clicked()), this, SLOT(selectTensorsFile()));

	this->linkServerManagerProperties();
}


//________________________________________________________________________
pq3DECReader::~pq3DECReader()
{
}


//________________________________________________________________________
void pq3DECReader::accept()
{
	pqLoadedFormObjectPanel::accept();
}


//________________________________________________________________________
void pq3DECReader::linkServerManagerProperties()
{
	// parent class hooks up some of our widgets in the ui
	pqLoadedFormObjectPanel::linkServerManagerProperties();
}


//________________________________________________________________________
void pq3DECReader::selectPointsFile()
{
	QString filters;
	filters += "Points File (*.txt)";

	pqFileDialog *fileDialog = new pqFileDialog(NULL,
		this, tr("Open Points File:"), QString(), filters);
	fileDialog->setObjectName("FileDialogOpenPoints");
	fileDialog->setFileMode(pqFileDialog::ExistingFile);

	if (fileDialog->exec() == QDialog::Accepted)
	{
		path = fileDialog->getSelectedFiles()[0];
	}

	if(path == QString())
		return;

	this->pointsFile->setText(path);
}

//________________________________________________________________________
void pq3DECReader::selectDispFile()
{
	QString filters;
	filters += "Displacement Vectors File (*.txt)";

	pqFileDialog *fileDialog = new pqFileDialog(NULL,
		this, tr("Open Displacement Vectors File:"), QString(), filters);
	fileDialog->setObjectName("FileDialogOpenDisp");
	fileDialog->setFileMode(pqFileDialog::ExistingFile);

	if (fileDialog->exec() == QDialog::Accepted)
	{
		path = fileDialog->getSelectedFiles()[0];
	}

	if(path == QString())
		return;

	this->dispFile->setText(path);
}

//________________________________________________________________________
void pq3DECReader::selectScalarsFile()
{
	QString filters;
	filters += "Zones Scalars File (*.txt)";

	pqFileDialog *fileDialog = new pqFileDialog(NULL,
		this, tr("Open Zones Scalars File:"), QString(), filters);
	fileDialog->setObjectName("FileDialogOpenScalars");
	fileDialog->setFileMode(pqFileDialog::ExistingFile);

	if (fileDialog->exec() == QDialog::Accepted)
	{
		path = fileDialog->getSelectedFiles()[0];
	}

	if(path == QString())
		return;

	this->scalarsFile->setText(path);
}


//________________________________________________________________________
void pq3DECReader::selectTensorsFile()
{
	QString filters;
	filters += "Zones Tensors File (*.txt)";

	
	pqFileDialog *fileDialog = new pqFileDialog(NULL,
		this, tr("Open Zones Tensors File:"), QString(), filters);
	fileDialog->setObjectName("FileDialogOpenTensors");
	fileDialog->setFileMode(pqFileDialog::ExistingFile);

	if (fileDialog->exec() == QDialog::Accepted)
	{
		path = fileDialog->getSelectedFiles()[0];
	}

	if(path == QString())
		return;

	this->tensorsFile->setText(path);
}