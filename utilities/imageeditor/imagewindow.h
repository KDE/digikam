/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-12
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju, Gilles Caulier
 *
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

#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

// Qt includes.

#include <qmainwindow.h>
#include <qstring.h>

// Kde includes.

#include <kurl.h>
#include <kio/job.h>

class QPopupMenu;
class QLabel;

class KAccel;
class KAction;

class ImageGUIClient;
class Canvas;
class AlbumIconView;

namespace Digikam
{
class GUIFactory;
}

class ImageWindow : public QMainWindow
{
    Q_OBJECT

public:

    ~ImageWindow();

    void loadURL(const KURL::List& urlList, const KURL& urlCurrent,
                 const QString& caption=QString::null,
                 bool allowSaving=true,
                 AlbumIconView* view=0L);
                 
    void applySettings();
    
    static ImageWindow* instance();
    
private:

    ImageGUIClient*      m_guiClient;
    
    Digikam::GUIFactory* m_guiFactory;
    
    Canvas*              m_canvas;
    
    QPopupMenu*          m_contextMenu;
    
    KAccel*              m_accel;

    QLabel*              m_nameLabel;
    QLabel*              m_zoomLabel;
    QLabel*              m_resLabel;
    
    KURL::List           m_urlList;
    KURL                 m_urlCurrent;
    KURL                 m_newFile;
    
    // Allow to use Image properties and Comments/Tags dialogs from main window.
    AlbumIconView*       m_view;
    
    int                  m_JPEGCompression;
    int                  m_PNGCompression;
    
    bool                 m_TIFFCompression;
    bool                 m_rotatedOrFlipped;
    bool                 m_fullScreen;
    bool                 m_fullScreenHideToolBar;
    bool                 m_allowSaving;
    
    static ImageWindow*  m_instance;

private:

    ImageWindow();
    void readSettings();
    void saveSettings();
    bool promptUserSave();
    void plugActionAccel(KAction* action);
    void unplugActionAccel(KAction* action);
    bool save();

signals:

    void signalFileDeleted(const KURL& url);
    void signalFileAdded(const KURL& url);
    void signalFileModified(const KURL& url);
    
private slots:

    void slotLoadCurrent();
    
    void slotLoadNext();
    void slotLoadPrev();
    void slotLoadFirst();
    void slotLoadLast();

    void slotToggleAutoZoom();
    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotResize();
    
    void slotContextMenu();
    void slotZoomChanged(float zoom);
    void slotChanged(bool);
    void slotSelected(bool);

    void slotRotatedOrFlipped();
    
    void slotSave();
    void slotSaveAs();
    void slotSaveAsResult(KIO::Job *job);
    
    void slotFilePrint();
    void slotFileProperties();
    void slotCommentsEdit();
    
    void slotDeleteCurrentItem();

    void slotImagePluginsHelp();
    
protected:

    void closeEvent(QCloseEvent *e);

};

#endif /* IMAGEWINDOW_H */
