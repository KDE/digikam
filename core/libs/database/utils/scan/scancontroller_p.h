/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller - private containers.
 *
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_SCAN_CONTROLLER_P_H
#define DIGIKAM_SCAN_CONTROLLER_P_H

#include "scancontroller.h"

// Qt includes

#include <QStringList>
#include <QFileInfo>
#include <QPixmap>
#include <QIcon>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QTimer>
#include <QEventLoop>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "collectionscanner.h"
#include "coredbaccess.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "filereadwritelock.h"
#include "coredbwatch.h"
#include "dprogressdlg.h"
#include "dmetadata.h"
#include "coredb.h"
#include "albummanager.h"
#include "album.h"
#include "coredbschemaupdater.h"

namespace Digikam
{

/*
 * This class is derived from the ClassicLoadingCacheFileWatch,
 * which means it has the full functionality of the class
 * and only extends it by listening to CollectionScanner information
 */
class Q_DECL_HIDDEN ScanControllerLoadingCacheFileWatch : public ClassicLoadingCacheFileWatch
{
    Q_OBJECT


public:

    explicit ScanControllerLoadingCacheFileWatch();

private Q_SLOTS:

    void slotImageChanged(const ImageChangeset& changeset);
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN SimpleCollectionScannerObserver : public CollectionScannerObserver
{
public:

    explicit SimpleCollectionScannerObserver(bool* const var);

    bool continueQuery();

public:

    bool* m_continue;
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN ScanController::Private
{
public:

    explicit Private();


    QPixmap albumPixmap();
    QPixmap rootPixmap();
    QPixmap actionPixmap();
    QPixmap errorPixmap();
    QPixmap restartPixmap();

    void garbageCollectHints(bool setAccessTime);

public:

    bool                            running;
    bool                            needsInitialization;
    bool                            needsCompleteScan;
    bool                            needsUpdateUniqueHash;
    bool                            idle;

    int                             scanSuspended;

    QStringList                     scanTasks;

    QStringList                     completeScanDeferredAlbums;
    bool                            deferFileScanning;
    bool                            finishScanAllowed;

    QMutex                          mutex;
    QWaitCondition                  condVar;

    bool                            continueInitialization;
    bool                            continueScan;
    bool                            continuePartialScan;

    bool                            fileWatchInstalled;

    QEventLoop*                     eventLoop;

    QTimer*                         showTimer;
    QTimer*                         relaxedTimer;
    QTimer*                         externalTimer;

    QPixmap                         albumPix;
    QPixmap                         rootPix;
    QPixmap                         actionPix;
    QPixmap                         errorPix;

    CollectionScannerHintContainer* hints;

    QDateTime                       lastHintAdded;

    DProgressDlg*                   progressDialog;

    ScanController::Advice          advice;

    bool                            needTotalFiles;
    int                             totalFilesToScan;
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN ScanControllerCreator
{
public:

    ScanController object;
};

} // namespace Digikam

#endif // DIGIKAM_SCAN_CONTROLLER_P_H
