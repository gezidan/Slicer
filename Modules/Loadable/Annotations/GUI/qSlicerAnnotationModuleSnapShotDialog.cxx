// QT includes
#include <QVariant>

// QSlicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// AnnotationsWidgets includes
#include "qSlicerAnnotationModuleSnapShotDialog.h"

// AnnotationLogics includes
#include "Logic/vtkSlicerAnnotationModuleLogic.h"

// VTK includes
#include <vtkImageData.h>

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog
::qSlicerAnnotationModuleSnapShotDialog(QWidget* parentWidget)
  :Superclass(parentWidget)
{
  this->m_Logic = 0;
  this->setLayoutManager(qSlicerApplication::application()->layoutManager());
  this->setWindowTitle("Annotation Screenshot");
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleSnapShotDialog::~qSlicerAnnotationModuleSnapShotDialog()
{
  if (this->m_Logic)
    {
    this->m_Logic = 0;
    }
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("setLogic: We need the Annotation module logic here!");
    return;
    }

  this->m_Logic = logic;
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::loadNode(const char* nodeId)
{
  if (!this->m_Logic || !nodeId)
    {
    qErrnoWarning("initialize: We need a logic and a valid node here!");
    return;
    }

  // Activate the mode "review"
  this->setData(QVariant(nodeId));

  // get the name..
  vtkStdString name = this->m_Logic->GetAnnotationName(nodeId);

  // ..and set it in the GUI
  this->setNameEdit(QString::fromStdString(name));

  // get the description..
  vtkStdString description = this->m_Logic->GetSnapShotDescription(nodeId);
  // ..and set it in the GUI
  this->setDescription(QString::fromStdString(description));

  // get the screenshot type..
  int screenshotType = this->m_Logic->GetSnapShotScreenshotType(nodeId);

  // ..and set it in the GUI
  this->setWidgetType((qMRMLScreenShotDialog::WidgetType)screenshotType);

  double scaleFactor = this->m_Logic->GetSnapShotScaleFactor(nodeId);
  this->setScaleFactor(scaleFactor);

  vtkImageData* imageData = this->m_Logic->GetSnapShotScreenshot(nodeId);
  this->setImageData(imageData);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::reset()
{
  QString name("Screenshot");
  // check to see if it's an already used name for a node (redrawing the
  // dialog causes it to reset and calling GetUniqueNameByString increments
  // the number each time).
  vtkCollection *col =
    this->m_Logic->GetMRMLScene()->GetNodesByName(name.toLatin1());
  if (col->GetNumberOfItems() > 0)
    {
    // get a new unique name
    name = this->m_Logic->GetMRMLScene()->GetUniqueNameByString(name.toLatin1());
    }

  this->resetDialog();
  this->setNameEdit(name);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleSnapShotDialog::accept()
{
  // name
  QString name = this->nameEdit();
  QByteArray nameBytes = name.toLatin1();

  // description
  QString description = this->description();
  QByteArray descriptionBytes = description.toLatin1();

  // we need to know of which type the screenshot is
  int screenshotType = static_cast<int>(this->widgetType());

  if (this->data().toString().isEmpty())
    {
    // this is a new snapshot
    this->m_Logic->CreateSnapShot(nameBytes.data(),
                                  descriptionBytes.data(),
                                  screenshotType,
                                  this->scaleFactor(),
                                  this->imageData());
    }
  else
    {
    // this snapshot already exists
    this->m_Logic->ModifySnapShot(vtkStdString(this->data().toString().toLatin1()),
                                  nameBytes.data(),
                                  descriptionBytes.data(),
                                  screenshotType,
                                  this->scaleFactor(),
                                  this->imageData());
    }
  this->Superclass::accept();
}
