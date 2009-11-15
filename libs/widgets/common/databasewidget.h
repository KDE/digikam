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

#ifndef DATABASEWIDGET_H
#define DATABASEWIDGET_H

// QT includes
#include <qstring.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qgroupbox.h>

// KDE includes
#include <kdialog.h>
#include <kurlrequester.h>
#include <kurl.h>

// Local includes
#include "digikam_export.h"

namespace Digikam
{


class DIGIKAM_EXPORT DatabaseWidget : public QWidget
{
    Q_OBJECT

public:

    DatabaseWidget(QWidget* parent);
    ~DatabaseWidget();

    KUrlRequester           *databasePathEdit;
    QString                  originalDbPath;
    QString                  originalDbType;
    QLabel                  *databasePathLabel;
    QComboBox               *databaseType;
    QLineEdit               *databaseName;
    QLineEdit               *hostName;
    QSpinBox                *hostPort;
    QLineEdit               *connectionOptions;

    QLineEdit               *userName;

    QLineEdit               *password;

    QGroupBox               *expertSettings;

public Q_SLOTS:

    void slotChangeDatabasePath(const KUrl&);
    void slotDatabasePathEdited(const QString&);
    void setDatabaseInputFields(const QString&);
    void checkDatabaseConnection();

private:
    void checkDBPath();
    void setupMainArea();
};

}  // namespace Digikam

#endif  // DATABASEWIDGET_H
