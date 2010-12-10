/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DATABASEWIDGET_H
#define DATABASEWIDGET_H

// Qt includes

#include <QString>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

// KDE includes

#include <kdialog.h>
#include <kurlrequester.h>
#include <kurl.h>

// Local includes

#include "albumsettings.h"
#include "databaseparameters.h"

namespace Digikam
{

class DatabaseWidgetPriv;

class DatabaseWidget : public QWidget
{
    Q_OBJECT

public:

    DatabaseWidget(QWidget* parent = 0);
    ~DatabaseWidget();

public:

    QString        originalDbPath;
    QString        originalDbType;
    QComboBox*     databaseType;
    QCheckBox*     internalServer;
    QSpinBox*      hostPort;

    QLineEdit*     databaseName;
    QLineEdit*     databaseNameThumbnails;
    QLineEdit*     hostName;
    QLineEdit*     connectionOptions;
    QLineEdit*     userName;
    QLineEdit*     password;

    KUrlRequester* databasePathEdit;

public:

    void setParametersFromSettings(const AlbumSettings* settings);
    DatabaseParameters getDatabaseParameters();

    QString currentDatabaseType() const;
    void setDatabaseInputFields(const QString&);

public Q_SLOTS:

    void slotChangeDatabasePath(const KUrl&);
    void slotDatabasePathEdited(const QString&);
    void slotHandleDBTypeIndexChanged(int index);
    void slotHandleInternalServerCheckbox(int enableFields);
    void checkDatabaseConnection();

private:

    void checkDBPath();
    void setupMainArea();

private:

    DatabaseWidgetPriv* const d;
};

}  // namespace Digikam

#endif  // DATABASEWIDGET_H
