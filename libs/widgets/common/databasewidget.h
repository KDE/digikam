/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "config-digikam.h"
#include "albumsettings.h"
#include "databaseparameters.h"

namespace Digikam
{

class DatabaseWidget : public QWidget
{
    Q_OBJECT

public:

    DatabaseWidget(QWidget* parent = 0, const QString & title = QString(""));
    ~DatabaseWidget();

public:

    QCheckBox*     internalServer;

    QString        imgOriginalDbPath;
    QString        imgOriginalDbType;
    QComboBox*     imgDatabaseType;
    QSpinBox*      imgHostPort;

    QLineEdit*     imgDatabaseName;
    QLineEdit*     imgHostName;
    QLineEdit*     imgConnectionOptions;
    QLineEdit*     imgUserName;
    QLineEdit*     imgPassword;

    QString        tmbOriginalDbPath;
    QString        tmbOriginalDbType;
    QComboBox*     tmbDatabaseType;
    QSpinBox*      tmbHostPort;

    QLineEdit*     tmbDatabaseName;
    QLineEdit*     tmbHostName;
    QLineEdit*     tmbConnectionOptions;
    QLineEdit*     tmbUserName;
    QLineEdit*     tmbPassword;

    KUrlRequester* imgDatabasePathEdit;
    KUrlRequester* tmbDatabasePathEdit;

public:

    void setParametersFromSettings(const AlbumSettings* settings);
    DatabaseParameters getDatabaseParameters();

    QString imgCurrentDatabaseType() const;
    QString tmbCurrentDatabaseType() const;
    void setDatabaseInputFields(const QString&);

public Q_SLOTS:

    void slotHandleInternalServerCheckbox(int enableFields);

    void slotImgChangeDatabasePath(const KUrl&);
    void slotImgDatabasePathEdited(const QString&);
    void slotImgHandleDBTypeIndexChanged(int index);
    void slotImgCheckDatabaseConnection();

    void slotTmbChangeDatabasePath(const KUrl&);
    void slotTmbDatabasePathEdited(const QString&);
    void slotTmbHandleDBTypeIndexChanged(int index);
    void slotTmbCheckDatabaseConnection();

private:

    void checkDBPath();
    void setupMainArea(const QString & title);

private:

    class DatabaseWidgetPriv;
    DatabaseWidgetPriv* const d;
};

} // namespace Digikam

#endif  // DATABASEWIDGET_H
