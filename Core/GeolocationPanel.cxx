/**
 * Geolocation test
 */

#include "GeolocationPanel.h"

// Qt
#include <QBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QString>

// std
#include <cassert>

// ParaView
#include "pqActiveObjects.h"
#include "pqActiveView.h"
#include "pqApplicationCore.h"
#include "pqPipelineSource.h"
#include "pqServerManagerModel.h"

// VTK
#include "vtkAlgorithm.h"
#include "vtkCamera.h"
#include "vtkCellLocator.h"
#include "vtkDataSet.h"
#include "vtkGenericCell.h"
#include "vtkImageData.h"
#include "vtkPVCompositeRepresentation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataRepresentation.h"
#include "vtkRectilinearGrid.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkStructuredGrid.h"
#include "vtkTransform.h"

// Simple macros for names
#define XSPIN_NAME "XSpin"
#define YSPIN_NAME "YSpin"
#define ZSPIN_NAME "ZSpin"
#define XDBLSPIN_NAME "XDoubleSpin"
#define YDBLSPIN_NAME "YDoubleSpin"
#define ZDBLSPIN_NAME "ZDoubleSpin"

extern VTK_COMMON_EXPORT void vtkOutputWindowDisplayWarningText(const char*);

//-----------------------------------------------------------------------------
GeolocationPanel::GeolocationPanel(QObject* p)
  : QActionGroup(p), currentModel(-1), currentMethod(STRUCTURED), currView(NULL), 
  xVal(0), yVal(0), zVal(0),
  buttonBox(NULL), xPos(NULL), yPos(NULL), zPos(NULL), xAbsBox(NULL), yAbsBox(NULL), zAbsBox(NULL), xAbs(0), yAbs(0), zAbs(0),
  sCoordsBox(NULL), fBoundsBox(NULL), aBoundsBox(NULL)
{
}

GeolocationPanel::~GeolocationPanel()
{
}

void GeolocationPanel::openDialog()
{
  currView = pqActiveObjects::instance().activeView();
  if(!currView)
    return;

  int res = createDialog();

  if(res != QDialog::Accepted)
    return;
  
  updateCamera();
}

int GeolocationPanel::createDialog()
{
  int retval = -1;
  QDialog msgBox;

  // Popup settings
  msgBox.setFixedSize(300, 250);
  msgBox.setWindowTitle("RVA : Geolocation");

  QGridLayout * layout      = new QGridLayout(&msgBox);
  QBoxLayout * posBoxLayout = new QBoxLayout(QBoxLayout::TopToBottom);

  this->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal, &msgBox);
  QLabel * modelLbl = new QLabel("Object", &msgBox);
  QComboBox * modelIdx = new QComboBox(&msgBox);
  
  populateList(modelIdx);
  createGroupBoxes(msgBox, posBoxLayout);

  // Setup main layout
  layout->addWidget(modelLbl, 0, 0);
  layout->addWidget(modelIdx, 0, 1);
  layout->addLayout(posBoxLayout, 1, 1);
  layout->addWidget(buttonBox, 3, 1);

  // Signal/slots
  connect(buttonBox, SIGNAL(accepted()), &msgBox, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), &msgBox, SLOT(reject()));
  connect(modelIdx, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCurrentModel(int)));

  updateCurrentModel(currentModel); // To set bounding information

  // Disable this button if we cannot actually use it.
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(currentModel != -1 && currentMethod != NONE && modelIdx->isEnabled());

  retval = msgBox.exec();

  // no longer need this pointer.  Qt will delete this object for us when 'msgBox' is deleted
  this->buttonBox = NULL; 

  // Values important for relative coordinates
  xVal = xPos->value();
  yVal = yPos->value();
  zVal = zPos->value();

  // Values important for absolute coordinates
  xAbs = xAbsBox->value();
  yAbs = yAbsBox->value();
  zAbs = zAbsBox->value();

  // No longer need pointers to widgets created by createGroupBoxes
  this->xAbsBox=this->yAbsBox=this->zAbsBox=NULL;
  this->xPos=this->yPos=this->zPos=NULL;

  // Only item without parent, so needs explicit delete
  delete posBoxLayout; posBoxLayout = NULL;

  return retval;
}

void GeolocationPanel::populateList(QComboBox* modelIdx)
{
  // Remove any existing items
  modelIdx->clear();

  // Add model indices to our list
  int numItems = (int)getNumberOfDataObjects();
  this->currentModel = (numItems > 0) ? 0 : - 1; // 0 by default if we have items, else -1 means none available
  
  for(int i = 0 ; i < numItems ; ++i)
    modelIdx->addItem(QString(getName(i)));

  if(currentModel == -1) {
    modelIdx->addItem(QString("No datasets found"));
    currentModel = 0;
    modelIdx->setDisabled(true);
  }

  // Select the active object
  pqPipelineSource* active = pqActiveObjects::instance().activeSource();
 
  while(active != NULL && currentModel < numItems-1 && getPipelineSource(currentModel) != active) 
       currentModel++;
  
  modelIdx->setCurrentIndex(currentModel);
}

// Don't worry about deleting objects with parents; Qt should delete all children once the parent
// is destroyed.
void GeolocationPanel::createGroupBoxes(QDialog& msgBox, QLayout* posBoxLayout)
{
  // Structured Coordinates
  sCoordsBox = new QGroupBox(tr("Voxel Index"), &msgBox);
  QBoxLayout * sCoordsLayout = new QBoxLayout(QBoxLayout::LeftToRight, sCoordsBox);
  xPos = new QSpinBox(&msgBox);
  yPos = new QSpinBox(&msgBox);
  zPos = new QSpinBox(&msgBox);
  sCoordsLayout->addWidget(xPos);
  sCoordsLayout->addWidget(yPos);
  sCoordsLayout->addWidget(zPos);
  sCoordsBox->setCheckable(true);
  sCoordsBox->setChecked(true);
  currentMethod = STRUCTURED;
  sCoordsBox->setEnabled(currentModel != -1);
  posBoxLayout->addWidget(sCoordsBox);
  xPos->setAccessibleName(XSPIN_NAME);
  yPos->setAccessibleName(YSPIN_NAME);
  zPos->setAccessibleName(ZSPIN_NAME);

  // Fractional Bounds
  fBoundsBox = new QGroupBox(tr("Fractional Bounds"), &msgBox);
  QBoxLayout * fBoundsLayout = new QBoxLayout(QBoxLayout::LeftToRight, fBoundsBox);
  QDoubleSpinBox* xDbl = new QDoubleSpinBox(&msgBox);
  QDoubleSpinBox* yDbl = new QDoubleSpinBox(&msgBox);
  QDoubleSpinBox* zDbl = new QDoubleSpinBox(&msgBox);
  fBoundsLayout->addWidget(xDbl);
  fBoundsLayout->addWidget(yDbl);
  fBoundsLayout->addWidget(zDbl);
  fBoundsBox->setCheckable(true);
  fBoundsBox->setChecked(false);
  fBoundsBox->setEnabled(currentModel != -1);
  xDbl->setSingleStep(0.1f);
  yDbl->setSingleStep(0.1f);
  zDbl->setSingleStep(0.1f);
  xDbl->setMaximum(1.f);
  yDbl->setMaximum(1.f);
  zDbl->setMaximum(1.f);
  posBoxLayout->addWidget(fBoundsBox);
  xDbl->setAccessibleName(XDBLSPIN_NAME);
  yDbl->setAccessibleName(YDBLSPIN_NAME);
  zDbl->setAccessibleName(ZDBLSPIN_NAME);

  // Absolute bounds
  aBoundsBox = new QGroupBox(tr("Object XYZ Coordinates"));
  QBoxLayout * aBoundsLayout = new QBoxLayout(QBoxLayout::LeftToRight, aBoundsBox);
  xAbsBox = new QDoubleSpinBox(&msgBox);
  yAbsBox = new QDoubleSpinBox(&msgBox);
  zAbsBox = new QDoubleSpinBox(&msgBox);
  // Maybe use the current object's bounds? (would need to update when model changes)
  // Maybe set step to be 5% of the model's range

  aBoundsLayout->addWidget(xAbsBox);
  aBoundsLayout->addWidget(yAbsBox);
  aBoundsLayout->addWidget(zAbsBox);
  aBoundsBox->setCheckable(true);
  aBoundsBox->setChecked(false);
  aBoundsBox->setEnabled(currentModel != -1);
  posBoxLayout->addWidget(aBoundsBox);

  // Signals/slots
  connect(sCoordsBox, SIGNAL(clicked(bool)), this, SLOT(updateCurrentMethod(bool)));
  connect(fBoundsBox, SIGNAL(clicked(bool)), this, SLOT(updateCurrentMethod(bool)));
  connect(aBoundsBox, SIGNAL(clicked(bool)), this, SLOT(updateCurrentMethod(bool)));

  // Update value callbacks
  connect(xPos, SIGNAL(valueChanged(int)), this, SLOT(updateValues(int)));
  connect(yPos, SIGNAL(valueChanged(int)), this, SLOT(updateValues(int)));
  connect(zPos, SIGNAL(valueChanged(int)), this, SLOT(updateValues(int)));
  connect(xDbl, SIGNAL(valueChanged(double)), this, SLOT(updateValues(double)));
  connect(yDbl, SIGNAL(valueChanged(double)), this, SLOT(updateValues(double)));
  connect(zDbl, SIGNAL(valueChanged(double)), this, SLOT(updateValues(double)));
}

void GeolocationPanel::updateValues(int val)
{
  QObject* sender = QObject::sender();
  assert(sender);

  QSpinBox* box = dynamic_cast<QSpinBox*>(sender);
  assert(box);

  QWidget* widget = box->parentWidget();
  assert(widget);

  QWidget* parent = widget->parentWidget();
  assert(parent);

  // Find the appropriate boxes to update
  QDoubleSpinBox* xDbl = NULL;
  QDoubleSpinBox* yDbl = NULL;
  QDoubleSpinBox* zDbl = NULL;
  QList<QDoubleSpinBox*> list = parent->findChildren<QDoubleSpinBox*>();
  QList<QDoubleSpinBox*>::iterator it = list.begin();
  for(; it != list.end() && (xDbl == NULL || yDbl == NULL || zDbl == NULL) ; it++) {
    QString str((*it)->accessibleName());
    if(str.compare(XDBLSPIN_NAME) == 0)
      xDbl = *it;
    else if(str.compare(YDBLSPIN_NAME) == 0)
      yDbl = *it;
    else if(str.compare(ZDBLSPIN_NAME) == 0)
      zDbl = *it;
  }

  assert(box && xDbl && yDbl && zDbl);
  if(!box || !xDbl || !yDbl || !zDbl)
    return; //paranoid

  int value = box->value();
  int ext[6];

  bool validExtent = getExtent(ext);
  QString name(box->accessibleName());

  //Todo check if ext is valid
  // NOTE: We block signals in here to avoid misfiring and causing errors
  // Update fraction information based on structured coordinate integer value...
  if(name.compare(XSPIN_NAME) == 0) {
    xDbl->blockSignals(true);
    //ext[1] - ext[0] = number of cells in X dir
    //ext[1] - ext[0] -1 = highest oordinate value
    int divisor = ext[1] - ext[0] - 1; // 
    if(divisor > 0)
      xDbl->setValue((double)val / divisor);
    xDbl->blockSignals(false);
  } else if(name.compare(YSPIN_NAME) == 0) {
    yDbl->blockSignals(true);
    int divisor = ext[3] - ext[2] - 1;
    if(divisor > 0)
      yDbl->setValue((double)val / divisor);
    yDbl->blockSignals(false);
  } else if(name.compare(ZSPIN_NAME) == 0) {
    zDbl->blockSignals(true);
    int divisor = ext[5] - ext[4] - 1;
    if(divisor > 0)
      zDbl->setValue((double)val / divisor);
    zDbl->blockSignals(false);
  }
}

static int max(int a,int b) { return a < b ? b: a; }

void GeolocationPanel::updateValues(double val)
{
  QObject* sender = QObject::sender();
  assert(sender);

  QDoubleSpinBox* box = dynamic_cast<QDoubleSpinBox*>(sender);
  assert(box);

  QWidget* widget = box->parentWidget();
  assert(widget);

  QWidget* parent = widget->parentWidget();
  assert(parent);

  // Find the appropriate boxes to update
  QSpinBox* xPos = NULL;
  QSpinBox* yPos = NULL;
  QSpinBox* zPos = NULL;
  QList<QSpinBox*> list = parent->findChildren<QSpinBox*>();
  QList<QSpinBox*>::iterator it = list.begin();
  for(; it != list.end() && (xPos == NULL || yPos == NULL || zPos == NULL) ; it++) {
    QString str((*it)->accessibleName());
    if(str.compare(XSPIN_NAME) == 0)
      xPos = *it;
    else if(str.compare(YSPIN_NAME) == 0)
      yPos = *it;
    else if(str.compare(ZSPIN_NAME) == 0)
      zPos = *it;
  }

  assert(box && xPos && yPos && zPos);

  int value = box->value();
  int ext[6];

  getExtent(ext);
  QString name(box->accessibleName());

  // NOTE: Block signals to avoid misfiring and crash bugs
  if(name.compare(XDBLSPIN_NAME) == 0) {
    xPos->blockSignals(true);
    int numCellsX = max(ext[1] - ext[0] - 1,0); // number of cells in X dir
    xPos->setValue(val * numCellsX);
    xPos->blockSignals(false);
  } else if(name.compare(YDBLSPIN_NAME) == 0) {
    yPos->blockSignals(true);
    int numCellsY = max(ext[3] - ext[2] - 1,0);
    yPos->setValue(val * numCellsY);
    yPos->blockSignals(false);
  } else if(name.compare(ZDBLSPIN_NAME) == 0) {
    zPos->blockSignals(true);
    int numCellsZ = max(ext[5] - ext[4] - 1,0);
    zPos->setValue(val * numCellsZ);
    zPos->blockSignals(false);
  }
}

void GeolocationPanel::updateCurrentMethod(bool checked)
{
  // 'checked' simulates a radio button
  if(currentModel == -1)
    return; // This should never happen

  // Determine the caller
  QObject* sender = QObject::sender();
  assert(sender);

  bool sCoords = checked && (sender == sCoordsBox);
  bool fBounds = checked && (sender == fBoundsBox);
  bool aBounds = checked && (sender == aBoundsBox);

  sCoordsBox->setChecked(sCoords);
  fBoundsBox->setChecked(fBounds);
  aBoundsBox->setChecked(aBounds);

  if(sCoords)
    currentMethod = STRUCTURED;
  else if(fBounds)
    currentMethod = FRACTIONAL;
  else if(aBounds)
    currentMethod = ABSOLUTE;
  else
    currentMethod = NONE;

  // If we have no method, we cannot submit "Ok"
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(currentMethod != NONE);
}

unsigned GeolocationPanel::getNumberOfDataObjects() const
{
  pqApplicationCore* core = pqApplicationCore::instance();
  if(!core) return 0;
  pqServerManagerModel* sm = core->getServerManagerModel();
  return sm ? sm->getNumberOfItems<pqPipelineSource*>()  : 0;
}

pqPipelineSource* GeolocationPanel::getPipelineSource(int i) const
{
  pqApplicationCore* core = pqApplicationCore::instance();
  if(!core) return 0;
  pqServerManagerModel* sm = core->getServerManagerModel();
  assert(sm);
  return sm ? sm->getItemAtIndex<pqPipelineSource*>(i) : 0;
}

pqPipelineSource* GeolocationPanel::getCurrentPipelineSource() const
{
  if(currentModel == -1)
    return NULL;
  return getPipelineSource(currentModel);
}

pqDataRepresentation* GeolocationPanel::getCurrentRepresentation() const
{
  pqPipelineSource* src = getCurrentPipelineSource();

  if(src == NULL || currView == NULL)
    return NULL;

  return src->getRepresentation(currView);
}

vtkSMRepresentationProxy* GeolocationPanel::getCurrentRepProxy() const
{
  pqDataRepresentation* data = getCurrentRepresentation();

  if(data == NULL)
    return NULL;

  return vtkSMRepresentationProxy::SafeDownCast(data->getProxy());
}

vtkSMSourceProxy* GeolocationPanel::getCurrentProxy() const
{
  if(currentModel < 0)
    return NULL;

  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();
  assert(pm);
  if(!pm) return NULL;

  const char* name = getName((unsigned)currentModel);

  if(name == NULL)
    return NULL;

  return vtkSMSourceProxy::SafeDownCast(pm->GetProxy("sources", name));
}

void GeolocationPanel::getBounds(double bounds[]) const
{
  for(int i = 0 ; i < 6 ; ++i) bounds[i] = 0; // 0 out the array to begin with

  if(currentModel < 0 || currView == NULL)
    return;

  vtkSMSourceProxy* pxy = getCurrentProxy();
  if(pxy == NULL)
    return;

  vtkPVDataInformation* data = pxy->GetDataInformation();

  if(data == NULL)
    return;

  data->GetBounds(bounds);
}

int GeolocationPanel::getExtent(int ext[]) const
{
  for(int i = 0 ; i < 6 ; ++i) ext[i] = 0; // 0 out the array to begin with

  if(currentModel < 0 || currView == NULL)
    return 0;

  vtkSMSourceProxy* pxy = getCurrentProxy();
  if(pxy == NULL)
    return 0;

  vtkPVDataInformation* data = pxy->GetDataInformation();

  if(data == NULL)
    return 0;

  data->GetExtent(ext);
  return ext[0] <= ext[1] && ext[2] <= ext[3] && ext[4] <=ext[5];
}

void GeolocationPanel::updateCamera()
{
  if(!currView) return;

  vtkSMRenderViewProxy* rView = vtkSMRenderViewProxy::SafeDownCast(currView->getViewProxy());
  vtkCamera* camProxy = NULL;
  double coords[3];

  if(rView == NULL)
    return;

  vtkSMRepresentationProxy* dataProxy = getCurrentRepProxy();

  // Zoom to correct model for proper perspective
  if(currentMethod != ABSOLUTE) { // Maybe we shouldn't zoom for absolute coordinates?
    if(dataProxy == NULL)
      return;
    rView->ZoomTo(dataProxy);
  }

  double bds[6];
  getBounds(bds);
  if(!getCellCoordinates(coords))
    return; // could not get valid coordinates (e.g. no representation yet so structured coords cannot be converted)
 
  double xWidth = bds[1] - bds[0];
  double yWidth = bds[3] - bds[2];
  double zWidth = bds[5] - bds[4];
  bool validBounds = xWidth >=0 && yWidth >=0 && zWidth >=0;

  camProxy = rView->GetActiveCamera();
  if(camProxy && dataProxy) {
    // How to scale so camera is in proper view
    std::vector<double> scale = vtkSMPropertyHelper(dataProxy, "Scale").GetDoubleArray();
    camProxy->SetViewUp(0, 0, 1);
    if(validBounds) {
    int x = coords[0] - 2*xWidth*scale[0];
    int y = coords[1] - 2*yWidth*scale[1];
    int z = coords[2] + 2*zWidth*scale[2];
    camProxy->SetPosition(x, y, z);
    } else {
    camProxy->SetPosition(coords[0]-20, coords[1]-20, coords[2]+20);
    }
    camProxy->SetFocalPoint(coords);
  }
  
  // Make sure we update the view
  
  currView->forceRender();
  currView = NULL;
}

void GeolocationPanel::updateCurrentModel(int idx)
{
// if there are no objects then idx=1 and numofDOs returns 0...
  if(idx == -1 || (unsigned)idx >= this->getNumberOfDataObjects())
    return;

  int ext[6];
  currentModel = idx;

  // If "apply" has not been clicked then we do not know the extent yet
  bool validExtent = getExtent(ext);

  // NOTE: We are trusting the values of xPos, yPos, and zPos
  //    that means this method must NEVER be called outside of
  //    the createDialog() method.
  assert(xPos && yPos && zPos);
  xPos->setMinimum(0);
  yPos->setMinimum(0);
  zPos->setMinimum(0);
  // max-min+1 gives us the number of points
  // max-min is the number of cells
  // max-min-1 is the maximum cell index (for one dimension)
  if(validExtent) {
    xPos->setMaximum(max(0,ext[1] - ext[0] - 1));
    yPos->setMaximum(max(0,ext[3] - ext[2] - 1));
    zPos->setMaximum(max(0,ext[5] - ext[4] - 1));

    // Check if focal point in cell
    if(getCurrentRepProxy()) 
      findViewingCell();
  }
}

const char* GeolocationPanel::getName(unsigned i) const
{
  pqApplicationCore* core = pqApplicationCore::instance();
  if(!core) return "NONE";

  pqServerManagerModel* sm = core->getServerManagerModel();
  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();

  assert(pm && sm); // Something seriously wrong
  if(!pm || !sm) return "NONE";

  pqPipelineSource* src = sm->getItemAtIndex<pqPipelineSource*>(i);
  if(!src) return "NONE";
  return pm->GetProxyName("sources", src->getProxy());
}

vtkDataSet* GeolocationPanel::getObject(int i) const
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqServerManagerModel* sm = core->getServerManagerModel();

  assert(sm);

  if(currentModel < 0 || !sm)
    return NULL;

  pqPipelineSource* src = sm->getItemAtIndex<pqPipelineSource*>(i);
  vtkSMProxy* pxy = src->getProxy();
  assert(pxy);
  vtkAlgorithm* obj = vtkAlgorithm::SafeDownCast(pxy->GetClientSideObject());
  assert(obj);

  return obj ? vtkDataSet::SafeDownCast(obj->GetOutputDataObject(0)) : NULL;
}

// This reference may be helpeful later if we do not care about server-client
// but we may be able to get away with simply vtkSMRepresentationProxy*
// http://www.paraview.org/pipermail/paraview/2011-April/020878.html
int GeolocationPanel::getCellCoordinates(double coords[]) const
{
  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();
  // SECOND ATTEMPT:
  //  Pretend we live in a world with no servers... O_O
  // i.e. this will not work in client-server environment
  if(currentModel == -1)
    return 0;

    coords[0] = xAbs;
    coords[1] = yAbs;
    coords[2] = zAbs;

  if(currentMethod == ABSOLUTE)  // Absolute coordinates
    return 1;

  // NOTE: This is extremely round-about but eventually we get to our data object
  // paraview doesn't like us doing this arbitrarily, apparently - the object was hidden well
  vtkDataSet* data = getObject(currentModel);

  if(data == NULL)
    return 0;

  // Now to actually do the work.
  int ext[6];
  bool validExtent = getExtent(ext);
  
  if(!validExtent) {
    // not valid extent - just use coords as absolute coordinates
    return 0;
  }

  int xMax = ext[1] - ext[0]; // number of cells
  int yMax = ext[3] - ext[2];
  int zMax = ext[5] - ext[4];

  // Equation seems to be same for most (all?) data types from
  // http://www.itk.org/Wiki/ParaView/Users_Guide/VTK_Data_Model#Uniform_Rectilinear_Grid_.28Image_Data.29
  // idx_flat = k*(npts_x*npts_y) + j*nptr_x + i
  int cellId = (zVal*(xMax*yMax)) + (yVal*xMax) + xVal;
  
  vtkGenericCell* cell = vtkGenericCell::New();
  data->GetCell(cellId, cell);

  // Get cell center
  double cellBounds[6];
  cell->GetBounds(cellBounds);
  cell->Delete();

  // Get Cell center
  for(int i = 0 ; i < 3 ; ++i) {
    coords[i] = (cellBounds[2*i] + cellBounds[(2*i)+1]) / 2;
  }

  transformBetweenObjectAndAbsoluteCoords(coords,false);
  return 1;
}

void GeolocationPanel::findViewingCell()
{
  vtkDataSet* data = getObject(currentModel);
  vtkSMRenderViewProxy* rView = vtkSMRenderViewProxy::SafeDownCast(currView->getViewProxy());
  vtkCamera* cam = rView->GetActiveCamera();
  double focalPoint[3];
  int coords[3] = { 0, 0, 0 };
  double skip[3];

  assert(data && rView && cam);

  cam->GetFocalPoint(focalPoint);
  transformBetweenObjectAndAbsoluteCoords(focalPoint, true);

  vtkImageData* imgData = vtkImageData::SafeDownCast(data);
  vtkRectilinearGrid* gridData = vtkRectilinearGrid::SafeDownCast(data);
  vtkStructuredGrid* sGrid = vtkStructuredGrid::SafeDownCast(data);

  int* extent = NULL;
  if(imgData) {
    imgData->ComputeStructuredCoordinates(focalPoint, coords, skip);
    extent = imgData->GetExtent();
    assert(extent);
  } else if(gridData) {
    gridData->ComputeStructuredCoordinates(focalPoint, coords, skip);
    extent = gridData->GetExtent();
    assert(extent);
  } else if(sGrid) {
    vtkCellLocator* locator = vtkCellLocator::New();
    locator->SetDataSet(data);
    locator->BuildLocator();
    vtkIdType cellId = locator->FindCell(focalPoint);
    locator->Delete();

    extent = sGrid->GetExtent();
    assert(extent);

    int nx = extent[1] - extent[0];
    int ny = extent[3] - extent[2];
    int nz = extent[5] - extent[4];

    coords[0] = cellId % nx;
    coords[1] = (cellId / nx) % ny;
    coords[2] = cellId / nx / ny;
  }

  if(extent && sGrid == NULL) { // Unnecessary for structured grids
    coords[0] -= extent[0];
    coords[1] -= extent[2];
    coords[2] -= extent[4];
  }

  xPos->setValue(coords[0]);
  yPos->setValue(coords[1]);
  zPos->setValue(coords[2]);

}

// NOTE: Must free the result
// vtkTransform makes scaling and transformation both easy and fun!
// Buy yours now for only $999,999.99!
void GeolocationPanel::transformBetweenObjectAndAbsoluteCoords(double* coords, bool toObject) const
{
  vtkSMRepresentationProxy* proxy = getCurrentRepProxy();

  assert(proxy);
  assert(coords);

  std::vector<double> scale = vtkSMPropertyHelper(proxy, "Scale").GetDoubleArray();
  std::vector<double> orientation = vtkSMPropertyHelper(proxy, "Orientation").GetDoubleArray();
  std::vector<double> translation = vtkSMPropertyHelper(proxy, "Position").GetDoubleArray(); 

  assert(scale.size() == 3 && orientation.size() == 3 && translation.size() == 3);

  vtkTransform* transform = vtkTransform::New();
  transform->Translate(translation[0],translation[1],translation[2]);
  transform->RotateZ(orientation[2]);
  transform->RotateX(orientation[0]);
  transform->RotateY(orientation[1]);
  transform->Scale(scale[0],scale[1],scale[2]);
  if(toObject)
    transform->Inverse();

  // Don't worry about the memory - it appears to be freed elsewhere
  double* transformed = transform->TransformDoublePoint(coords);

  for(int i = 0 ; i < 3 ; ++i)
    coords[i] = transformed[i];

  transform->Delete();
}
