/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLModelHierarchyNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkMRMLModelHierarchyNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelHierarchyNode");
  if(ret)
    {
    return (vtkMRMLModelHierarchyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelHierarchyNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelHierarchyNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelHierarchyNode");
  if(ret)
    {
    return (vtkMRMLModelHierarchyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelHierarchyNode;
}


//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNode::vtkMRMLModelHierarchyNode()
{
  this->ModelDisplayNode = NULL;
  this->HideFromEditors = 1;
}

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNode::~vtkMRMLModelHierarchyNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "modelNodeRef") ||
        !strcmp(attName, "modelNodeID") )
      {
      this->SetDisplayableNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ModelNodeID, this);
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLModelHierarchyNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
//  vtkMRMLModelHierarchyNode *node = (vtkMRMLModelHierarchyNode *) anode;

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  if (this->ModelDisplayNode)
    {
    os << indent << "ModelDisplayNode ID = " <<
      (this->ModelDisplayNode->GetID() ? this->ModelDisplayNode->GetID() : "(none)") << "\n";
    }
}

//-----------------------------------------------------------
void vtkMRMLModelHierarchyNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

}

//-----------------------------------------------------------
void vtkMRMLModelHierarchyNode::UpdateReferences()
{
  Superclass::UpdateReferences();  
}

vtkMRMLModelNode* vtkMRMLModelHierarchyNode::GetModelNode()
{
  vtkMRMLModelNode* node = vtkMRMLModelNode::SafeDownCast(
    this->GetAssociatedNode());
  return node;
}

//----------------------------------------------------------------------------
vtkMRMLModelDisplayNode* vtkMRMLModelHierarchyNode::GetModelDisplayNode()
{
  vtkMRMLModelDisplayNode* node = NULL;
  vtkMRMLNode* snode = Superclass::GetDisplayNode();
  if (snode)
    {
    node = vtkMRMLModelDisplayNode::SafeDownCast(snode);
    }
  return node;
}



//---------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLModelDisplayNode *dnode = this->GetModelDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLModelDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}

//----------------------------------------------------------------------------
vtkMRMLModelHierarchyNode* vtkMRMLModelHierarchyNode::GetUnExpandedParentNode()
{
  vtkMRMLModelHierarchyNode *node = NULL;
  vtkMRMLDisplayableHierarchyNode *dhnode = Superclass::GetUnExpandedParentNode();
  if (dhnode != NULL)
    {
    node = vtkMRMLModelHierarchyNode::SafeDownCast(dhnode);
    }
  return node;
}


  
//---------------------------------------------------------------------------
void vtkMRMLModelHierarchyNode:: GetChildrenModelNodes(vtkCollection *models)
{
  if (models == NULL)
    {
    return;
    }
  vtkMRMLScene *scene = this->GetScene();
  vtkMRMLNode *mnode = NULL;
  vtkMRMLModelHierarchyNode *hnode = NULL;
  for (int n=0; n < scene->GetNumberOfNodes(); n++) 
    {
    mnode = scene->GetNthNode(n);
    if (mnode->IsA("vtkMRMLModelNode"))
      {
      hnode = vtkMRMLModelHierarchyNode::SafeDownCast(
          vtkMRMLDisplayableHierarchyNode::GetDisplayableHierarchyNode(scene, mnode->GetID()));
      while (hnode)
        {
        if (hnode == this) 
          {
          models->AddItem(mnode);
          break;
          }
          hnode = vtkMRMLModelHierarchyNode::SafeDownCast(hnode->GetParentNode());
        }// end while
      }// end if
    }// end for
}


