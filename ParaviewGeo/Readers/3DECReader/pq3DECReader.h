#ifndef _pq3DECReader_h
#define _pq3DECReader_h

#include "pqLoadedFormObjectPanel.h"
#include "pqComponentsExport.h"

#include <QString>

class QLineEdit;

class pq3DECReader : public pqLoadedFormObjectPanel {

	Q_OBJECT
public:
	/// constructor
	pq3DECReader(pqProxy* proxy, QWidget* p = NULL);
	/// destructor
	~pq3DECReader();

	virtual void accept();

	protected slots:
		void selectPointsFile();
		void selectDispFile();
		void selectScalarsFile();
		void selectTensorsFile();   

protected:
	/// populate widgets with properties from the server manager
	virtual void linkServerManagerProperties();



	QLineEdit* pointsFile;
	QLineEdit* dispFile;
	QLineEdit* scalarsFile;
	QLineEdit* tensorsFile;

	QString path;
};

#endif

