
// QT includes
#include <QApplication>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QTreeView>

// CTK includes
#include <ctkModelTester.h>

// qMRML includes
#include "qMRMLSceneFactoryWidget.h"
#include "qMRMLSceneDisplayableModel.h"
#include "qMRMLUtils.h"

#include "TestingMacros.h"
#include <vtkEventBroker.h>

#include "GUI/qMRMLAnnotationTreeWidget.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include "MRML/vtkMRMLAnnotationRulerNode.h"


// STD includes
#include <cstdlib>
#include <iostream>

// MRML includes
#include <vtkMRMLDisplayableHierarchyNode.h>

int qMRMLSceneAnnotationModelAndAnnotationTreeWidgetTest1(int argc, char * argv [])
{
  QApplication app(argc, argv);

  qMRMLSceneFactoryWidget sceneFactory(0);

  sceneFactory.generateScene();

  qMRMLAnnotationTreeWidget* view = new qMRMLAnnotationTreeWidget(0);

  //view->setSelectionBehavior(QAbstractItemView::SelectRows);

  vtkSlicerAnnotationModuleLogic* logic = vtkSlicerAnnotationModuleLogic::New();

  logic->SetMRMLScene(sceneFactory.mrmlScene());

  logic->InitializeEventListeners();

  view->setAndObserveLogic(logic);
  view->setMRMLScene(sceneFactory.mrmlScene());
  view->hideScene();

  view->show();
  view->resize(500, 800);

  QTreeView view3;
  view3.setModel(view->sceneModel());
  view3.show();
/*
  qMRMLTreeWidget view2;
  view2.setSceneModelType("Displayable");
  view2.sceneModel()->setMRMLScene(sceneFactory.mrmlScene());
  view2.show();
*/
  double worldCoordinates1[3] = {0,0,0};
  double worldCoordinates2[3] = {50,50,50};

  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

  vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);
  rulerNode->SetDistanceMeasurement(distance);

  rulerNode->SetName(sceneFactory.mrmlScene()->GetUniqueNameByString("AnnotationRuler"));

  rulerNode->Initialize(sceneFactory.mrmlScene());

  rulerNode->Delete();

  std::cout << "Measurement in rulerNode: " << rulerNode->GetDistanceMeasurement() << std::endl;
/*
  QModelIndex index = view->d_func()->SceneModel->indexFromNode(sceneFactory.mrmlScene()->GetNthNodeByClass(0,"vtkMRMLAnnotationRulerNode"));

  qMRMLAbstractItemHelper* helper = view->d_func()->SceneModel->itemFromIndex(index);
  std::cout << helper->data(Qt::DisplayRole) << std::endl;
*/

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

