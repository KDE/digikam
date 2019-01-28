/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "odmpform.h"

// Qt includes

#include <QFile>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericOneDrivePlugin
{

ODMPForm::ODMPForm()
{
}

ODMPForm::~ODMPForm()
{
}

bool ODMPForm::addFile(const QString& imgPath)
{
    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    m_buffer = file.readAll();
    file.close();

    return true;
}

QByteArray ODMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericOneDrivePlugin
