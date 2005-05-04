/* ============================================================
 * Authors: Renchi Raju <renchi at pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date   : 2003-02-03
 * Description : digiKam setup dialog.
 * 
 * Copyright 2003-2004 by Renchi Raju and Gilles Caulier
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
 
#ifndef SETUP_H
#define SETUP_H

// KDE includes.

#include <kdialogbase.h>

class QFrame;

class SetupGeneral;
class SetupExif;
class SetupCollections;
class SetupMime;
class SetupEditor;
class SetupPlugins;
class SetupCamera;
class SetupMisc;

class Setup : public KDialogBase 
{
    Q_OBJECT

public:

    enum Page {
        General = 0,
        Exif,
        Collections,
        Mime,
        Editor,
        Plugins,
        Camera,
        Miscellaneous
    };
    
    Setup(QWidget* parent=0, const char* name=0,
          Page page=General);
    ~Setup();

    SetupPlugins     *pluginsPage_;
    SetupEditor      *editorPage_;
    
private:

    QFrame           *page_general;
    QFrame           *page_exif;
    QFrame           *page_collections;
    QFrame           *page_mime;
    QFrame           *page_editor;
    QFrame           *page_plugins;
    QFrame           *page_camera;
    QFrame           *page_misc;
    
    SetupGeneral     *generalPage_;
    SetupExif        *exifPage_;
    SetupCollections *collectionsPage_;
    SetupMime        *mimePage_;
    SetupCamera      *cameraPage_;
    SetupMisc        *miscPage_;

private slots:

    void slotOkClicked();
};

#endif  // SETUP_H
