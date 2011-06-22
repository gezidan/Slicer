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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// VTK includes
#include <vtkSmartPointer.h>

// qMRMLWidgets includes
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceControllerWidget.h>

// SlicerQt includes
#include "qSlicerSliceControllerModuleWidget.h"
#include "ui_qSlicerSliceControllerModule.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLNode.h"

// STL include
#include <map>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtcoremodules.qSlicerSliceControllerWidget");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerSliceControllerModuleWidgetPrivate:
    public Ui_qSlicerSliceControllerModule
{
public:
  void createSliceController(vtkMRMLSliceNode *sn, qSlicerLayoutManager *lm);
  void removeSliceController(vtkMRMLSliceNode *sn);

protected:
  typedef std::map<vtkSmartPointer<vtkMRMLSliceNode>, qMRMLSliceControllerWidget* > ControllerMapType;
  ControllerMapType ControllerMap;
};

//-----------------------------------------------------------------------------
void 
qSlicerSliceControllerModuleWidgetPrivate::createSliceController(vtkMRMLSliceNode *sn, qSlicerLayoutManager *layoutManager)
{
  if (this->ControllerMap.find(sn) != this->ControllerMap.end())
    {
    logger.trace(QString("createSliceController - SliceNode already added to module"));
    return;
    }

  // create the SliceControllerWidget and wire it to the appropriate
  // slice node
  qMRMLSliceControllerWidget *widget = new qMRMLSliceControllerWidget(CTKCollapsibleButton);
  widget->setSliceViewName( sn->GetName() ); // call before setting slice node
  widget->setMRMLSliceNode( sn );

  qMRMLSliceWidget *sliceWidget = layoutManager->sliceWidget(sn->GetLayoutName());
  widget->setSliceLogics(layoutManager->mrmlSliceLogics());
  widget->setSliceLogic(sliceWidget->sliceController()->sliceLogic());

  // add the widget to the display
  SliceControllersLayout->addWidget(widget);

  // cache the widget. we'll clean this up on the NodeRemovedEvent
  this->ControllerMap[sn] = widget;
}

//-----------------------------------------------------------------------------
void 
qSlicerSliceControllerModuleWidgetPrivate::removeSliceController(vtkMRMLSliceNode *sn)
{
  // find the widget for the SliceNode
  ControllerMapType::iterator cit = this->ControllerMap.find(sn);
  if (cit == this->ControllerMap.end())
    {
    logger.trace(QString("removeSliceController - SliceNode has no SliceController managed by this module."));
    return;
    }

  // unpack the widget
  SliceControllersLayout->removeWidget((*cit).second);

  // delete the widget
  delete (*cit).second;

  // remove entry from the map
  this->ControllerMap.erase(cit);
}



//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidget::qSlicerSliceControllerModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qSlicerSliceControllerModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidget::~qSlicerSliceControllerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setup()
{
  Q_D(qSlicerSliceControllerModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerSliceControllerModuleWidget);

  vtkMRMLScene* oldScene = this->mrmlScene();

  this->Superclass::setMRMLScene(newScene);

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  // Search the scene for the available SliceWidgets and create a
  // SliceController and connect it up
  newScene->InitTraversal();
  for (vtkMRMLNode *sn = NULL; (sn=newScene->GetNextNodeByClass("vtkMRMLSliceNode"));)
    {
    vtkMRMLSliceNode *snode = vtkMRMLSliceNode::SafeDownCast(sn);
    if (snode)
      {
      d->createSliceController(snode, layoutManager);
      }
    }

  // Need to listen for any new slice nodes being added
  this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeAddedEvent, 
                      this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));

  // Need to listen for any slice nodes being removed
  this->qvtkReconnect(oldScene, newScene, vtkMRMLScene::NodeRemovedEvent, 
                      this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
}

// --------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerSliceControllerModuleWidget);

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    logger.trace(QString("onSliceNodeAddedEvent - layoutName: %1").arg(layoutName));

    // create the slice controller
    d->createSliceController(sliceNode, layoutManager);
    }
}

// --------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerSliceControllerModuleWidget);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (sliceNode)
    {
    QString layoutName = sliceNode->GetLayoutName();
    logger.trace(QString("onSliceNodeRemovedEvent - layoutName: %1").arg(layoutName));
                                             
    // destroy the slice controller
    d->removeSliceController(sliceNode);
    }
}
