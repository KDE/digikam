/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2011 by Gilles Caulier<caulier dot gilles at gmail dot com>
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

#ifndef MIGRATIONDLG_H
#define MIGRATIONDLG_H

// QT includes

#include <QThread>
#include <QProgressBar>

// KDE includes

#include <kdialog.h>

// Local includes

#include "databasewidget.h"
#include "databasebackend.h"
#include "databasecopymanager.h"

namespace Digikam
{

class DatabaseCopyThread : public QThread
{
    Q_OBJECT

public:

    DatabaseCopyThread(QWidget* parent);
    ~DatabaseCopyThread();

    bool stop;
    void run();

    void init(DatabaseParameters fromDatabaseWidget, DatabaseParameters toDatabaseWidget);
    DatabaseCopyManager copyManager;

private:

    class DatabaseCopyThreadPriv;
    DatabaseCopyThreadPriv* const d;
};

// --------------------------------------------------------------------

class MigrationDlg : public KDialog
{
    Q_OBJECT

public:

    MigrationDlg(QWidget* parent);
    ~MigrationDlg();

private Q_SLOTS:

    void performImgCopy();
#ifdef USE_THUMBS_DB
    void performThumbCopy();
#endif
    void unlockInputFields();
    void lockInputFields();

    void handleFinish(int finishState, QString errorMsg);
    void handleStepStarted(const QString& stepName);
    void handleSmallStepStarted(int currValue, int maxValue);


private:

    void setupMainArea();
    void dataInit();

private:

    class MigrationDlgPriv;
    MigrationDlgPriv* const d;
};

}  // namespace Digikam

#endif  // MIGRATIONDLG_H
