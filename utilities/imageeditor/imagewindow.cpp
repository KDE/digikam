/* ============================================================
 * File  : imagewindow.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-12
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju, Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// Qt includes.
  
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>

// KDE includes.

#include <klocale.h>
#include <kaction.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kpropertiesdialog.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kprinter.h>
#include <ktempfile.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <libkexif/kexif.h>
#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

// Local includes.

#include "exifrestorer.h"
#include "guifactory.h"
#include "canvas.h"
#include "imageguiclient.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "imagecommentedit.h"
#include "albummanager.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "imagewindow.h"

ImageWindow* ImageWindow::instance()
{
    if (!m_instance)
        new ImageWindow();

    return m_instance;
}

ImageWindow* ImageWindow::m_instance = 0;

ImageWindow::ImageWindow()
    : QMainWindow(0,0,WType_TopLevel|WDestructiveClose)
{
    m_instance = this;

    // -- build the gui -------------------------------------
    
    m_guiFactory = new Digikam::GUIFactory();
    m_guiClient  = new ImageGUIClient(this);
    m_guiFactory->insertClient(m_guiClient);

    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (Digikam::ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) {
        if (plugin) {
            m_guiFactory->insertClient(plugin);
            plugin->setParentWidget(this);
            plugin->setEnabledSelectionActions(false);
        }
    }
    
    m_contextMenu = new QPopupMenu(this);
    m_guiFactory->buildGUI(this);
    m_guiFactory->buildGUI(m_contextMenu);

    // -- construct the view ---------------------------------
    
    m_canvas    = new Canvas(this);
    setCentralWidget(m_canvas);

    statusBar()->setSizeGripEnabled(false);
    m_nameLabel = new QLabel(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_nameLabel,1);
    m_zoomLabel = new QLabel(statusBar());
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_zoomLabel,1);
    m_resLabel  = new QLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_resLabel,1);
    
    // -- setup connections ---------------------------
    
    connect(m_guiClient, SIGNAL(signalNext()),
            SLOT(slotLoadNext()));
    connect(m_guiClient, SIGNAL(signalPrev()),
            SLOT(slotLoadPrev()));
    connect(m_guiClient, SIGNAL(signalFirst()),
            SLOT(slotLoadFirst()));
    connect(m_guiClient, SIGNAL(signalLast()),
            SLOT(slotLoadLast()));
    connect(m_guiClient, SIGNAL(signalExit()),
            SLOT(close()));

    connect(m_guiClient, SIGNAL(signalSave()),
            SLOT(slotSave()));
    connect(m_guiClient, SIGNAL(signalSaveAs()),
            SLOT(slotSaveAs()));
                        
    connect(m_guiClient, SIGNAL(signalRestore()),
            m_canvas, SLOT(slotRestore()));

    connect(m_guiClient, SIGNAL(signalFilePrint()),
            SLOT(slotFilePrint()));
    connect(m_guiClient, SIGNAL(signalFileProperties()),
            SLOT(slotFileProperties()));
    connect(m_guiClient, SIGNAL(signalDeleteCurrentItem()),
            SLOT(slotDeleteCurrentItem()));            
    connect(m_guiClient, SIGNAL(signalExifInfo()),
            SLOT(slotExifInfo()));            
    connect(m_guiClient, SIGNAL(signalCommentsEdit()),
            SLOT(slotCommentsEdit()));            
            
            
    connect(m_guiClient, SIGNAL(signalZoomPlus()),
            m_canvas, SLOT(slotIncreaseZoom()));
    connect(m_guiClient, SIGNAL(signalZoomMinus()),
            m_canvas, SLOT(slotDecreaseZoom()));
    connect(m_guiClient, SIGNAL(signalZoomFit()),
            SLOT(slotToggleAutoZoom()));

    connect(m_guiClient, SIGNAL(signalRotate90()),
            m_canvas, SLOT(slotRotate90()));
    connect(m_guiClient, SIGNAL(signalRotate180()),
            m_canvas, SLOT(slotRotate180()));
    connect(m_guiClient, SIGNAL(signalRotate270()),
            m_canvas, SLOT(slotRotate270()));

    connect(m_guiClient, SIGNAL(signalFlipHoriz()),
            m_canvas, SLOT(slotFlipHoriz()));
    connect(m_guiClient, SIGNAL(signalFlipVert()),
            m_canvas, SLOT(slotFlipVert()));

    connect(m_guiClient, SIGNAL(signalCrop()),
            m_canvas, SLOT(slotCrop()));
    connect(m_guiClient, SIGNAL(signalResize()),
            SLOT(slotResize()));
                            
    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            SLOT(slotContextMenu()));
    connect(m_canvas, SIGNAL(signalZoomChanged(float)),
            SLOT(slotZoomChanged(float)));
    connect(m_canvas, SIGNAL(signalSelected(bool)),
            SLOT(slotSelected(bool)));
    connect(m_canvas, SIGNAL(signalChanged(bool)),
            SLOT(slotChanged(bool)));
    connect(m_canvas, SIGNAL(signalShowNextImage()),
            SLOT(slotLoadNext()));
    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            SLOT(slotLoadPrev()));

    // -- read settings --------------------------------
    readSettings();
}


ImageWindow::~ImageWindow()
{
    m_instance = 0;

    saveSettings();
    delete m_guiClient;
    delete m_guiFactory;

    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (Digikam::ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) {
        if (plugin) {
            plugin->setParentWidget(0);
            plugin->setEnabledSelectionActions(false);
        }
    }
}

void ImageWindow::loadURL(const KURL::List& urlList,
                          const KURL& urlCurrent,
                          const QString& caption)
{
    setCaption(caption);
    
    m_urlList    = urlList;
    m_urlCurrent = urlCurrent;
    
    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::readSettings()
{
    KConfig* config = kapp->config();
    
    int width, height;
    bool autoZoom;
    bool fullScreen;
    
    config->setGroup("ImageViewer Settings");
    width = config->readNumEntry("Width", 500);
    height = config->readNumEntry("Height", 500);
    autoZoom = config->readBoolEntry("AutoZoom", true);
    fullScreen = config->readBoolEntry("FullScreen", false);
    //config->setGroup("EXIF Settings");
    //setExifOrientation = config->readBoolEntry("EXIF Set Orientation", true);

    resize(width, height);

    if (autoZoom) {
        m_guiClient->m_zoomFitAction->activate();
        m_guiClient->m_zoomPlusAction->setEnabled(false);
        m_guiClient->m_zoomMinusAction->setEnabled(false);
    }
//     if (fullScreen)
//         d->bFullScreen->animateClick();
}

void ImageWindow::saveSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("Width", width());
    config->writeEntry("Height", height());
    config->writeEntry("AutoZoom", m_guiClient->m_zoomFitAction->isChecked());
    //config->writeEntry("FullScreen", d->bFullScreen->isOn());
    config->sync();
}

void ImageWindow::slotLoadCurrent()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    uint index = m_urlList.findIndex(m_urlCurrent);

    if (it != m_urlList.end()) {

        m_canvas->load(m_urlCurrent.path());

        QString text = m_urlCurrent.filename() + 
                       i18n(" (%2 of %3)")
                       .arg(QString::number(index+1))
                       .arg(QString::number(m_urlList.count()));
        m_nameLabel->setText(text);
    }

    if (m_urlList.count() == 1) {
        m_guiClient->m_navPrevAction->setEnabled(false);
        m_guiClient->m_navNextAction->setEnabled(false);
        m_guiClient->m_navFirstAction->setEnabled(false);
        m_guiClient->m_navLastAction->setEnabled(false);
    }
    else {
        m_guiClient->m_navPrevAction->setEnabled(true);
        m_guiClient->m_navNextAction->setEnabled(true);
        m_guiClient->m_navFirstAction->setEnabled(true);
        m_guiClient->m_navLastAction->setEnabled(true);
    }
    
    if (index == 0) {
        m_guiClient->m_navPrevAction->setEnabled(false);
        m_guiClient->m_navFirstAction->setEnabled(false);
    }
        
    if (index == m_urlList.count()-1) {
        m_guiClient->m_navNextAction->setEnabled(false);
        m_guiClient->m_navLastAction->setEnabled(false);
    }

}

void ImageWindow::slotLoadNext()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end()) {

        if (m_urlCurrent != m_urlList.last()) {
           KURL urlNext = *(++it);
           m_urlCurrent = urlNext;
           slotLoadCurrent();
        }
    }
}

void ImageWindow::slotLoadPrev()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.begin()) {

        if (m_urlCurrent != m_urlList.first()) 
        {
            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            slotLoadCurrent();
        }
    }
}


void ImageWindow::slotLoadFirst()
{
    m_urlCurrent = m_urlList.first();
    slotLoadCurrent();
}

void ImageWindow::slotLoadLast()
{
    m_urlCurrent = m_urlList.last();
    slotLoadCurrent();
}

void ImageWindow::slotToggleAutoZoom()
{
    bool checked = m_guiClient->m_zoomFitAction->isChecked();

    m_guiClient->m_zoomPlusAction->setEnabled(!checked);
    m_guiClient->m_zoomMinusAction->setEnabled(!checked);

    m_canvas->slotToggleAutoZoom();
}

void ImageWindow::slotResize()
{
    int width  = m_canvas->imageWidth();
    int height = m_canvas->imageHeight();

    ImageResizeDlg dlg(this, &width, &height);
    if (dlg.exec() == QDialog::Accepted && 
        (width != m_canvas->imageWidth() ||
        height != m_canvas->imageHeight())) 
        m_canvas->resize(width, height);
}

void ImageWindow::slotContextMenu()
{
    m_contextMenu->exec(QCursor::pos());    
}

void ImageWindow::slotZoomChanged(float zoom)
{
    m_zoomLabel->setText(i18n("Zoom: ") +
                         QString::number(zoom*100, 'f', 2) +
                         QString("%"));

    m_guiClient->m_zoomPlusAction->
        setEnabled(!m_canvas->maxZoom() &&
                   !m_guiClient->m_zoomFitAction->isChecked());
    m_guiClient->m_zoomMinusAction->
        setEnabled(!m_canvas->minZoom() &&
                   !m_guiClient->m_zoomFitAction->isChecked());
}

void ImageWindow::slotChanged(bool val)
{
    m_resLabel->setText(QString::number(m_canvas->imageWidth())  +
                        QString("x") +
                        QString::number(m_canvas->imageHeight()) +
                        QString(" ") +
                        i18n("pixels"));

    m_guiClient->m_restoreAction->setEnabled(val);
    m_guiClient->m_saveAction->setEnabled(val);
}

void ImageWindow::slotSelected(bool val)
{
    m_guiClient->m_cropAction->setEnabled(val);

    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (Digikam::ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) {
        if (plugin) {
            m_guiFactory->insertClient(plugin);
            plugin->setEnabledSelectionActions(val);
        }
    }
}

void ImageWindow::slotFileProperties()
{
    (void) new KPropertiesDialog( m_urlCurrent, this, "props dialog", true );
}

void ImageWindow::slotExifInfo()
{
    KExif *exif = new KExif(this);
    
    if (exif->loadFile(m_urlCurrent.path()) == 0)
        exif->show();
    else 
        {
        delete exif;
        KMessageBox::sorry(this,
                           i18n("This item has no Exif Information"));
        }
}

void ImageWindow::slotCommentsEdit()
{
    KURL u(m_urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
        return;
    
    AlbumDB* db = AlbumManager::instance()->albumDB();
    QString comments( db->getItemCaption(palbum, m_urlCurrent.fileName()) );
  
    if (ImageCommentEdit::editComments(m_urlCurrent.filename(), comments, this)) 
    {
        db->setItemCaption(palbum, m_urlCurrent.fileName(), comments);
    
        AlbumSettings *settings = AlbumSettings::instance();
        
        if (settings->getSaveExifComments())
        {
            KFileMetaInfo metaInfo(m_urlCurrent, "image/jpeg", KFileMetaInfo::Fastest);
            if (metaInfo.isValid () && metaInfo.mimeType() == "image/jpeg")
            {
                // store as JPEG JFIF comment
                if (metaInfo.containsGroup("Jpeg EXIF Data"))
                {
                    metaInfo["Jpeg EXIF Data"].item("Comment").setValue(comments);
                    metaInfo.applyChanges();
                }
            }
        }
    }       
}

void ImageWindow::slotDeleteCurrentItem()
{
    QString warnMsg(i18n("About to Delete File \"%1\"\nAre you sure?")
                    .arg(m_urlCurrent.filename()));

    if (KMessageBox::warningContinueCancel(this,
                                           warnMsg,
                                           i18n("Warning"),
                                           i18n("Delete"))
        !=  KMessageBox::Continue)
    {
        return;
    }

    if (!KIO::NetAccess::del(m_urlCurrent, this))
    {
        QString errMsg(KIO::NetAccess::lastErrorString());
        KMessageBox::error(this, errMsg, errMsg);
        return;
    }

    emit signalFileDeleted(m_urlCurrent);
    
    KURL CurrentToRemove = m_urlCurrent;
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end())
    {
        if (m_urlCurrent != m_urlList.last())
        {
            // Try to get the next image in the current Album...
          
            KURL urlNext = *(++it);
            m_urlCurrent = urlNext;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
        else if (m_urlCurrent != m_urlList.first())
        {
            // Try to get the previous image in the current Album...

            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
    }

    // No image in the current Album -> Quit ImageEditor...
    
    KMessageBox::information(this,
                             i18n("There is no image to show in the current Album!\n"
                                  "The ImageEditor will be closed..."),
                             i18n("No image in the current Album"));
    
    close();
}

void ImageWindow::slotFilePrint()
{
    KPrinter printer;
    printer.setDocName( m_urlCurrent.filename() );
    printer.setCreator( "Digikam-ImageEditor");

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage( this, "ImageEditor page"));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
        {
        KTempFile tmpFile( "digikam_imageeditor", ".png" );
        
        if ( tmpFile.status() == 0 )
            {
            tmpFile.setAutoDelete( true );
            
            if ( m_canvas->save(tmpFile.name()) )
               ImagePrint printOperations(tmpFile.name(), printer, m_urlCurrent.filename());
            }
        }
}

void ImageWindow::slotSave()
{
    QString tmpFile = locateLocal("tmp", m_urlCurrent.filename());
    
    bool result = m_canvas->save(tmpFile);

    if (result == false) 
        {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to Album\n\"%2\"")
                                 .arg(m_urlCurrent.filename())
                                 .arg(m_urlCurrent.path().section('/', -2, -2)));
        return;
        }

    ExifRestorer exifHolder;
    exifHolder.readFile(m_urlCurrent.path(), ExifRestorer::ExifOnly);

    if (exifHolder.hasExif()) 
       {
       ExifRestorer restorer;
       restorer.readFile(tmpFile, ExifRestorer::EntireImage);
       restorer.insertExifData(exifHolder.exifData());
       restorer.writeFile(tmpFile);
       }
    else 
        kdWarning() << ("No Exif Data Found") << endl;
    
    if( rotatedOrFlipped )
       KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);

    KIO::FileCopyJob* job = KIO::file_move(KURL(tmpFile), m_urlCurrent,
                                           -1, true, false, false);

    connect(job, SIGNAL(result(KIO::Job *) ),
            this, SLOT(slotSaveResult(KIO::Job *)));
}

void ImageWindow::slotSaveResult(KIO::Job *job)
{
    if (job->error()) 
       {
       job->showErrorDialog(this);
       return;
       }
}

void ImageWindow::slotSaveAs()
 {
    // Get the new filename. 
    
    // PENDING (Gilles): we must used the list of Imlib2 files format supported instead (provide by the 
    // imlib2 plugins installed in the system).
    
    QStringList mimetypes = KImageIO::mimeTypes( KImageIO::Writing );
     
    m_newFile = KURL(KFileDialog::getSaveFileName(m_urlCurrent.directory(),
                                                mimetypes.join(" "),
                                                this,
                                                i18n("New image filename")));
 
    // Check for cancel.
     
    if (!m_newFile.isValid()) return;
 
    QString tmpFile = locateLocal("tmp", m_urlCurrent.filename());
     
    int result = m_canvas->saveAs(tmpFile);
 
    if (result != 1) 
       {
       KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to Album\n\"%2\"")
                          .arg(m_newFile.filename())
                          .arg(m_newFile.path().section('/', -2, -2)));
       return;
       }
     
    ExifRestorer exifHolder;
    exifHolder.readFile(m_urlCurrent.path(), ExifRestorer::ExifOnly);
 
    if (exifHolder.hasExif()) 
       {
       ExifRestorer restorer;
       restorer.readFile(tmpFile, ExifRestorer::EntireImage);
       restorer.insertExifData(exifHolder.exifData());
       restorer.writeFile(tmpFile);
       }
    else 
       kdWarning() << ("No Exif Data Found") << endl;
    
    if( rotatedOrFlipped )
       KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);
    
    KIO::FileCopyJob* job = KIO::file_move(KURL(tmpFile), m_newFile,
                                           -1, true, false, false);
  
    connect(job, SIGNAL(result(KIO::Job *) ),
            this, SLOT(slotSaveAsResult(KIO::Job *)));
}

void ImageWindow::slotSaveAsResult(KIO::Job *job)
{
    if (job->error()) 
       {
       job->showErrorDialog(this);
       return;
       }

    // Added new file URL into list if the new file have been added in the current Album
    // and added the comments if exists.

    KURL su(m_urlCurrent.directory());
    PAlbum *sourcepAlbum = AlbumManager::instance()->findPAlbum(su);

    if (!sourcepAlbum)
        return;

    KURL tu(m_newFile.directory());
    PAlbum *targetpAlbum = AlbumManager::instance()->findPAlbum(tu);

    if (!targetpAlbum)
        return;

    // Copy the comments from the original image to the target image.
                    
    AlbumDB* db = AlbumManager::instance()->albumDB();
    QString comments( db->getItemCaption(sourcepAlbum, m_urlCurrent.fileName()) );
    
    db->setItemCaption(targetpAlbum, m_newFile.fileName(), comments);
    
    if ( m_urlCurrent.directory() == m_newFile.directory() &&  // Target Album = current Album ?
         m_urlList.find(m_newFile) == m_urlList.end() )        // The image file not already exist
       {                                                       // in the list.            
       m_canvas->slotRestore();
       m_canvas->load(m_newFile.path());
       KURL::List::iterator it = m_urlList.find(m_urlCurrent);
       m_urlList.insert(it, m_newFile);
       m_urlCurrent = m_newFile;
       }
    
    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));      // Load the new target images.
}


#include "imagewindow.moc"

