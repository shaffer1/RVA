/**
 * Geolocation test
 */

#ifndef __GEOLOCATIONPANEL_H__
#define __GEOLOCATIONPANEL_H__

#include <QActionGroup>
#include <QString>
#include <map>

class QDialogButtonBox;
class QSpinBox;
class QDoubleSpinBox;
class QGroupBox;
class QLayout;
class QComboBox;

class pqDataRepresentation;
class pqPipelineSource;
class pqView;
class vtkDataSet;
class vtkSMRepresentationProxy;
class vtkSMSourceProxy;
class vtkTransform;

class GeolocationPanel : public QActionGroup
{
  Q_OBJECT
public:
  GeolocationPanel(QObject* p);
  ~GeolocationPanel();

private slots:
  void openDialog();
  void updateCurrentModel(int);
  void updateCurrentMethod(bool);
  void updateValues(int);
  void updateValues(double);

private:
  enum Method { NONE = -1, STRUCTURED = 1, FRACTIONAL = 2, ABSOLUTE = 3 };

  // GUI methods
  int createDialog();
  void createGroupBoxes(QDialog&, QLayout*);
  void populateList(QComboBox*);

  // View methods
  void updateCamera();

  // Internal data methods
  unsigned getNumberOfDataObjects() const;
  void getBounds(double bounds[]) const;
  int getExtent(int ext[]) const;
  int getCellCoordinates(double coords[]) const; //0(Fail) 1(success)
  pqPipelineSource* getCurrentPipelineSource() const;
  pqPipelineSource* getPipelineSource(int i) const;
  pqDataRepresentation* getCurrentRepresentation() const;
  vtkSMSourceProxy* getCurrentProxy() const;
  vtkSMRepresentationProxy* getCurrentRepProxy() const;
  QString getObjectName(int i) const;
  const char* getName(unsigned i) const;
  vtkDataSet* getObject(int i) const;
  void findViewingCell();
  void transformBetweenObjectAndAbsoluteCoords(double* xyz, bool toObject) const;

  int currentModel;
  Method currentMethod;
  double xVal, yVal, zVal;
  QDialogButtonBox * buttonBox;
  QSpinBox * xPos, * yPos, * zPos;
  QDoubleSpinBox * xAbsBox, * yAbsBox, * zAbsBox;
  double xAbs, yAbs, zAbs;
  QGroupBox * sCoordsBox, * fBoundsBox, * aBoundsBox;
  pqView* currView;
};

#endif /* __GEOLOCATIONPANEL_H__ */
