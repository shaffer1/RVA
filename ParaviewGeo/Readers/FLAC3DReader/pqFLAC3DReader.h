#ifndef _pqFLAC3DReader_h
#define _pqFLAC3DReader_h

#include "pqLoadedFormObjectPanel.h"
#include "pqComponentsExport.h"

#include <QString>

class QLineEdit;

class pqFLAC3DReader : public pqLoadedFormObjectPanel {

	Q_OBJECT
public:
	/// constructor
	pqFLAC3DReader(pqProxy* proxy, QWidget* p = NULL);
	/// destructor
	~pqFLAC3DReader();

	virtual void accept();

	protected slots:
		void selectDispFile();
		void selectScalarsFile();
		void selectTensorsFile();   

protected:
	/// populate widgets with properties from the server manager
	virtual void linkServerManagerProperties();



	QLineEdit* dispFile;
	QLineEdit* scalarsFile;
	QLineEdit* tensorsFile;

	QString path;
};

#endif

