/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-26
 * Description : database server error reporting
 *
 * Copyright (C) 2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2016 by Swati Lodha <swatilodha27 at gmail dot com>
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

#ifndef DATABASE_SERVER_ERROR_H_
#define DATABASE_SERVER_ERROR_H_

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseServerError
{
public:

    enum DatabaseServerErrorEnum
    {
        /**
         * No errors occurred while starting the database server
         */
        NoErrors = 0,

        /**
         * The requested database type is not supported.
         */
        NotSupported,

        /**
         * A error has occurred while starting the database server executable.
         */
        StartError
    };

public:

    explicit DatabaseServerError(DatabaseServerErrorEnum errorType = NoErrors, const QString& errorText = QString());
    DatabaseServerError(const DatabaseServerError& dbServerError);
    ~DatabaseServerError();

    int     getErrorType() const;
    void    setErrorType(DatabaseServerErrorEnum errorType);
    QString getErrorText() const;
    void    setErrorText(const QString& errorText);

private:

    QString m_ErrorText;
    int     m_ErrorType;
};

} // namespace Digikam

#endif /* DATABASE_SERVER_ERROR_H_ */
