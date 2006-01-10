/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-12
 * Description : digiKam image editor GUI
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

#include <qstring.h>

// Kde includes.

#include <kmainwindow.h>
#include <kurl.h>

class QPopupMenu;
class QLabel;
class QStringList;
class QSplitter;

class KAccel;
class KAction;
class KActionMenu;
class KToolBarPopupAction;
class KToggleAction;
class KSelectAction;

namespace Digikam
{

class AlbumIconView;
class Canvas;
class ImagePropertiesSideBarDB;
class ICCSettingsContainer;
class IOFileSettingsContainer;

class ImageWindow : public KMainWindow
{
    Q_OBJECT

public:

    ~ImageWindow();

    void loadURL(const KURL::List& urlList, const KURL& urlCurrent,
                 const QString& caption=QString::null,
                 bool allowSaving=true,
                 AlbumIconView* view=0L);
                 
    void applySettings();
    
    static ImageWindow* imagewindow();
    
signals:

    void signalFileDeleted(const KURL& url);
    void signalFileAdded(const KURL& url);
    void signalFileModified(const KURL& url);

private:

    bool                    m_rotatedOrFlipped;
    bool                    m_fullScreen;
    bool                    m_fullScreenHideToolBar;
    bool                    m_removeFullScreenButton;

    // If image editor is launched by camera interface, current 
    // image cannot be saved.
    bool                    m_allowSaving;
    
    // If current image file format is only available in read only,
    // typicially all RAW image file formats.
    bool                    m_isReadOnly;

    QPopupMenu             *m_contextMenu;

    QSplitter              *m_splitter;
                              
    QLabel*                 m_nameLabel;
    QLabel*                 m_zoomLabel;
    QLabel*                 m_resLabel;
    
    KURL::List              m_urlList;
    KURL                    m_urlCurrent;
    
    KAccel                 *m_accel;
    
    // Actions

    KAction                *m_navNextAction;
    KAction                *m_navPrevAction;
    KAction                *m_navFirstAction;
    KAction                *m_navLastAction;

    KAction                *m_saveAction;
    KAction                *m_saveAsAction;
    KAction                *m_restoreAction;
     
    KAction                *m_zoomPlusAction;
    KAction                *m_zoomMinusAction;
    KToggleAction          *m_zoomFitAction;
    KToggleAction          *m_fullScreenAction;
    KSelectAction          *m_viewHistogramAction;

    KActionMenu            *m_rotateAction;
    KActionMenu            *m_flipAction;
    KAction                *m_rotate90Action;
    KAction                *m_rotate180Action;
    KAction                *m_rotate270Action;
    KAction                *m_flipHorzAction;
    KAction                *m_flipVertAction;

    KAction                *m_resizeAction;
    KAction                *m_cropAction;
    
    KAction                *m_fileprint;
    KAction                *m_fileDelete;
    
    KAction                *m_copyAction;
    KToolBarPopupAction    *m_undoAction;
    KToolBarPopupAction    *m_redoAction;
    
    KAction                *m_imagePluginsHelp;
    
    static ImageWindow     *m_instance;

    Canvas                 *m_canvas;

    ICCSettingsContainer    *m_ICCSettings;
    
    IOFileSettingsContainer *m_IOFileSettings;
    
    // Allow to use Image properties and 
    // Comments/Tags dialogs from main window.
    AlbumIconView           *m_view;
    
    Digikam::ImagePropertiesSideBarDB *m_rightSidebar;
    
private:

    ImageWindow();
    void buildGUI();
    void readSettings();
    void saveSettings();
    bool promptUserSave();
    void plugActionAccel(KAction* action);
    void unplugActionAccel(KAction* action);
    bool save();
    bool saveAs();

private slots:

    void slotLoadCurrent();
    
    void slotLoadNext();
    void slotLoadPrev();
    void slotLoadFirst();
    void slotLoadLast();

    void slotToggleAutoZoom();
    void slotViewHistogram();
    void slotToggleFullScreen();
    void slotEscapePressed();

    void slotResize();
    
    void slotContextMenu();
    void slotZoomChanged(float zoom);
    void slotChanged(bool, bool);
    void slotSelected(bool);

    void slotRotatedOrFlipped();
    
    void slotSave()   { if (m_isReadOnly) saveAs(); else save(); };
    void slotSaveAs() { saveAs(); };

    void slotAboutToShowUndoMenu();
    void slotAboutToShowRedoMenu();
    
    void slotFilePrint();
    
    void slotDeleteCurrentItem();

    void slotImagePluginsHelp();

    void slotEditKeys();
    void slotConfToolbars();
    void slotNewToolbarConfig();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    
protected:

    void closeEvent(QCloseEvent *e);
};

}  // namespace Digikam

#endif /* IMAGEWINDOW_H */
