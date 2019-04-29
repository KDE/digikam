/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#ifndef SHOW_FOTO_PRIVATE_H
#define SHOW_FOTO_PRIVATE_H

// Qt includes

#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QPointer>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStandardPaths>
#include <QStyle>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMimeDatabase>
#include <QMessageBox>
#include <QApplication>
#include <QMimeType>
#include <QTemporaryFile>
#include <QSplitter>
#include <QAction>
#include <QUrl>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>

// Local includes

#include "showfoto.h"
#include "showfotoiteminfo.h"
#include "showfotothumbnailbar.h"
#include "dsplashscreen.h"
#include "itempropertiessidebar.h"
#include "showfotodragdrophandler.h"
#include "thumbnailloadthread.h"
#include "drawdecoder.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "canvas.h"
#include "editorcore.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "dfileoperations.h"
#include "iccsettingscontainer.h"
#include "imagedialog.h"
#include "iofilesettings.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "metaenginesettings.h"
#include "savingcontext.h"
#include "showfotosetup.h"
#include "showfotosetupmisc.h"
#include "setupicc.h"
#include "slideshow.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailsize.h"
#include "dnotificationwrapper.h"
#include "showfotodelegate.h"
#include "showfotothumbnailmodel.h"
#include "showfotocategorizedview.h"
#include "showfotosettings.h"
#include "dmetainfoiface.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dpluginloader.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowFoto::Private
{
public:

    explicit Private()
      : validIccPath(true),
        itemsNb(0),
        vSplitter(nullptr),
        fileOpenAction(nullptr),
        openFilesInFolderAction(nullptr),
        mediaServerAction(nullptr),
        first(nullptr),
        model(nullptr),
        dDHandler(nullptr),
        filterModel(nullptr),
        thumbLoadThread(nullptr),
        thumbBar(nullptr),
        thumbBarDock(nullptr),
        normalDelegate(nullptr),
        rightSideBar(nullptr),
        splash(nullptr),
        settings(nullptr)
    {
    }

    bool                             validIccPath;

    int                              itemsNb;

    QSplitter*                       vSplitter;
    QAction*                         fileOpenAction;
    QUrl                             currentLoadedUrl;
    QUrl                             lastOpenedDirectory;
    QAction*                         openFilesInFolderAction;
    QAction*                         mediaServerAction;
    QAction*                         first;

    ShowfotoItemInfoList             infoList;
    ShowfotoThumbnailModel*          model;
    ShowfotoDragDropHandler*         dDHandler;
    ShowfotoFilterModel*             filterModel;
    Digikam::ThumbnailLoadThread*    thumbLoadThread;
    ShowfotoThumbnailBar*            thumbBar;
    Digikam::ThumbBarDock*           thumbBarDock;
    ShowfotoNormalDelegate*          normalDelegate;
    Digikam::ItemPropertiesSideBar* rightSideBar;
    Digikam::DSplashScreen*          splash;
    ShowfotoSettings*                settings;
};

} // namespace Showfoto

#endif // SHOW_FOTO_PRIVATE_H
