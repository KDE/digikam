/* ============================================================
 * File  : imagewindow.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-12
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>

#include <klocale.h>
#include <kaction.h>
#include <kconfig.h>
#include <kapplication.h>

#include "guifactory.h"

#include "canvas.h"
#include "imageguiclient.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
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
    m_guiClient  = new ImageGUIClient();
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

    connect(m_guiClient, SIGNAL(signalRestore()),
            m_canvas, SLOT(slotRestore()));
    
    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            SLOT(slotContextMenu()));
    connect(m_canvas, SIGNAL(signalZoomChanged(float)),
            SLOT(slotZoomChanged(float)));
    connect(m_canvas, SIGNAL(signalSelected(bool)),
            SLOT(slotSelected(bool)));
    connect(m_canvas, SIGNAL(signalChanged(bool)),
            SLOT(slotChanged(bool)));

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
                          const KURL& urlCurrent)
{
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
        m_guiClient->m_zoomFitAction->setChecked(true);
        slotToggleAutoZoom();
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

    ImageResizeDlg dlg(&width, &height);
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

    m_guiClient->m_zoomPlusAction->setEnabled(!m_canvas->maxZoom());
    m_guiClient->m_zoomMinusAction->setEnabled(!m_canvas->minZoom());
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


#include "imagewindow.moc"
