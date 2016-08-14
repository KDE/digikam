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
