/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLTensorVolumeNode.h"

#include <vtkPolyData.h>

#include <stdlib.h>
#include <iostream>

#include "TestingMacros.h"

int vtkMRMLTensorVolumeNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLTensorVolumeNode > node1 = vtkSmartPointer< vtkMRMLTensorVolumeNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLTensorVolumeNode, node1);
  
  return EXIT_SUCCESS;
}
