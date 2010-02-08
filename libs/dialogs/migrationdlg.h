/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009 by Holger Foerster <Hamsi2k at freenet dot de>
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
#include "digikam_export.h"
#include "databasewidget.h"
#include "databasebackend.h"
#include "databasecopymanager.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseCopyThread : public QThread
{
        Q_OBJECT

        public:
            DatabaseCopyThread(QWidget* parent);
            bool stop;
            void run();

            void init(DatabaseParameters fromDatabaseWidget, DatabaseParameters toDatabaseWidget);
            DatabaseCopyManager copyManager;

        private:
            DatabaseParameters fromDatabaseParameters;
            DatabaseParameters toDatabaseParameters;
};


class DIGIKAM_EXPORT MigrationDlg : public KDialog
{
    Q_OBJECT

public:

    MigrationDlg(QWidget* parent);
    ~MigrationDlg();

private Q_SLOTS:
    void performCopy();
    void unlockInputFields();
    void lockInputFields();

    void handleFinish(int finishState, QString errorMsg);
    void handleStepStarted(QString stepName);
    void handleSmallStepStarted(int currValue, int maxValue);
private:

//    MigrationDlgPriv* const d;
    DatabaseWidget *fromDatabaseWidget;
    DatabaseWidget *toDatabaseWidget;
    QPushButton    *migrateButton;
    QPushButton    *cancelButton;
    QProgressBar   *progressBar;
    QProgressBar   *progressBarSmallStep;
    DatabaseCopyThread *thread;

    void setupMainArea();
    void dataInit();
};

}  // namespace Digikam

#endif  // MIGRATIONDLG_H
