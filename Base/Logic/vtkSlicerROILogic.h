///  vtkSlicerROILogic - slicer logic class for ROI manipulation
/// 
/// This class manages the logic associated ROI

#ifndef __vtkSlicerROILogic_h
#define __vtkSlicerROILogic_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// VTK includes
class vtkMRMLROIListNode;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerROILogic : public vtkMRMLAbstractLogic 
{
public:
  /// The Usual vtk class functions
  static vtkSlicerROILogic *New();
  vtkTypeRevisionMacro(vtkSlicerROILogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Update logic state when MRML scene changes
  void ProcessMRMLEvents(); 
  virtual void ProcessMRMLEvents ( vtkObject * caller, 
      unsigned long event, void * callData ) 
    { Superclass::ProcessMRMLEvents(caller, event, callData); }

  vtkMRMLROIListNode * AddROIList();

protected:
  vtkSlicerROILogic();
  ~vtkSlicerROILogic();
  vtkSlicerROILogic(const vtkSlicerROILogic&);
  void operator=(const vtkSlicerROILogic&);
};

#endif
