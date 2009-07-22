/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database element configuration
 *
 * Copyright (C) 2009 by Holger Foerster <hamsi2k at freenet dot de>
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

#ifndef DATABASECONFIGELEMENT_H_
#define DATABASECONFIGELEMENT_H_

// Qt includes.

#include <QtGlobal>
#include <QMap>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT databaseActionElement
{
    public:
        QString m_Mode;
        int     m_Order;
        QString m_Statement;
};

class DIGIKAM_EXPORT databaseAction
{
    public:
        QString m_Name;
        QString m_Mode;
        QList<databaseActionElement> m_DBActionElements;
};

class DIGIKAM_EXPORT databaseconfigelement
{
public:
    databaseconfigelement();
    virtual ~databaseconfigelement();

    QString m_DatabaseID;
    QString m_HostName;
    QString m_Port;
    QString m_ConnectOptions;
    QString m_DatabaseName;
    QString m_UserName;
    QString m_Password;
    QMap<QString, databaseAction> m_SQLStatements;
};

} // namespace Digikam

#endif /* DATABASECONFIGELEMENT_H_ */
