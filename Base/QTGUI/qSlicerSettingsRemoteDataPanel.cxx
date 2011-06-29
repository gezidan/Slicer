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

// Qt includes
#include <QDebug>
#include <QMainWindow>

// CTK includes
#include <ctkLogger.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsRemoteDataPanel.h"
#include "ui_qSlicerSettingsRemoteDataPanel.h"

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLScene.h>

static ctkLogger logger("org.commontk.libs.widgets.qSlicerSettingsRemoteDataPanel");

// --------------------------------------------------------------------------
// qSlicerSettingsRemoteDataPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsRemoteDataPanelPrivate: public Ui_qSlicerSettingsRemoteDataPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsRemoteDataPanel);
protected:
  qSlicerSettingsRemoteDataPanel* const q_ptr;

public:
  qSlicerSettingsRemoteDataPanelPrivate(qSlicerSettingsRemoteDataPanel& object);
  void init();

};

// --------------------------------------------------------------------------
// qSlicerSettingsRemoteDataPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsRemoteDataPanelPrivate::qSlicerSettingsRemoteDataPanelPrivate(qSlicerSettingsRemoteDataPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsRemoteDataPanelPrivate::init()
{
  Q_Q(qSlicerSettingsRemoteDataPanel);

  this->setupUi(q);

  // Default cache directory value from the cache manager
  Q_ASSERT(qSlicerApplication::application()->mrmlScene()->GetCacheManager());
  const char * pathPtr
      = qSlicerApplication::application()->mrmlScene()->GetCacheManager()->GetRemoteCacheDirectory();
  this->CacheDirectoryButton->setDirectory(QLatin1String(pathPtr));

  // Connect panel widgets with associated slots
  QObject::connect(this->CacheDirectoryButton, SIGNAL(directoryChanged(const QString&)),
                   q, SLOT(onCacheDirectoryChanged(const QString&)));

  // Register settings with their corresponding widgets
  q->registerProperty("RemoteData/CacheDirectory", this->CacheDirectoryButton,
                      "directory", SIGNAL(directoryChanged(const QString&)));
}

// --------------------------------------------------------------------------
// qSlicerSettingsRemoteDataPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsRemoteDataPanel::qSlicerSettingsRemoteDataPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsRemoteDataPanelPrivate(*this))
{
  Q_D(qSlicerSettingsRemoteDataPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsRemoteDataPanel::~qSlicerSettingsRemoteDataPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsRemoteDataPanel::onCacheDirectoryChanged(const QString& path)
{
  Q_ASSERT(qSlicerApplication::application()->mrmlScene()->GetCacheManager());
  QByteArray bytes = path.toUtf8();
  char * pathPtr = bytes.data();
  qSlicerApplication::application()->mrmlScene()->GetCacheManager()->SetRemoteCacheDirectory(pathPtr);
}

