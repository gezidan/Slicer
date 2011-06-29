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

  This file was originally developed by Danielle Pace, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerSettingsRemoteDataPanel_h
#define __qSlicerSettingsRemoteDataPanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QSettings;
class qSlicerSettingsRemoteDataPanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSettingsRemoteDataPanel
  : public ctkSettingsPanel
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSettingsRemoteDataPanel(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerSettingsRemoteDataPanel();

protected slots:
  void onEnableAsynchronousIOChanged(bool);
  void onForceRedownloadChanged(bool);
  void onCacheDirectoryChanged(const QString& path);
  void onCacheSizeLimitChanged(int);
  void onCacheFreeBufferSizeChanged(int);

protected:
  QScopedPointer<qSlicerSettingsRemoteDataPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSettingsRemoteDataPanel);
  Q_DISABLE_COPY(qSlicerSettingsRemoteDataPanel);
};

#endif
