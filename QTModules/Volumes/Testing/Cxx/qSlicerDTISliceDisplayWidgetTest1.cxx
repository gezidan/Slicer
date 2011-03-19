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
#include <QApplication>
#include <QTimer>

// Volumes includes
#include "qSlicerDTISliceDisplayWidget.h"

// MRML includes
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
int qSlicerDTISliceDisplayWidgetTest1( int argc, char * argv[] )
{
  QApplication app(argc, argv);

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer<vtkMRMLDiffusionTensorDisplayPropertiesNode> propertiesNode =
    vtkSmartPointer<vtkMRMLDiffusionTensorDisplayPropertiesNode>::New();
  scene->AddNode(propertiesNode);
  vtkSmartPointer<vtkMRMLDiffusionTensorVolumeSliceDisplayNode> displayNode =
    vtkSmartPointer<vtkMRMLDiffusionTensorVolumeSliceDisplayNode>::New();
  displayNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(propertiesNode->GetID());
  scene->AddNode(displayNode);

  qSlicerDTISliceDisplayWidget widget;
  widget.setMRMLScene(scene);
  widget.setMRMLDTISliceDisplayNode(displayNode);

  widget.show();
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
