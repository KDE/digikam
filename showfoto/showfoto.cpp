/* ============================================================
 * File  : showfoto.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
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
 
#include <qlayout.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>

// KDE includes.

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kaccel.h>
#include <kpropertiesdialog.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kio/netaccess.h>

#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

#include <cstdio>
#include <unistd.h>

// Local includes.

#include "exifrestorer.h"
#include "canvas.h"
#include "thumbbar.h"
#include "showfoto.h"

ShowFoto::ShowFoto(const KURL::List& urlList)
{
    m_config = kapp->config();
    m_fullScreen = false;
    
    QWidget* widget = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);

    m_canvas = new Canvas(widget);
    m_bar    = new ThumbBarView(widget);
    lay->addWidget(m_canvas);
    lay->addWidget(m_bar);

    setCentralWidget(widget);
    setupActions();
    applySettings();

    connect(m_bar, SIGNAL(signalURLSelected(const KURL&)),
            SLOT(slotOpenURL(const KURL&)));
    connect(m_canvas, SIGNAL(signalSelected(bool)),
            m_cropAction, SLOT(setEnabled(bool)));
    connect(m_canvas, SIGNAL(signalChanged(bool)),
            m_saveAction, SLOT(setEnabled(bool)));
    connect(m_canvas, SIGNAL(signalChanged(bool)),
            m_saveAsAction, SLOT(setEnabled(bool)));
    connect(m_canvas, SIGNAL(signalChanged(bool)),
            m_revertAction, SLOT(setEnabled(bool)));
    connect(m_canvas, SIGNAL(signalChanged(bool)),
            m_undoAction, SLOT(setEnabled(bool)));

    for (KURL::List::const_iterator it = urlList.begin();
         it != urlList.end(); ++it)
    {
        new ThumbBarItem(m_bar, *it);
    }

    resize(640,480);
    setAutoSaveSettings();
}

ShowFoto::~ShowFoto()
{
    saveSettings();
    delete m_bar;
    delete m_canvas;
}

void ShowFoto::setupActions()
{

    KStdAction::open(this, SLOT(slotOpenFile()),
                     actionCollection(), "open_file");
    KStdAction::quit(this, SLOT(close()),
                     actionCollection());
    KStdAction::forward(this, SLOT(slotNext()),
                        actionCollection(), "go_fwd");
    KStdAction::back(this, SLOT(slotPrev()),
                     actionCollection(), "go_bwd");

    new KAction(i18n("Properties"), 0,
                ALT+Key_Return,
                this, SLOT(slotFileProperties()),
                actionCollection(), "file_properties");

    m_revertAction = KStdAction::revert(m_canvas, SLOT(slotRestore()),
                                        actionCollection(), "revert");

    m_undoAction = KStdAction::undo(m_canvas, SLOT(slotUndo()),
                                    actionCollection(), "undo");
    
    m_redoAction = KStdAction::redo(m_canvas, SLOT(slotRedo()),
                                    actionCollection(), "redo");

    m_saveAction   = KStdAction::save(this, SLOT(slotSave()),
                                      actionCollection(), "save");
    
    m_saveAsAction  = KStdAction::saveAs(this, SLOT(slotSaveAs()),
                                         actionCollection(), "saveas");

    m_revertAction->setEnabled(false);
    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);
                     
    // ---------------------------------------------------------------
    
    m_zoomPlusAction =
        KStdAction::zoomIn(m_canvas, SLOT(slotIncreaseZoom()),
                           actionCollection(), "zoom_plus");
    m_zoomMinusAction =
        KStdAction::zoomOut(m_canvas, SLOT(slotDecreaseZoom()),
                            actionCollection(), "zoom_minus");
    m_zoomFitAction
        = new KToggleAction(i18n("Zoom &AutoFit"), "viewmagfit",
                            Key_A,
                            this, SLOT(slotAutoFit()),
                            actionCollection(), "zoom_fit");

#if KDE_IS_VERSION(3,2,0)
    m_fullScreenAction =
        KStdAction::fullScreen(this, SLOT(slotToggleFullScreen()),
                               actionCollection(), this, "full_screen");
#else 
    m_fullScreenAction =
        new KToggleAction(i18n("Fullscreen"), "window_fullscreen",
                          CTRL+SHIFT+Key_F, this,
                          SLOT(slotToggleFullScreen()),
                          actionCollection(), "full_screen");
#endif

    m_showBarAction =
        new KToggleAction(i18n("Hide thumbnails"), 0, Key_T,
                          this, SLOT(slotToggleShowBar()),
                          actionCollection(), "show_thumbs");

    m_viewHistogramAction = 
        new KSelectAction(i18n("View &Histogram"), 0, Key_H,
                          this, SLOT(slotViewHistogram()),
                          actionCollection(), "view_histogram");
    m_viewHistogramAction->setEditable(false);
    
    QStringList selectItems;
    selectItems << i18n("Hide");
    selectItems << i18n("Luminosity");
    selectItems << i18n("Red");
    selectItems << i18n("Green");
    selectItems << i18n("Blue");
    selectItems << i18n("Alpha");
    m_viewHistogramAction->setItems(selectItems);

    // ---------------------------------------------------------------
    
    new KAction(i18n("Rotate 90"), 0, Key_9,
                m_canvas, SLOT(slotRotate90()),
                actionCollection(), "rotate_90");
    new KAction(i18n("Rotate 180"), 0, Key_8,
                m_canvas, SLOT(slotRotate180()),
                actionCollection(), "rotate_180");
    new KAction(i18n("Rotate 270"), 0, Key_7,
                m_canvas, SLOT(slotRotate270()),
                actionCollection(), "rotate_270");

    new KAction(i18n("Flip Horizontally"), 0, Key_Asterisk,
                m_canvas, SLOT(slotFlipHoriz()),
                actionCollection(), "flip_horiz");
    new KAction(i18n("Flip Vertically"), 0, Key_Slash,
                m_canvas, SLOT(slotFlipVert()),
                actionCollection(), "flip_vert");

    m_cropAction = new KAction(i18n("Crop"), "crop",
                               CTRL+Key_C,
                               m_canvas, SLOT(slotCrop()),
                               actionCollection(), "crop");
    m_cropAction->setEnabled(false);

    // ---------------------------------------------------------------

    new KAction(i18n("Increase Gamma"), 0, Key_G,
                this, SLOT(slotChangeBCG()),
                actionCollection(), "gamma_plus");
    new KAction(i18n("Decrease Gamma"), 0, SHIFT+Key_G,
                this, SLOT(slotChangeBCG()),
                actionCollection(), "gamma_minus");
    new KAction(i18n("Increase Brightness"), 0, Key_B,
                this, SLOT(slotChangeBCG()),
                actionCollection(), "brightness_plus");
    new KAction(i18n("Decrease Brightness"), 0, SHIFT+Key_B,
                this, SLOT(slotChangeBCG()),
                actionCollection(), "brightness_minus");
    new KAction(i18n("Increase Contrast"), 0, Key_C,
                this, SLOT(slotChangeBCG()),
                actionCollection(), "contrast_plus");
    new KAction(i18n("Decrease Contrast"), 0, SHIFT+Key_C,
                this, SLOT(slotChangeBCG()),
                actionCollection(), "contrast_minus");
    
    // ---------------------------------------------------------------
    
    createGUI("showfotoui.rc", false);

    KAccel *accel = new KAccel(this);
    accel->insert("Exit fullscreen", i18n("Exit Fullscreen"),
                  i18n("Exit out of the fullscreen mode"),
                  Key_Escape, this, SLOT(slotEscapePressed()),
                  false, true);
    accel->insert("Next Image Key_Space", i18n("Next Image"),
                  i18n("Load Next Image"),
                  Key_Space, this, SLOT(slotNext()),
                  false, true);
    accel->insert("Previous Image Key_Backspace", i18n("Previous Image"),
                  i18n("Load Previous Image"),
                  Key_Backspace, this, SLOT(slotPrev()),
                  false, true);
    accel->insert("Next Image Key_Next", i18n("Next Image"),
                  i18n("Load Next Image"),
                  Key_Next, this, SLOT(slotNext()),
                  false, true);
    accel->insert("Previous Image Key_Prior", i18n("Previous Image"),
                  i18n("Load Previous Image"),
                  Key_Prior, this, SLOT(slotPrev()),
                  false, true);
    accel->insert("Zoom Plus Key_Plus", i18n("Zoom In"),
                  i18n("Zoom into Image"),
                  Key_Plus, m_canvas, SLOT(slotIncreaseZoom()),
                  false, true);
    accel->insert("Zoom Plus Key_Minus", i18n("Zoom Out"),
                  i18n("Zoom out of Image"),
                  Key_Minus, m_canvas, SLOT(slotDecreaseZoom()),
                  false, true);
}

void ShowFoto::applySettings()
{
    bool showBar = false;
    bool autoFit = true;
    
    m_config->setGroup("MainWindow");
    showBar = m_config->readBoolEntry("Show Thumbnails", true);
    autoFit = m_config->readBoolEntry("Zoom Autofit", true);

    if (!showBar)
        m_showBarAction->activate();

    if (autoFit)
        m_zoomFitAction->activate();

    QRect histogramRect = m_config->readRectEntry("Histogram Rectangle");
    if (!histogramRect.isNull())
        m_canvas->setHistogramPosition(histogramRect.topLeft());
    
    int histogramType = m_config->readNumEntry("HistogramType", 0);
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    m_viewHistogramAction->setCurrentItem(histogramType);
    slotViewHistogram(); // update
}

void ShowFoto::saveSettings()
{
    m_config->setGroup("MainWindow");
    m_config->writeEntry("Show Thumbnails", !m_showBarAction->isChecked());
    m_config->writeEntry("Zoom Autofit", m_zoomFitAction->isChecked());
    
    int histogramType = m_viewHistogramAction->currentItem();
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    m_config->writeEntry("HistogramType", histogramType);

    QPoint pt;
    QRect rc(0, 0, 0, 0);
    if (m_canvas->getHistogramPosition(pt)) 
        rc = QRect(pt.x(), pt.y(), 1, 1);
    m_config->writeEntry("Histogram Rectangle", rc);
}

void ShowFoto::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    if (!promptUserSave())
        return;
    
    e->accept();
}

void ShowFoto::slotOpenFile()
{
    if (!promptUserSave())
        return;
    
    QString mimes = KImageIO::mimeTypes(KImageIO::Reading).join(" ");
    KURL::List urls =  KFileDialog::getOpenURLs(QString::null,
                                                mimes,
                                                this,
                                                i18n("Open images"));
        
    if (!urls.isEmpty())
    {
        m_bar->clear();
        for (KURL::List::const_iterator it = urls.begin();
             it != urls.end(); ++it)
        {
            new ThumbBarItem(m_bar, *it);
        }
    }
}

void ShowFoto::slotFileProperties()
{
    ThumbBarItem* curr = m_bar->currentItem();
    
    if (curr)
        (void) new KPropertiesDialog( curr->url(), this, "props dialog", true );
}

void ShowFoto::slotNext()
{
    if (!promptUserSave())
        return;

    ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->next())
    {
        m_bar->setSelected(curr->next());
    }
}

void ShowFoto::slotPrev()
{
    if (!promptUserSave())
        return;

    ThumbBarItem* curr = m_bar->currentItem();
    if (curr && curr->prev())
    {
        m_bar->setSelected(curr->prev());
    }
}

void ShowFoto::slotSave()
{
    save();
}

void ShowFoto::slotSaveAs()
{
    ThumbBarItem* curr = m_bar->currentItem();
    if (!curr)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return;
    }
    
    KURL url(curr->url());
    
    // The typemines listed is the base imagefiles supported by imlib2.
    QStringList mimetypes;
    mimetypes << "image/jpeg" << "image/png" << "image/tiff" << "image/gif"
              << "image/x-tga" << "image/x-bmp" <<  "image/x-xpm"
              << "image/x-portable-anymap";

    KFileDialog saveDialog(url.isLocalFile() ? url.directory() : QDir::homeDirPath(),
                           QString::null,
                           this,
                           "imageFileSaveDialog",
                           false);
    saveDialog.setOperationMode( KFileDialog::Saving );
    saveDialog.setMode( KFile::File );
    saveDialog.setCaption( i18n("New Image File Name") );
    saveDialog.setMimeFilter(mimetypes);

    if ( saveDialog.exec() != KFileDialog::Accepted )
    {
       return;
    }

    KURL saveAsURL = saveDialog.selectedURL();
    QString format = KImageIO::typeForMime(saveDialog.currentMimeFilter());

    if (!saveAsURL.isValid())
    {
        KMessageBox::error(this, i18n("Invalid target selected"));
        return;
    }

    url.cleanPath();
    saveAsURL.cleanPath();

    if (url.equals(saveAsURL))
    {
        slotSave();
        return;
    }

    QFileInfo fi(saveAsURL.path());
    if ( fi.exists() )
    {
        int result =
            KMessageBox::warningYesNo( this, i18n("About to overwrite file %1. "
                                                  "Are you sure you want to continue?")
                                       .arg(saveAsURL.filename()) );

        if (result != KMessageBox::Yes)
            return;
    }

    QString tmpFile = saveAsURL.directory() + QString("/.showfoto-tmp-")
                      + saveAsURL.filename();
    if (!m_canvas->saveAsTmpFile(tmpFile, 85, 1, true, format.lower()))
    {
        KMessageBox::error(this, i18n("Failed to save file '%1'")
                           .arg(saveAsURL.filename()));
        ::unlink(QFile::encodeName(tmpFile));
        return;
    }

    // only try to write exif if both src and destination are jpeg files
    if (QString(QImageIO::imageFormat(url.path())).upper() == "JPEG" &&
        format.upper() == "JPEG")
    {
        ExifRestorer exifHolder;
        exifHolder.readFile(url.path(), ExifRestorer::ExifOnly);

        if (exifHolder.hasExif())
        {
            ExifRestorer restorer;
            restorer.readFile(tmpFile, ExifRestorer::EntireImage);
            restorer.insertExifData(exifHolder.exifData());
            restorer.writeFile(tmpFile);
            KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);
        }
        else
            kdDebug() << ("slotSaveAs::No Exif Data Found") << endl;
    }

    kdDebug() << "renaming to " << saveAsURL.path() << endl;
    if (::rename(QFile::encodeName(tmpFile),
                 QFile::encodeName(saveAsURL.path())) != 0)
    {
        KMessageBox::error(this, i18n("Failed to overwrite original file"));
        ::unlink(QFile::encodeName(tmpFile));
        return;
    }

    // add the file to the list of images if it's not there already
    ThumbBarItem* foundItem = 0;
    for (ThumbBarItem *item = m_bar->firstItem(); item; item = item->next())
    {
        if (item->url().equals(saveAsURL))
        {
            foundItem = item;
            m_bar->invalidateThumb(item);
            break;
        }
    }

    if (!foundItem)
    {
        foundItem = new ThumbBarItem(m_bar, saveAsURL);
    }

    m_bar->setSelected(foundItem);
}

bool ShowFoto::promptUserSave()
{
    ThumbBarItem* curr = m_bar->currentItem();
    if (!curr)
        return true;
    
    if (m_saveAction->isEnabled())
    {
        int result =
            KMessageBox::warningYesNoCancel(this,
                                            i18n("The image '%1\' has been modified.\n"
                                                 "Do you want to save it?")
                                                 .arg(curr->url().filename()),
                                            QString::null,
                                            KStdGuiItem::save(),
                                            KStdGuiItem::discard());
        if (result == KMessageBox::Yes)
        {
            return save();
        }
        else if (result == KMessageBox::No)
        {
            return true;
        }
        else
            return false;
    }
    return true;
}

bool ShowFoto::save()
{
    ThumbBarItem* curr = m_bar->currentItem();
    if (!curr)
    {
        kdWarning() << k_funcinfo << "This should not happen" << endl;
        return true;
    }

    KURL url = curr->url();
    if (!url.isLocalFile())
    {
        KMessageBox::sorry(this, i18n("No support for saving non-local files"));
        return false;
    }
    
    QString tmpFile = url.directory() + QString("/.showfoto-tmp-")
                      + url.filename();
    if (!m_canvas->saveAsTmpFile(tmpFile, 85, 1, true))
    {
        KMessageBox::error(this, i18n("Failed to save file '%1'")
                           .arg(url.filename()));
        ::unlink(QFile::encodeName(tmpFile));
        return false;
    }

    ExifRestorer exifHolder;
    exifHolder.readFile(url.path(), ExifRestorer::ExifOnly);
    if (exifHolder.hasExif())
    {
        ExifRestorer restorer;
        restorer.readFile(tmpFile, ExifRestorer::EntireImage);
        restorer.insertExifData(exifHolder.exifData());
        restorer.writeFile(tmpFile);
    }
    else
        kdDebug() << ("slotSave::No Exif Data Found") << endl;

    if ( m_canvas->exifRotated() )
        KExifUtils::writeOrientation(tmpFile, KExifData::NORMAL);

    kdDebug() << "renaming to " << url.path() << endl;
    if (::rename(QFile::encodeName(tmpFile),
                 QFile::encodeName(url.path())) != 0)
    {
        KMessageBox::error(this, i18n("Failed to overwrite original file"));
        ::unlink(QFile::encodeName(tmpFile));
        return false;
    }

    slotOpenURL(curr->url());
    m_bar->invalidateThumb(curr);
    
    return true;
}


void ShowFoto::slotOpenURL(const KURL& url)
{
    QString localFile;
#if KDE_IS_VERSION(3,2,0)
    KIO::NetAccess::download(url, localFile, this);
#else
    KIO::NetAccess::download(url, localFile);
#endif
    m_canvas->load(localFile);
}

void ShowFoto::slotAutoFit()
{
    bool checked = m_zoomFitAction->isChecked();

    m_zoomPlusAction->setEnabled(!checked);
    m_zoomMinusAction->setEnabled(!checked);

    m_canvas->slotToggleAutoZoom();
}

void ShowFoto::slotToggleFullScreen()
{
    if (m_fullScreen)
    {

#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        QObject* obj = child("mainToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            toolBar->show();
        }
    
        m_fullScreen = false;
    }
    else
    {
        // hide the menubar and the statusbar
        menuBar()->hide();

        QObject* obj = child("mainToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            toolBar->hide();
        }

        showFullScreen();
        m_fullScreen = true;
    }
}

void ShowFoto::slotEscapePressed()
{
    if (!m_fullScreen)
        return;

    m_fullScreenAction->activate();
}

void ShowFoto::slotToggleShowBar()
{
    if (m_showBarAction->isChecked())
    {
        m_bar->hide();
    }
    else
    {
        m_bar->show();
    }
}

void ShowFoto::slotViewHistogram()
{
    int curItem = m_viewHistogramAction->currentItem();
    m_canvas->setHistogramType(curItem);
}

void ShowFoto::slotChangeBCG()
{
    QString name;
    if (sender())
        name = sender()->name();

    if (name == "gamma_plus")
    {
        m_canvas->increaseGamma();
    }
    else if  (name == "gamma_minus")
    {
        m_canvas->decreaseGamma();
    }
    else if  (name == "brightness_plus")
    {
        m_canvas->increaseBrightness();
    }
    else if  (name == "brightness_minus")
    {
        m_canvas->decreaseBrightness();
    }
    else if  (name == "contrast_plus")
    {
        m_canvas->increaseContrast();
    }
    else if  (name == "contrast_minus")
    {
        m_canvas->decreaseContrast();
    }
}

#include "showfoto.moc"
