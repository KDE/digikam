/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-28
 * Description : scan pictures interface.
 * 
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

// Qt includes.

#include <QThread>
#include <QString>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "digikam_export.h"
#include "initializationobserver.h"


namespace Digikam
{

class CollectionScanner;
class SplashScreen;
class ScanControllerPriv;

class DIGIKAM_EXPORT ScanController : public QThread, public InitializationObserver
{
    Q_OBJECT

public:

    static ScanController *instance();
    void shutDown();

    enum Advice
    {
        Success,
        ContinueWithoutDatabase,
        AbortImmediately
    };

    /**
     * Calls DatabaseAccess::checkReadyForUse(), providing progress
     * feedback if schema updating occurs.
     * Synchronous, returns when ready.
     */
    Advice databaseInitialization();

    /**
     * Carries out a complete collection scan, providing progress feedback.
     * Synchronous, returns when ready.
     * The database will be locked while the scan is running.
     */
    void completeCollectionScan(SplashScreen *splash=0);

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     */
    void scheduleCollectionScan(const QString &path);

    /** Temporarilly suspend collection scanning.
     *  All scheduled scanning tasks are queued
     *  and will be done when resumeCollectionScan()
     *  has been called.
     *  Calling these methods is recursive, you must resume
     *  as often as you called suspend.
     */
    void suspendCollectionScan();
    void resumeCollectionScan();

signals:

    void databaseInitialized(bool success);
    void completeScanDone();
    void triggerShowProgressDialog();
    void incrementProgressDialog(int);
    void errorFromInitialization(const QString &);
    void progressFromInitialization(const QString &, int);

private slots:

    void slotTotalFilesToScan(int count);
    void slotStartScanningAlbum(const QString &albumRoot, const QString &album);
    void slotFinishedScanningAlbum(const QString &, const QString &, int filesScanned);
    void slotStartScanningAlbumRoot(const QString &albumRoot);
    void slotStartScanningForStaleAlbums();
    void slotStartScanningAlbumRoots();

    void slotShowProgressDialog();
    void slotTriggerShowProgressDialog();

    void slotProgressFromInitialization(const QString &message, int numberOfSteps);
    void slotErrorFromInitialization(const QString &errorMessage);

protected:

    virtual void run();

private:

    friend class ScanControllerCreator;
    ScanController();
    ~ScanController();

    ScanControllerPriv *d;

    virtual void moreSchemaUpdateSteps(int numberOfSteps);
    virtual void schemaUpdateProgress(const QString &message, int numberOfSteps);
    virtual void finishedSchemaUpdate(UpdateResult result);
    virtual void connectCollectionScanner(CollectionScanner *scanner) ;
    virtual void error(const QString &errorMessage);

    void createProgressDialog();
};

}  // namespace Digikam

#endif /* SCANCONTROLLER_H */
