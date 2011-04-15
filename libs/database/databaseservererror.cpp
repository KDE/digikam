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

// Local includes
#include "databaseservererror.h"

namespace Digikam
{

DatabaseServerError::DatabaseServerError(DatabaseServerErrorEnum errorType, const QString& errorText)
{
    m_ErrorText = errorText;
    m_ErrorType = errorType;
}

DatabaseServerError::DatabaseServerError(const DatabaseServerError& dbServerError)
{
    m_ErrorText = dbServerError.m_ErrorText;
    m_ErrorType = dbServerError.m_ErrorType;
}

DatabaseServerError::~DatabaseServerError()
{
}

// Marshall the DatabaseServerError data into a D-BUS argument
DatabaseServerError& DatabaseServerError::operator<<(const QDBusArgument& argument)
{
    argument.beginStructure();
    argument >> m_ErrorType >> m_ErrorText;
    argument.endStructure();
    return *this;
}

// Retrieve the DatabaseServerError data from the D-BUS argument
const DatabaseServerError& DatabaseServerError::operator>>(QDBusArgument& argument) const
{
    argument.beginStructure();
    argument << m_ErrorType << m_ErrorText;
    argument.endStructure();
    return *this;
}

int DatabaseServerError::getErrorType() const
{
    return m_ErrorType;
}

void DatabaseServerError::setErrorType(DatabaseServerErrorEnum errorType)
{
    m_ErrorType = errorType;
}

QString DatabaseServerError::getErrorText() const
{
    return m_ErrorText;
}
void DatabaseServerError::setErrorText(const QString& errorText)
{
    m_ErrorText=errorText;
}

} // namespace Digikam
