

#ifndef __qMRMLChartView_p_h
#define __qMRMLChartView_p_h

// Qt includes
class QToolButton;

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
class ctkPopupWidget;

// qMRML includes
#include "qMRMLChartView.h"

class vtkMRMLDisplayableManagerGroup;
class vtkMRMLChartViewNode;
class vtkMRMLCameraNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLChartViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLChartView);
protected:
  qMRMLChartView* const q_ptr;
public:
  qMRMLChartViewPrivate(qMRMLChartView& object);
  ~qMRMLChartViewPrivate();

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);                                        
  vtkMRMLScene *mrmlScene();
                                       
public slots:
  /// Handle MRML scene event
  void startProcessing();
  void endProcessing();

  void updateWidgetFromMRML();

protected:
  void initDisplayableManagers();

  vtkMRMLDisplayableManagerGroup*    DisplayableManagerGroup;
  vtkMRMLScene*                      MRMLScene;
  vtkMRMLChartViewNode*              MRMLChartViewNode;
  
  QToolButton*                       PinButton;
  ctkPopupWidget*                    PopupWidget;
};

#endif
