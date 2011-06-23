/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
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
#include "qMRMLSceneTransformModel.h"
#include "qMRMLSceneModel_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLTransformableNode.h>

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneTransformModel::parentNode(vtkMRMLNode* node)const
{
  // MRML Transformable nodes
  vtkMRMLTransformableNode* transformableNode =
    vtkMRMLTransformableNode::SafeDownCast(node);
  if (transformableNode)
    {
    return transformableNode->GetParentTransformNode();
    }
  return 0;
}

/*
//------------------------------------------------------------------------------
int qMRMLSceneTransformModel::nodeIndex(vtkMRMLNode* node)const
{
  const char* nodeId = node ? node->GetID() : 0;
  if (nodeId == 0)
    {
    return -1;
    }
  const char* nId = 0;
  int index = -1;
  vtkMRMLNode* parent = qMRMLSceneTransformModel::parentNode(node);
  vtkCollection* sceneCollection = node->GetScene()->GetCurrentScene();
  vtkMRMLNode* n = 0;
  vtkCollectionSimpleIterator it;
  for (sceneCollection->InitTraversal(it);
       (n = (vtkMRMLNode*)sceneCollection->GetNextItemAsObject(it)) ;)
    {
    // note: parent can be NULL, it means that the scene is the parent
    if (parent == qMRMLSceneTransformModel::parentNode(n))
      {
      ++index;
      nId = n->GetID();
      if (nId && !strcmp(nodeId, nId))
        {
        return index;
        }
      }
    }
  return -1;
}
*/

//------------------------------------------------------------------------------
bool qMRMLSceneTransformModel::canBeAChild(vtkMRMLNode* node)const
{
  return node ? node->IsA("vtkMRMLTransformableNode") : false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneTransformModel::canBeAParent(vtkMRMLNode* node)const
{
  return node ? node->IsA("vtkMRMLTransformNode") : false;
}

//------------------------------------------------------------------------------
bool qMRMLSceneTransformModel::reparent(vtkMRMLNode* node, vtkMRMLNode* newParent)
{
  Q_ASSERT(node);
  if (!node || qMRMLSceneTransformModel::parentNode(node) == newParent)
    {
    return false;
    }
  Q_ASSERT(newParent != node);
  // MRML Transformable Nodes
  vtkMRMLTransformableNode* transformableNode =
    vtkMRMLTransformableNode::SafeDownCast(node);
  if (transformableNode)
    {
    transformableNode->SetAndObserveTransformNodeID( newParent ? newParent->GetID() : 0 );
    transformableNode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
class qMRMLSceneTransformModelPrivate: public qMRMLSceneModelPrivate
{
protected:
  Q_DECLARE_PUBLIC(qMRMLSceneTransformModel);
public:
  qMRMLSceneTransformModelPrivate(qMRMLSceneTransformModel& object);

};

//------------------------------------------------------------------------------
qMRMLSceneTransformModelPrivate
::qMRMLSceneTransformModelPrivate(qMRMLSceneTransformModel& object)
  : qMRMLSceneModelPrivate(object)
{

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneTransformModel::qMRMLSceneTransformModel(QObject *vparent)
  :qMRMLSceneModel(new qMRMLSceneTransformModelPrivate(*this), vparent)
{
}

//------------------------------------------------------------------------------
qMRMLSceneTransformModel::~qMRMLSceneTransformModel()
{
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLSceneTransformModel::supportedDropActions()const
{
  return Qt::MoveAction;
}
