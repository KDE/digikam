/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-12
 * Description : digiKam image editor GUI
 * 
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

// KDE includes.

#include <kurl.h>

// Local includes.

#include "editorwindow.h"

class QPopupMenu;
class QStringList;

class KAccel;
class KAction;
class KActionMenu;
class KToolBarPopupAction;
class KToggleAction;
class KSelectAction;

namespace Digikam
{

class AlbumIconView;
class ImagePropertiesSideBarDB;

class ImageWindow : public EditorWindow
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
    static bool         imagewindowCreated();
    
signals:

    void signalFileDeleted(const KURL& url);
    void signalFileAdded(const KURL& url);
    void signalFileModified(const KURL& url);

private:

    bool                    m_rotatedOrFlipped;
    bool                    m_fullScreenHideToolBar;
    bool                    m_removeFullScreenButton;

    // If image editor is launched by camera interface, current 
    // image cannot be saved.
    bool                    m_allowSaving;
    
    QPopupMenu             *m_contextMenu;

    KURL::List              m_urlList;
    KURL                    m_urlCurrent;
    
    KAccel                 *m_accel;
    
    // Actions

    KActionMenu            *m_flipAction;
    KAction                *m_flipHorzAction;
    KAction                *m_flipVertAction;

    // Allow to use Image properties and 
    // Comments/Tags dialogs from main window.
    AlbumIconView            *m_view;
    
    ImagePropertiesSideBarDB *m_rightSidebar;

    static ImageWindow       *m_instance;

private:

    ImageWindow();
    void setupUserArea();
    void saveSettings();
    bool promptUserSave();
    void setupActions();

    void readSettings();
    void plugActionAccel(KAction* action);
    void unplugActionAccel(KAction* action);
    bool save();
    bool saveAs();
    void finishSaving(bool success);

private slots:

    void slotLoadCurrent();
    
    void slotForward();
    void slotBackward();
    void slotFirst();
    void slotLast();

    void slotViewHistogram();
    void slotToggleFullScreen();
    
    void slotContextMenu();
    void slotChanged(bool, bool);
    void slotSelected(bool);

    void slotRotatedOrFlipped();
    
    void slotDeleteCurrentItem();

    void slotFilePrint(){ printImage(m_urlCurrent); };

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    
    void slotLoadingStarted(const QString &filename);
    void slotLoadingFinished(const QString &filename, bool success, bool isReadOnly);
    void slotLoadingProgress(const QString& filePath, float progress);
    void slotSavingStarted(const QString &filename);
    void slotSavingFinished(const QString &filename, bool success);
    void slotSavingProgress(const QString& filePath, float progress);
};

}  // namespace Digikam

#endif /* IMAGEWINDOW_H */
