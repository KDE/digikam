/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
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

class QStringList;

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
                 
    static ImageWindow* imagewindow();
    static bool         imagewindowCreated();

    void applySettings();
    void setup(bool iccSetupPage=false);

signals:

    void signalFileDeleted(const KURL& url);
    void signalFileAdded(const KURL& url);
    void signalFileModified(const KURL& url);

private:

    // If image editor is launched by camera interface, current
    // image cannot be saved.
    bool                      m_allowSaving;
    
    KURL::List                m_urlList;
    KURL                      m_urlCurrent;
    
    // Allow to use Image properties and
    // Comments/Tags dialogs from main window.
    AlbumIconView            *m_view;
    
    ImagePropertiesSideBarDB *m_rightSidebar;

    static ImageWindow       *m_instance;

private:

    void closeEvent(QCloseEvent* e);

    void setupActions();
    void setupConnections();
    void setupUserArea();
    void toggleGUI2FullScreen();

    bool save();
    bool saveAs();

    void saveIsComplete();
    void saveAsIsComplete(); 
    void setViewToURL(const KURL &url);

    ImageWindow();

private slots:

    void slotForward();
    void slotBackward();
    void slotFirst();
    void slotLast();
    void slotFilePrint(){ printImage(m_urlCurrent); };
        
    void slotLoadCurrent();
    void slotDeleteCurrentItem();
    
    void slotChanged();
    void slotUndoStateChanged(bool, bool, bool);
    void slotUpdateItemInfo();

    void slotContextMenu();
    
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
};

}  // namespace Digikam

#endif /* IMAGEWINDOW_H */
