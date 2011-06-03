#ifndef __qSlicerVolumeRenderingModuleWidget_h
#define __qSlicerVolumeRenderingModuleWidget_h

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerVolumeRenderingModuleExport.h"

class qSlicerVolumeRenderingModuleWidgetPrivate;
class vtkMRMLAnnotationROINode;
class vtkMRMLNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeRenderingModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVolumeRenderingModuleWidget();

  vtkMRMLScalarVolumeNode* mrmlVolumeNode()const;
  vtkMRMLVolumeRenderingDisplayNode* mrmlDisplayNode()const;
  vtkMRMLAnnotationROINode* mrmlROINode()const;
  QList<vtkMRMLViewNode*> mrmlViewNodes()const;

public slots:

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setMRMLDisplayNode(vtkMRMLNode* node);

  void setMRMLROINode(vtkMRMLNode* node);

  void setMRMLVolumePropertyNode(vtkMRMLNode* node);

  void addVolumeIntoView(vtkMRMLNode* node);

  void fitROIToVolume();
protected slots:
  void onCurrentMRMLVolumeNodeChanged(vtkMRMLNode* node);
  void onVisibilityChanged(bool);
  void onCropToggled(bool);

  void onCurrentMRMLDisplayNodeChanged(vtkMRMLNode* node);
  void onCurrentMRMLROINodeChanged(vtkMRMLNode* node);
  void onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode* node);
  void onCheckedViewNodesChanged();

  void updateFromMRMLDisplayNode();

protected:
  QScopedPointer<qSlicerVolumeRenderingModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModuleWidget);
};

#endif
