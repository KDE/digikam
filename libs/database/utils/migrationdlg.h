/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DATABASE_MIGRATION_DIALOG_H
#define DATABASE_MIGRATION_DIALOG_H

// QT includes

#include <QThread>
#include <QProgressBar>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "databasewidget.h"
#include "coredbbackend.h"
#include "coredbcopymanager.h"

namespace Digikam
{

class DatabaseCopyThread : public QThread
{
    Q_OBJECT

public:

    explicit DatabaseCopyThread(QWidget* const parent);
    ~DatabaseCopyThread();

    void init(const DbEngineParameters& fromDatabaseWidget, const DbEngineParameters& toDatabaseWidget);
    void run();

public:

    CoreDbCopyManager m_copyManager;

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT MigrationDlg : public QDialog
{
    Q_OBJECT

public:

    explicit MigrationDlg(QWidget* const parent);
    ~MigrationDlg();

private Q_SLOTS:

    void performCopy();
    void unlockInputFields();
    void lockInputFields();

    void handleFinish(int finishState, const QString& errorMsg);
    void handleStepStarted(const QString& stepName);
    void handleSmallStepStarted(int currValue, int maxValue);


private:

    void setupMainArea();
    void dataInit();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DATABASE_MIGRATION_DIALOG_H
