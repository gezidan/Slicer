/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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
#include <QTimer>
#include <QWidget>

// SlicerQt includes
#include <qMRMLThreeDWidget.h>
#include <qSlicerAbstractModuleRepresentation.h>
#include <qSlicerApplication.h>

// Volumes includes
#include "qSlicerModelsModule.h"
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLViewNode.h>
// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
int qSlicerModelsModuleWidgetTestScene( int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);

  if (argc < 2)
    {
    std::cerr << "Usage: qSlicerModelsModuleWidgetTest1 sceneFilePath [-I]"
              << std::endl;
    return EXIT_FAILURE;
    }

  qSlicerModelsModule module;
  module.initialize(0);

  vtkNew<vtkMRMLScene> scene;
  scene->SetURL(argv[1]);
  scene->Connect();

  module.setMRMLScene(scene.GetPointer());

  dynamic_cast<QWidget*>(module.widgetRepresentation())->show();

  qMRMLThreeDWidget view;
  view.setMRMLScene(scene.GetPointer());
  scene->InitTraversal();
  view.setMRMLViewNode(vtkMRMLViewNode::SafeDownCast(
    scene->GetNextNodeByClass("vtkMRMLViewNode")));
  view.show();

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }
  return app.exec();
}
