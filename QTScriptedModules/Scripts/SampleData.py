import os
from __main__ import slicer
import qt, ctk

#
# SampleData
#

class SampleData:
  def __init__(self, parent):
    parent.title = "Sample Data"
    parent.category = "Informatics"
    parent.contributor = "Steve Pieper and Danielle Pace"
    parent.helpText = """
The SampleData module can be used to download data for working with in slicer.  Use of this module requires an active network connection.  Sample data is downloaded from <a>href=http://www.slicer.org/slicerWiki/index.php/SampleData> into the cache directory specified in the application settings under Remote Data Settings.
    """
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper and Danielle Pace.
    """
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # Trigger the menu to be added when application has started up
    qt.QTimer.singleShot(0, self.addMenu);
    
  def addMenu(self):
    i = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
    a = qt.QAction(i, 'Download Sample Data', slicer.util.mainWindow())
    a.setToolTip('Go to the SampleData module to download data from the network')
    a.connect('triggered()', self.select)

    menuFile = slicer.util.lookupTopLevelWidget('menuFile')
    if menuFile:
      for action in menuFile.actions():
        if action.text == 'Add Data':
          menuFile.insertAction(action,a)

  def select(self):
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModuleByTitle('Sample Data')


#
# SampleData widget
#

class SampleDataWidget:

  def __init__(self, parent=None):
    self.observerTags = []

    # This wiki page has the table with all of the sample data and screenshots
    self.sampleDataPage = "http://www.slicer.org/slicerWiki/index.php/SampleData"

    # Handles the network connections
    self.manager = qt.QNetworkAccessManager()
    self.manager.connect('finished(QNetworkReply*)', self.onNetworkResult)
    self.manager.connect('authenticationRequired(QNetworkReply*, QAuthenticator*)', self.onAuthenticationRequired)
    self.manager.connect('proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)', self.onProxyAuthenticationRequired)

    # Contains information on the sample data
    self.sampleNames = []
    self.sampleLinks = []
    self.sampleIconLinks = {}
    self.numSamples = 0

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    pass
    
  def exit(self):
    pass

  def updateGUIFromMRML(self, caller, event):
    pass

  def setup(self):
    # Sample data collapsible button
    dataCollapsibleButton = ctk.ctkCollapsibleButton()
    dataCollapsibleButton.text = "SampleData"
    self.layout.addWidget(dataCollapsibleButton)
    dataFormLayout = qt.QFormLayout(dataCollapsibleButton)

    # Add combobox to select sample data to download
    self.sampleComboBox = qt.QComboBox()
    self.sampleComboBox.connect('currentIndexChanged(int)', self.downloadVolume)
    dataFormLayout.addRow("Data:", self.sampleComboBox)

    # Add refresh button
    self.refreshButton = qt.QPushButton("Refresh")
    self.refreshButton.connect('clicked()', self.updateGUIFromWeb)
    dataFormLayout.addRow(self.refreshButton)

    # Status collapsible button
    statusCollapsibleButton = ctk.ctkCollapsibleButton()
    statusCollapsibleButton.text = "Status"
    self.layout.addWidget(statusCollapsibleButton)
    statusLayout = qt.QVBoxLayout(statusCollapsibleButton)

    # Add status section
    self.log = qt.QTextBrowser()
    statusLayout.addWidget(self.log)

    # Fetch the sample data information and populate the sampleComboBox
    self.updateGUIFromWeb()

  def initiateDownload(self, uri):
    # Triggers call to onNetworkResult()
    reply = self.manager.get(qt.QNetworkRequest(qt.QNetworkRequest(qt.QUrl(uri))))

  def updateGUIFromWeb(self):
    self.reportStatus("<b>Retrieving sample data information from Slicer wiki</b>")
    self.initiateDownload(self.sampleDataPage)

  def onNetworkResult(self, reply):
    # Error handling taken care of by the functions called here.

    if reply.url().toString() == self.sampleDataPage:
      self.handleSampleDataInformation(reply)
    else:
      self.handleIcon(reply)

  def populateSampleDataWidget(self):
    self.sampleComboBox.clear()
    self.sampleComboBox.addItem('Select sample data to download...')
    self.sampleComboBox.insertSeparator(1)

    # Add combobox to select sample data to download (starting with blank screenshot icons in case the icons cannot be downloaded)
    for name in self.sampleNames:
      self.sampleComboBox.addItem('Download %s' % name)

  def setSampleDataWidgetIcon(self, image, uri):
    icon = qt.QIcon(qt.QPixmap().fromImage(image))
    for index in self.sampleIconLinks[uri]:
      self.sampleComboBox.setItemIcon(index, icon)

  def handleSampleDataInformation(self, reply):
    if self.networkReplyHasErrors("accessing Slicer wiki", reply):
      return

    # Get the wiki page
    byteArray = reply.readAll()
    if (byteArray.size() == 0):
      self.reportError("accessing Slicer wiki", "error reading wiki page")
      return

    # Parse the table from the wiki page (there should only be one)
    success = self.parseWikiPage(byteArray)
    if not success:
      return

    # Clear the combobox and re-add the header and separator
    self.populateSampleDataWidget()

    self.reportStatus("Sample data information successfully retrieved")

    # Trigger downloading the screenshots
    for iconUrl in self.sampleIconLinks.keys():
      self.initiateDownload(iconUrl)

    self.reportStatus("Idle")

  def parseWikiPage(self, byteArray):
    # Extract the table
    beginTableString = "<table"
    endTableString = "/table>"
    if (byteArray.count(beginTableString) != 1 or byteArray.count(endTableString) != 1):
      self.reportError("accessing Slicer wiki", "error parsing wiki page")
      return False
    beginTableIndex = byteArray.indexOf(beginTableString)
    endTableIndex = byteArray.indexOf(endTableString)    
    if (beginTableIndex == -1 or endTableIndex == -1):
      self.reportError("accessing Slicer wiki", "error parsing wiki page")
      return False
    tableArray = byteArray.mid(beginTableIndex, endTableIndex-beginTableIndex+len(endTableString))

    # Parse the rows from the wiki page
    beginRowString = "<tr"
    endRowString = "/tr>"
    numRows = tableArray.count(beginRowString)
    if (numRows != tableArray.count(endRowString) or numRows <= 1):
      self.reportError("accessing Slicer wiki", "error parsing wiki page")
      return False

    self.sampleNames = []
    self.sampleLinks = []
    self.sampleIconLinks = {}
    self.numSamples = 0

    for i in range(0, numRows):
      beginRowIndex = tableArray.indexOf(beginRowString)
      endRowIndex = tableArray.indexOf(endRowString)

      if (beginRowIndex == -1 or endRowIndex == -1):
        self.reportError("accessing Slicer wiki", "error parsing wiki page")
        return False

      rowArray = tableArray.mid(beginRowIndex, endRowIndex-beginRowIndex+len(endRowString))
      tableArray = tableArray.right(tableArray.length()-endRowIndex-len(endRowString))

      # Create a list containing tuples of sample name, the data, and the screenshot
      # The first row is the table header, so ignore it
      if i != 0:
        beginDataString = "<a href"
        endDataString = "/a>"

        expectedNumData = 2
        if (rowArray.count(beginDataString) != expectedNumData or rowArray.count(endDataString) != expectedNumData):
          self.reportError("accessing Slicer wiki", "error parsing wiki page")
          return False

        # Extract the link to the data
        beginDataIndex = rowArray.indexOf(beginDataString)
        endDataIndex = rowArray.indexOf(endDataString)	
        dataArray = rowArray.mid(beginDataIndex, endDataIndex-beginDataIndex+len(endDataString))
        rowArray = rowArray.right(rowArray.length()-endDataIndex-len(endDataString))

        beginLinkString = "href=\""
        endLinkString = "\""
        beginLinkIndex = dataArray.indexOf(beginLinkString)
        endLinkIndex = dataArray.indexOf(endLinkString, beginLinkIndex+len(beginLinkString))
        linkArray = dataArray.mid(beginLinkIndex+len(beginLinkString), endLinkIndex-beginLinkIndex-len(beginLinkString))

        # Extract the name of the data
        beginNameString = ">"
        endNameString = "</a>"
        endNameIndex = dataArray.lastIndexOf(endNameString)
        beginNameIndex = dataArray.lastIndexOf(beginNameString, endNameIndex-len(endNameString))
	nameArray = dataArray.mid(beginNameIndex+len(beginNameString), endNameIndex-beginNameIndex-len(beginNameString))

        # Extract the screenshot icon
        beginIconString = "src=\""
        endIconString = "\""
        beginIconIndex = rowArray.indexOf(beginIconString)
        endIconIndex = rowArray.indexOf(endIconString, beginIconIndex+len(beginIconString))
        iconArray = rowArray.mid(beginIconIndex+len(beginIconString), endIconIndex-beginIconIndex-len(beginIconString))
        iconUrl = "http://www.slicer.org" + iconArray.data()
        
        # Add to the list
        self.sampleNames.append(nameArray.data())
        self.sampleLinks.append(linkArray.data())
        iconIndex = self.numSamples + 2 # compensate for explanation text and separator
        if iconUrl in self.sampleIconLinks:
          self.sampleIconLinks[iconUrl].append(iconIndex)
        else:
          self.sampleIconLinks[iconUrl] = [iconIndex]
        self.numSamples = self.numSamples + 1
    return True
    
  def handleIcon(self, reply):
    if self.networkReplyHasErrors("fetching sample data icon", reply):
      return

    image = qt.QImage()
    image.loadFromData(reply.readAll())
    self.setSampleDataWidgetIcon(image, reply.url().toString())

  def reportStatus(self, infoString):
    self.log.insertHtml('<p>Status: <i>%s</i>\n' % infoString)
    self.log.insertPlainText('\n')
    scrollBar = self.log.verticalScrollBar()
    scrollBar.setValue(scrollBar.maximum)
    
  def reportError(self, task, infoString):
    self.log.insertHtml('<p>Error %s: <i>%s</i>\n' % (task, infoString))
    self.log.insertPlainText('\n')

  def networkReplyHasErrors(self, task, reply):
    if reply.error() != qt.QNetworkReply.NoError:
      self.reportError(task, reply.errorString())
      return True
    return False

  def onAuthenticationRequired(self, reply, authenticator):
    self.reportError("Authentication required") # TODO can recover?

  def onProxyAuthenticationRequired(self, proxy, authenticator):
    self.reportError("Proxy authentication required") # TODO can recover?

  def downloadVolume(self, index):
    # Compensate for first two entries in the combobox (explanation text and separator)
    index = index - 2  
    if index >= self.numSamples or index < 0:
      return

    # get the name and uri
    name = self.sampleNames[index]
    uri = self.sampleLinks[index]
    if (not name or not uri):
      return

    # start the download
    self.reportStatus('<b>Requesting download</b> <i>%s</i> from %s...' % (name,uri))
    self.log.repaint()
    slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)
    vl = slicer.modules.volumes.logic()
    volumeNode = vl.AddArchetypeVolume(uri, name, 0)
    if volumeNode:
      storageNode = volumeNode.GetStorageNode()
      storageNode.AddObserver('ModifiedEvent', self.processStorageEvents)
      # Automatically select the volume to display
      self.reportStatus("Displaying...")
      self.log.repaint()
      mrmlLogic = slicer.app.mrmlApplicationLogic()
      selNode = mrmlLogic.GetSelectionNode()
      selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
      mrmlLogic.PropagateVolumeSelection(1)
      self.reportStatus("Finished")
      self.log.repaint()
      self.processStorageEvents(storageNode, 'ModifiedEvent')
    else:
      self.reportStatus('<b>Download failed!</b>')
      self.log.repaint()
    # Reset back to the explanation text
    self.sampleComboBox.setCurrentIndex(0)

  def processStorageEvents(self, node, event):
    state = node.GetReadStateAsString()
    if state == 'TransferDone':
      self.reportStatus("Transfer done")
      self.reportStatus("Idle")
      node.RemoveObserver('ModifiedEvent', self.processStorageEvents)
    else:
      self.reportStatus(state)
    self.log.repaint()
    slicer.app.processEvents(qt.QEventLoop.ExcludeUserInputEvents)

