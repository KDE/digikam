/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-26
 * Description : class for error reporting
 *
 * Copyright (C) 2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#ifndef DATABASESERVERERROR_H_
#define DATABASESERVERERROR_H_

// QT includes
#include <QString>
#include <QVariant>
#include <QDBusArgument>

// Local includes
#include "dbusutilities.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT DatabaseServerError
{
public:
    enum DatabaseServerErrorEnum
    {
        /**
         * No errors occurred while starting the dbms
         */
        NoErrors=0,

        /**
         * The requested dbms type is not supported.
         */
        NotSupported,

        /**
         * A error has occurred while starting the dbms executable.
         */
        StartError
    };

    explicit DatabaseServerError(DatabaseServerErrorEnum errorType=NoErrors, const QString& errorText="");
    DatabaseServerError(const DatabaseServerError& dbServerError);
    ~DatabaseServerError();

    // Marshall the DatabaseServerError data into a D-BUS argument
    DatabaseServerError& operator<<(const QDBusArgument& argument);
    // Retrieve the DatabaseServerError data from the D-BUS argument
    const DatabaseServerError& operator>>(QDBusArgument& argument) const;

    int                      getErrorType() const;
    void                     setErrorType(DatabaseServerErrorEnum errorType);
    QString                  getErrorText() const;
    void                     setErrorText(const QString& errorText);


private:
    QString                     m_ErrorText;
    int                         m_ErrorType;
};

} // namespace Digikam

// custom macro from our dbusutilities.h
DECLARE_METATYPE_FOR_DBUS(Digikam::DatabaseServerError)

#endif /* DATABASESERVERERROR_H_ */
