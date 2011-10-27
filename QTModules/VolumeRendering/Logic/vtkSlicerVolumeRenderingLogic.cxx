/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeRenderingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// 
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkSlicerVolumeRenderingLogic.h"

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumePropertyStorageNode.h>

// VTKSYS includes
#include <itksys/SystemTools.hxx> 

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <algorithm>

double higherAndUnique(double value, double &previousValue)
{
  value = std::max(value, previousValue);
  if (value == previousValue)
    {
    value += 0.00000000000001;
    }
  previousValue = value;
  return value;
}

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerVolumeRenderingLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::vtkSlicerVolumeRenderingLogic()
{
  this->PresetsScene = 0;
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic::~vtkSlicerVolumeRenderingLogic()
{
  if (this->PresetsScene)
    {
    this->PresetsScene->Delete();
    }
  this->RemoveAllVolumeRenderingDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Display nodes:" << std::endl;
  for (unsigned int i = 0; i < this->DisplayNodes.size(); ++i)
    {
    os << indent << this->DisplayNodes[i]->GetID() << std::endl;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RegisterNodes()
{
  assert(this->GetMRMLScene());
  // :NOTE: 20050513 tgl: Guard this so it is only registered once.
  vtkMRMLVolumeRenderingScenarioNode *vrsNode = vtkMRMLVolumeRenderingScenarioNode::New();
  this->GetMRMLScene()->RegisterNodeClass(vrsNode);
  vrsNode->Delete();

  vtkMRMLVolumeRenderingDisplayNode *vrpNode = vtkMRMLVolumeRenderingDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(vrpNode);
  vrpNode->Delete();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::AddVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it = 
    std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it != this->DisplayNodes.end())
    {
    // already added
    return;
    }
  // push empty...
  //this->DisplayNodes.push_back(0);
  it = this->DisplayNodes.insert(this->DisplayNodes.end(), 0);
  // .. then set and observe
  vtkSetAndObserveMRMLNodeMacro(*it, node);
  this->UpdateVolumeRenderingDisplayNode(node);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::RemoveVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  DisplayNodesType::iterator it = 
    std::find(this->DisplayNodes.begin(), this->DisplayNodes.end(), node);
  if (it == this->DisplayNodes.end())
    {
    return;
    }
  // unobserve
  vtkSetAndObserveMRMLNodeMacro(*it, 0);
  this->DisplayNodes.erase(it);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::RemoveAllVolumeRenderingDisplayNodes()
{
  for (; this->DisplayNodes.size();)
    {
    this->RemoveVolumeRenderingDisplayNode(
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(this->DisplayNodes[0]));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::AddAllVolumeRenderingDisplayNodes()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  std::vector<vtkMRMLNode*> volumeRenderingDisplayNodes;
  this->GetMRMLScene()->GetNodesByClass(
    "vtkMRMLVolumeRenderingDisplayNode", volumeRenderingDisplayNodes);
  std::vector<vtkMRMLNode*>::const_iterator it;
  for (it = volumeRenderingDisplayNodes.begin();
       it != volumeRenderingDisplayNodes.end(); ++it)
    {
    this->AddVolumeRenderingDisplayNode(
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(*it));
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::UpdateVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* node)
{
  assert(node);
  if (!node->GetVolumeNode())
    {
    return;
    }
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      node->GetVolumeNode()->GetDisplayNode());
  assert(displayNode);
  if (node->GetFollowVolumeDisplayNode())
    {
    // observe display node if not already observing it
    if (!this->GetMRMLNodesObserverManager()->GetObservationsCount(displayNode))
      {
      vtkObserveMRMLNodeMacro(displayNode);
      }
    this->CopyScalarDisplayToVolumeRenderingDisplayNode(node);
    }
  else
    {
    // unobserve display node
    vtkUnObserveMRMLNodeMacro(displayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::SetMRMLSceneInternal(vtkMRMLScene* scene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(scene, sceneEvents.GetPointer());

  this->RemoveAllVolumeRenderingDisplayNodes();
  this->AddAllVolumeRenderingDisplayNodes();
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->AddVolumeRenderingDisplayNode(vrDisplayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->RemoveVolumeRenderingDisplayNode(vrDisplayNode);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::OnMRMLNodeModified(vtkMRMLNode* node)
{
  vtkMRMLVolumeRenderingDisplayNode* vrDisplayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  if (vrDisplayNode)
    {
    this->UpdateVolumeRenderingDisplayNode(vrDisplayNode);
    }
  vtkMRMLScalarVolumeDisplayNode* scalarVolumeDisplayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node);
  if (scalarVolumeDisplayNode)
    {
    for (unsigned int i = 0; i < this->DisplayNodes.size(); ++i)
      {
      vrDisplayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
        this->DisplayNodes[i]);
      if (vrDisplayNode->GetVolumeNode()->GetDisplayNode() ==
          scalarVolumeDisplayNode &&
          vrDisplayNode->GetFollowVolumeDisplayNode())
        {
        this->CopyScalarDisplayToVolumeRenderingDisplayNode(
          vrDisplayNode, scalarVolumeDisplayNode);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::UpdateTranferFunctionRangeFromImage(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == 0 || vspNode->GetVolumeNode() == 0 || vspNode->GetVolumePropertyNode() == 0)
  {
    return;
  }
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
  vtkVolumeProperty *prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  if (input == NULL || prop == NULL)
    {
    return;
    }

  //update scalar range
  vtkColorTransferFunction *functionColor = prop->GetRGBTransferFunction();

  vtkDataArray* scalars = input->GetPointData()->GetScalars();
  if (!scalars)
    {
    return;
    }

  double rangeNew[2];
  scalars->GetRange(rangeNew);
  functionColor->AdjustRange(rangeNew);
  
  vtkPiecewiseFunction *functionOpacity = prop->GetScalarOpacity();
  functionOpacity->AdjustRange(rangeNew);

  rangeNew[1] = (rangeNew[1] - rangeNew[0])*0.25;
  rangeNew[0] = 0;

  functionOpacity = prop->GetGradientOpacity();
  functionOpacity->RemovePoint(255);//Remove the standard value
  functionOpacity->AdjustRange(rangeNew);
}


//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::SetWindowLevelAndThresholdToVolumeProp(double scalarRange[2],
                                         double threshold[2],
                                         double windowLevel[2],
                                         vtkLookupTable* lut,
                                         vtkVolumeProperty* volumeProp)
{
  assert(scalarRange && threshold && windowLevel && volumeProp);
  // Sanity check
  threshold[0] = std::max(std::min(threshold[0], scalarRange[1]), scalarRange[0]);
  threshold[1] = std::min(std::max(threshold[1], scalarRange[0]), scalarRange[1]);

  double windowLevelMinMax[2];
  windowLevelMinMax[0] = windowLevel[1] - 0.5 * windowLevel[0];
  windowLevelMinMax[1] = windowLevel[1] + 0.5 * windowLevel[0];

  volumeProp->SetInterpolationTypeToLinear();

  double previous = VTK_DOUBLE_MIN;

  vtkNew<vtkPiecewiseFunction> opacity;
  // opacity doesn't support duplicate points
  opacity->AddPoint(higherAndUnique(scalarRange[0], previous), 0.0);
  opacity->AddPoint(higherAndUnique(threshold[0], previous), 0.0);
  opacity->AddPoint(higherAndUnique(threshold[0], previous), 1.0);
  opacity->AddPoint(higherAndUnique(threshold[1], previous), 1.0);
  opacity->AddPoint(higherAndUnique(threshold[1], previous), 0.0);
  opacity->AddPoint(higherAndUnique(scalarRange[1], previous), 0.0);

  vtkNew<vtkColorTransferFunction> colorTransfer;

  const int size = lut ? lut->GetNumberOfTableValues() : 0;
  if (size == 0)
    {
    double black[3] = {0., 0., 0.};
    double white[3] = {1., 1., 1.};
    colorTransfer->AddRGBPoint(scalarRange[0], black[0], black[1], black[2]);
    colorTransfer->AddRGBPoint(windowLevelMinMax[0], black[0], black[1], black[2]);
    colorTransfer->AddRGBPoint(windowLevelMinMax[1], white[0], white[1], white[2]);
    colorTransfer->AddRGBPoint(scalarRange[1], white[0], white[1], white[2]);
    }
  else if (size == 1)
    {
    double color[4];
    lut->GetTableValue(0, color);

    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[0], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(windowLevelMinMax[0], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(windowLevelMinMax[1], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[1], previous),
                               color[0], color[1], color[2]);
    }
  else // if (size > 1)
    {
    previous = VTK_DOUBLE_MIN;

    double color[4];
    lut->GetTableValue(0, color);
    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[0], previous),
                               color[0], color[1], color[2]);

    double value = windowLevelMinMax[0];

    double step = windowLevel[0] / (size - 1);

    int downSamplingFactor = 64;
    for (int i = 0; i < size; i += downSamplingFactor,
                              value += downSamplingFactor*step)
      {
      lut->GetTableValue(i, color);
      colorTransfer->AddRGBPoint(higherAndUnique(value, previous),
                                 color[0], color[1], color[2]);
      }

    lut->GetTableValue(size - 1, color);
    colorTransfer->AddRGBPoint(higherAndUnique(windowLevelMinMax[1], previous),
                               color[0], color[1], color[2]);
    colorTransfer->AddRGBPoint(higherAndUnique(scalarRange[1], previous),
                               color[0], color[1], color[2]);
    }

  vtkPiecewiseFunction *volumePropOpacity = volumeProp->GetScalarOpacity();
  if (this->IsDifferentFunction(opacity.GetPointer(), volumePropOpacity))
    {
    volumePropOpacity->DeepCopy(opacity.GetPointer());
    }
  vtkColorTransferFunction *volumePropColorTransfer = volumeProp->GetRGBTransferFunction();
  if (this->IsDifferentFunction(colorTransfer.GetPointer(), volumePropColorTransfer))
    {
    volumePropColorTransfer->DeepCopy(colorTransfer.GetPointer());
    }

  volumeProp->ShadeOn();
  volumeProp->SetAmbient(0.30);
  volumeProp->SetDiffuse(0.60);
  volumeProp->SetSpecular(0.50);
  volumeProp->SetSpecularPower(40);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::CopyScalarDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  assert(vspNode);
  this->CopyScalarDisplayToVolumeRenderingDisplayNode(
    vspNode,
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      vspNode->GetVolumeNode()->GetDisplayNode()));
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic
::CopyScalarDisplayToVolumeRenderingDisplayNode(
  vtkMRMLVolumeRenderingDisplayNode* vspNode,
  vtkMRMLScalarVolumeDisplayNode* vpNode)
{
  assert(vspNode);
  assert(vspNode->GetVolumePropertyNode());
  assert(vpNode);

  double scalarRange[2];
  vpNode->GetScalarRange(scalarRange);

  double windowLevel[2];
  windowLevel[0] = vpNode->GetWindow();
  windowLevel[1] = vpNode->GetLevel();

  double threshold[2];
  threshold[0] = vpNode->GetLowerThreshold();
  threshold[1] = vpNode->GetUpperThreshold();

  vtkLookupTable* lut = vpNode->GetColorNode() ?
    vpNode->GetColorNode()->GetLookupTable() : 0;
  vtkVolumeProperty *prop =
    vspNode->GetVolumePropertyNode()->GetVolumeProperty();

  int disabledModify = vspNode->StartModify();
  this->SetWindowLevelAndThresholdToVolumeProp(
    scalarRange, threshold, windowLevel, lut, prop);
  vspNode->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::FitROIToVolume(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{

  // resize the ROI to fit the volume
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(vspNode->GetROINode());
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());


  if (volumeNode && roiNode)
  {
    int disabledModify = roiNode->StartModify();
  
    double xyz[3];
    double center[3];

    vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, xyz,  center);
    for (int i = 0; i < 3; i++)
    {
      xyz[i] *= 0.5;
    }

    roiNode->SetXYZ(center);
    roiNode->SetRadiusXYZ(xyz);

    roiNode->EndModify(disabledModify);
  }


}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::CreateVolumeRenderingDisplayNode()
{
  vtkMRMLVolumeRenderingDisplayNode *node = NULL;

  if (this->GetMRMLScene())
  {
    node = vtkMRMLVolumeRenderingDisplayNode::New();
    node->SetCurrentVolumeMapper(vtkMRMLVolumeRenderingDisplayNode::VTKCPURayCast);
    this->GetMRMLScene()->AddNode(node);
    node->Delete();
  }

  return node;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingScenarioNode* vtkSlicerVolumeRenderingLogic::CreateScenarioNode()
{
  vtkMRMLVolumeRenderingScenarioNode *node = NULL;

  if (this->GetMRMLScene())
  {
    node = vtkMRMLVolumeRenderingScenarioNode::New();
    this->GetMRMLScene()->AddNode(node);
    node->Delete();
  }

  return node;
}

// Description:
// Remove ViewNode from VolumeRenderingDisplayNode for a VolumeNode,
//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::RemoveViewFromVolumeDisplayNodes(
                                          vtkMRMLVolumeNode *volumeNode, 
                                          vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || volumeNode == NULL)
    {
    return;
    }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      dnode->RemoveViewNodeID(viewNode->GetID());
      }
    }
}

// Description:
// Find volume rendering display node reference in the volume
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeByID(
                                                              vtkMRMLVolumeNode *volumeNode, 
                                                              char *displayNodeID)
{
  if (displayNodeID == NULL || volumeNode == NULL)
    {
    return NULL;
    }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode && !strcmp(displayNodeID, dnode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Find first volume rendering display node
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetFirstVolumeRenderingDisplayNode(
                                                              vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == NULL)
    {
    return NULL;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Find volume rendering display node referencing the view node and volume node
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
                                                              vtkMRMLVolumeNode *volumeNode, 
                                                              vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || volumeNode == NULL)
    {
    return NULL;
    }
  int ndnodes = volumeNode->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      volumeNode->GetNthDisplayNode(i));
    if (dnode)
      {
      for (int j=0; j<dnode->GetNumberOfViewNodeIDs(); j++)
        {
        if (!strcmp(viewNode->GetID(), dnode->GetNthViewNodeID(j)))
          {
          return dnode;
          }
        }
      }
    }
  return NULL;
}

// Description:
// Find volume rendering display node referencing the view node in the scene
//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic::GetVolumeRenderingDisplayNodeForViewNode(
                                                              vtkMRMLViewNode *viewNode)
{
  if (viewNode == NULL || viewNode->GetScene() == NULL)
    {
    return NULL;
    }
  std::vector<vtkMRMLNode *> nodes;
  viewNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i=0; i<nodes.size(); i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
      nodes[i]);
    if (dnode && dnode->IsViewNodeIDPresent(viewNode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* vtkSlicerVolumeRenderingLogic
::GetFirstVolumeRenderingDisplayNodeByROINode(vtkMRMLAnnotationROINode* roiNode)
{
  if (roiNode == NULL || roiNode->GetScene() == NULL)
    {
    return NULL;
    }
  std::vector<vtkMRMLNode *> nodes;
  roiNode->GetScene()->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (unsigned int i = 0; i < nodes.size(); ++i)
    {
    vtkMRMLVolumeRenderingDisplayNode *dnode =
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]);
    if (dnode && dnode->GetROINodeID() &&
        !strcmp(dnode->GetROINodeID(), roiNode->GetID()))
      {
      return dnode;
      }
    }
  return NULL;
}

// Description:
// Update vtkMRMLVolumeRenderingDisplayNode from VolumeNode,
// if needed create vtkMRMLVolumePropertyNode and vtkMRMLAnnotationROINode
// and initioalize them from VolumeNode
//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingLogic::UpdateDisplayNodeFromVolumeNode(
                                          vtkMRMLVolumeRenderingDisplayNode *displayNode,
                                          vtkMRMLVolumeNode *volumeNode,
                                          vtkMRMLVolumePropertyNode **propNode,
                                          vtkMRMLAnnotationROINode **roiNode)
{

  if (volumeNode == NULL)
    {
    displayNode->SetAndObserveVolumeNodeID(NULL);
    return;
    }

  displayNode->SetAndObserveVolumeNodeID(volumeNode->GetID());

  if (*propNode == NULL)
    {
    *propNode = vtkMRMLVolumePropertyNode::New();
    this->GetMRMLScene()->AddNode(*propNode);
    (*propNode)->Delete();
    }
  displayNode->SetAndObserveVolumePropertyNodeID((*propNode)->GetID());

  if (*roiNode == NULL)
    {
    *roiNode = vtkMRMLAnnotationROINode::New();
    // By default, the ROI is interactive. It could be an application setting.
    (*roiNode)->SetInteractiveMode(1);
    // by default, show the ROI only if cropping is enabled
    (*roiNode)->SetVisibility(displayNode->GetCroppingEnabled());
    this->GetMRMLScene()->AddNode(*roiNode);
    (*roiNode)->Delete();
    }
  displayNode->SetAndObserveROINodeID((*roiNode)->GetID());

  //this->UpdateVolumePropertyFromImageData(displayNode);
  this->CopyScalarDisplayToVolumeRenderingDisplayNode(displayNode);

  this->FitROIToVolume(displayNode);
}

//----------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* vtkSlicerVolumeRenderingLogic
::AddVolumePropertyFromFile (const char* filename)
{
  vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::New();
  vtkMRMLVolumePropertyStorageNode *vpStorageNode = vtkMRMLVolumePropertyStorageNode::New();

  // check for local or remote files
  int useURI = 0; // false;
  if (this->GetMRMLScene()->GetCacheManager() != NULL)
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }
  
  itksys_stl::string name;
  const char *localFile;
  if (useURI)
    {
    vpStorageNode->SetURI(filename);
     // reset filename to the local file name
    localFile = ((this->GetMRMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
    }
  else
    {
    vpStorageNode->SetFileName(filename);
    localFile = filename;
    }
  const itksys_stl::string fname(localFile);
  // the model name is based on the file name (itksys call should work even if
  // file is not on disk yet)
  name = itksys::SystemTools::GetFilenameName(fname);
  
  // check to see which node can read this type of file
  if (!vpStorageNode->SupportedFileType(name.c_str()))
    {
    vpStorageNode->Delete();
    vpStorageNode = NULL;
    }

  /* don't read just yet, need to add to the scene first for remote reading
  if (vpStorageNode->ReadData(vpNode) != 0)
    {
    storageNode = vpStorageNode;
    }
  */
  if (vpStorageNode != NULL)
    {
    std::string uname( this->GetMRMLScene()->GetUniqueNameByString(name.c_str()));

    vpNode->SetName(uname.c_str());

    this->GetMRMLScene()->SaveStateForUndo();

    vpNode->SetScene(this->GetMRMLScene());
    vpStorageNode->SetScene(this->GetMRMLScene());

    this->GetMRMLScene()->AddNode(vpStorageNode);
    vpNode->SetAndObserveStorageNodeID(vpStorageNode->GetID());

    this->GetMRMLScene()->AddNode(vpNode);

    // the scene points to it still
    vpNode->Delete();

    // now set up the reading
    int retval = vpStorageNode->ReadData(vpNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddVolumePropertyFromFile: error reading " << filename);
      this->GetMRMLScene()->RemoveNode(vpNode);
      this->GetMRMLScene()->RemoveNode(vpStorageNode);
      vpNode = NULL;
      }
    }
  else
    {
    vtkDebugMacro("Couldn't read file, returning null model node: " << filename);
    vpNode->Delete();
    vpNode = NULL;
    }
  if (vpStorageNode)
    {
    vpStorageNode->Delete();
    }
  return vpNode;
}

//---------------------------------------------------------------------------
vtkMRMLScene* vtkSlicerVolumeRenderingLogic::GetPresetsScene()
{
  if (!this->PresetsScene)
    {
    this->PresetsScene = vtkMRMLScene::New();
    this->LoadPresets(this->PresetsScene);
    }
  return this->PresetsScene;
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::LoadPresets(vtkMRMLScene* scene)
{
  this->PresetsScene->RegisterNodeClass(vtkNew<vtkMRMLVolumePropertyNode>().GetPointer());

  if (this->GetModuleShareDirectory().empty())
    {
    vtkErrorMacro(<< "Failed to load presets: Share directory *NOT* set !");
    return false;
    }

  std::string presetFileName = this->GetModuleShareDirectory() + "/presets.xml";
  scene->SetURL(presetFileName.c_str());
  int connected = scene->Connect();
  if (connected != 1)
    {
    vtkErrorMacro(<< "Failed to load presets [" << presetFileName << "]");
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::IsDifferentFunction(
  vtkPiecewiseFunction* function1, vtkPiecewiseFunction* function2)const
{
  if ((function1 != 0) ^ (function2 != 0))
    {
    return true;
    }
  if (function1->GetSize() != function2->GetSize())
    {
    return true;
    }
  bool different = false;
  for (int i = 0; i < function1->GetSize(); ++i)
    {
    double node1[4];
    function1->GetNodeValue(i, node1);
    double node2[4];
    function2->GetNodeValue(i, node2);
    for (unsigned int j = 0; j < 4; ++j)
      {
      if (node1[j] != node2[j])
        {
        different = true;
        break;
        }
      }
    if (different)
      {
      break;
      }
    }
  return different;
}


//---------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingLogic::IsDifferentFunction(
  vtkColorTransferFunction* function1, vtkColorTransferFunction* function2)const
{
  if ((function1 != 0) ^ (function2 != 0))
    {
    return true;
    }
  if (function1->GetSize() != function2->GetSize())
    {
    return true;
    }
  bool different = false;
  for (int i = 0; i < function1->GetSize(); ++i)
    {
    double node1[6];
    function1->GetNodeValue(i, node1);
    double node2[6];
    function2->GetNodeValue(i, node2);
    for (unsigned int j = 0; j < 6; ++j)
      {
      if (node1[j] != node2[j])
        {
        different = true;
        break;
        }
      }
    if (different)
      {
      break;
      }
    }
  return different;
}
