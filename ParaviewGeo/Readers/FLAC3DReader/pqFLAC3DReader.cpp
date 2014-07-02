#include "pqFLAC3DReader.h"

#include <pqFileDialog.h>
#include <QToolButton>
#include <QLineEdit>

//________________________________________________________________________
pqFLAC3DReader::pqFLAC3DReader(pqProxy* pxy, QWidget* p)
: pqLoadedFormObjectPanel(":/PVGReaders/FLAC3DReader/pqFLAC3DReader.ui", pxy, p)
{
	this->dispFile = this->findChild<QLineEdit*>("DisplacementVectorsFile");
	this->scalarsFile = this->findChild<QLineEdit*>("ZonesScalarsFile");
	this->tensorsFile = this->findChild<QLineEdit*>("ZonesTensorsFile");

	QToolButton* browseDisp = this->findChild<QToolButton*>("BrowseDispFile");
	QToolButton* browseScalars = this->findChild<QToolButton*>("BrowseScalarsFile");
	QToolButton* browseTensors = this->findChild<QToolButton*>("BrowseTensorsFile");

	QObject::connect(browseDisp, SIGNAL(clicked()), this, SLOT(selectDispFile()));
	QObject::connect(browseScalars, SIGNAL(clicked()), this, SLOT(selectScalarsFile()));
	QObject::connect(browseTensors, SIGNAL(clicked()), this, SLOT(selectTensorsFile()));

	this->linkServerManagerProperties();
}


//________________________________________________________________________
pqFLAC3DReader::~pqFLAC3DReader()
{
}


//________________________________________________________________________
void pqFLAC3DReader::accept()
{
	pqLoadedFormObjectPanel::accept();
}


//________________________________________________________________________
void pqFLAC3DReader::linkServerManagerProperties()
{
	// parent class hooks up some of our widgets in the ui
	pqLoadedFormObjectPanel::linkServerManagerProperties();
}



//________________________________________________________________________
void pqFLAC3DReader::selectDispFile()
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
void pqFLAC3DReader::selectScalarsFile()
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
void pqFLAC3DReader::selectTensorsFile()
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