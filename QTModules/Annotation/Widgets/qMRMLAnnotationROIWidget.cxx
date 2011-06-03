/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLAnnotationROIWidget.h"
#include "ui_qMRMLAnnotationROIWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLAnnotationROINode.h>

// 0.001 because the sliders only handle 2 decimals
#define SLIDERS_EPSILON 0.001

// --------------------------------------------------------------------------
class qMRMLAnnotationROIWidgetPrivate: public Ui_qMRMLAnnotationROIWidget
{
  Q_DECLARE_PUBLIC(qMRMLAnnotationROIWidget);
protected:
  qMRMLAnnotationROIWidget* const q_ptr;
public:
  qMRMLAnnotationROIWidgetPrivate(qMRMLAnnotationROIWidget& object);
  void init();

  vtkMRMLAnnotationROINode* ROINode;
  bool IsProcessingOnMRMLNodeModified;
};

// --------------------------------------------------------------------------
qMRMLAnnotationROIWidgetPrivate::qMRMLAnnotationROIWidgetPrivate(qMRMLAnnotationROIWidget& object)
  : q_ptr(&object)
{
  this->ROINode = 0;
  this->IsProcessingOnMRMLNodeModified = false;
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidgetPrivate::init()
{
  Q_Q(qMRMLAnnotationROIWidget);
  this->setupUi(q);
  QObject::connect(this->DisplayClippingBoxButton, SIGNAL(toggled(bool)),
                   q, SLOT(setDisplayClippingBox(bool)));
  QObject::connect(this->InteractiveModeCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setInteractiveMode(bool)));
  QObject::connect(this->LRRangeWidget, SIGNAL(valuesChanged(double, double)),
                   q, SLOT(updateROI()));
  QObject::connect(this->PARangeWidget, SIGNAL(valuesChanged(double, double)),
                   q, SLOT(updateROI()));
  QObject::connect(this->ISRangeWidget, SIGNAL(valuesChanged(double, double)),
                   q, SLOT(updateROI()));
  q->setEnabled(this->ROINode != 0);
}

// --------------------------------------------------------------------------
// qMRMLAnnotationROIWidget methods

// --------------------------------------------------------------------------
qMRMLAnnotationROIWidget::qMRMLAnnotationROIWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLAnnotationROIWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationROIWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLAnnotationROIWidget::~qMRMLAnnotationROIWidget()
{
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationROINode* qMRMLAnnotationROIWidget::mrmlROINode()const
{
  Q_D(const qMRMLAnnotationROIWidget);
  return d->ROINode;
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setMRMLAnnotationROINode(vtkMRMLAnnotationROINode* roiNode)
{
  Q_D(qMRMLAnnotationROIWidget);
  qvtkReconnect(d->ROINode, roiNode, vtkCommand::ModifiedEvent,
                this, SLOT(onMRMLNodeModified()));

  d->ROINode = roiNode;
  this->onMRMLNodeModified();
  this->setEnabled(roiNode != 0);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setMRMLAnnotationROINode(vtkMRMLNode* roiNode)
{
  this->setMRMLAnnotationROINode(vtkMRMLAnnotationROINode::SafeDownCast(roiNode));
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::onMRMLNodeModified()
{
  Q_D(qMRMLAnnotationROIWidget);
  qDebug() << "qMRMLAnnotationROIWidget::onMRMLNodeModified";
  if (!d->ROINode)
    {
    return;
    }

  d->IsProcessingOnMRMLNodeModified = true;

  // Visibility
  d->DisplayClippingBoxButton->setChecked(d->ROINode->GetVisibility());

  // Interactive Mode
  bool interactive = d->ROINode->GetInteractiveMode();
  d->LRRangeWidget->setTracking(interactive);
  d->PARangeWidget->setTracking(interactive);
  d->ISRangeWidget->setTracking(interactive);
  d->InteractiveModeCheckBox->setChecked(interactive);

  // ROI
  double xyz[3];
  double rxyz[3];

  d->ROINode->GetXYZ(xyz);
  d->ROINode->GetRadiusXYZ(rxyz);

  double bounds[6];
  for (int i=0; i < 3; ++i)
    {
    bounds[i]   = xyz[i]-rxyz[i];
    bounds[3+i] = xyz[i]+rxyz[i];
    }
  d->LRRangeWidget->setValues(bounds[0], bounds[3]);
  d->PARangeWidget->setValues(bounds[1], bounds[4]);
  d->ISRangeWidget->setValues(bounds[2], bounds[5]);

  d->IsProcessingOnMRMLNodeModified = false;
  this->updateROI();
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setExtent(double min, double max)
{
  Q_D(qMRMLAnnotationROIWidget);
  d->LRRangeWidget->setRange(min, max);
  d->PARangeWidget->setRange(min, max);
  d->ISRangeWidget->setRange(min, max);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setDisplayClippingBox(bool visible)
{
  Q_D(qMRMLAnnotationROIWidget);
  d->ROINode->SetVisibility(visible);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::setInteractiveMode(bool interactive)
{
  Q_D(qMRMLAnnotationROIWidget);
  d->ROINode->SetInteractiveMode(interactive);
}

// --------------------------------------------------------------------------
void qMRMLAnnotationROIWidget::updateROI()
{
  Q_D(qMRMLAnnotationROIWidget);

  // Ignore the calls made in onMRMLNodeModified() (except the last) as it
  // would set the node in an inconsistent state.
  if (d->IsProcessingOnMRMLNodeModified)
    {
    return;
    }

  double bounds[6];
  d->LRRangeWidget->values(bounds[0],bounds[1]);
  d->PARangeWidget->values(bounds[2],bounds[3]);
  d->ISRangeWidget->values(bounds[4],bounds[5]);

  double xyz[3];
  xyz[0] = 0.5*(bounds[1]+bounds[0]);
  xyz[1] = 0.5*(bounds[3]+bounds[2]);
  xyz[2] = 0.5*(bounds[5]+bounds[4]);

  int wasModifying = d->ROINode->StartModify();

  double nodeXYZ[3];
  d->ROINode->GetXYZ(nodeXYZ);
  // Qt sliders truncates decimals, we don't want to change the node if it's
  // the same values but with only different unsignificant decimals.
  if (fabs(nodeXYZ[0] - xyz[0]) > SLIDERS_EPSILON ||
      fabs(nodeXYZ[1] - xyz[1]) > SLIDERS_EPSILON ||
      fabs(nodeXYZ[2] - xyz[2]) > SLIDERS_EPSILON)
    {
    d->ROINode->SetXYZ(xyz);
    }

  double rxyz[3];
  rxyz[0] = 0.5*(bounds[1]-bounds[0]);
  rxyz[1] = 0.5*(bounds[3]-bounds[2]);
  rxyz[2] = 0.5*(bounds[5]-bounds[4]);

  double nodeRXYZ[3];
  d->ROINode->GetRadiusXYZ(nodeRXYZ);
  // Qt sliders truncates decimals, we don't want to change the node if it's
  // the same values but with only different unsignificant decimals.
  if (fabs(nodeRXYZ[0] - rxyz[0]) > SLIDERS_EPSILON ||
      fabs(nodeRXYZ[1] - rxyz[1]) > SLIDERS_EPSILON ||
      fabs(nodeRXYZ[2] - rxyz[2]) > SLIDERS_EPSILON)
    {
    d->ROINode->SetRadiusXYZ(rxyz);
    }
  d->ROINode->EndModify(wasModifying);
}
