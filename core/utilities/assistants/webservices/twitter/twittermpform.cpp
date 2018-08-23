/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * Copyright (C) 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
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

// local includes

#include "twittermpform.h"

// Qt includes

#include <QFile>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

TwMPForm::TwMPForm()
{
}

TwMPForm::~TwMPForm()
{
}

bool TwMPForm::addFile(const QString& imgPath)
{
    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    m_buffer = file.readAll();

    return true;
}

QByteArray TwMPForm::formData() const
{
    return m_buffer;
}

} // namespace Digikam
