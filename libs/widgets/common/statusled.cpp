/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-02-15
 * Description : a led indicator.
 * 
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qpixmap.h>
#include <qstring.h>

// KDE includes.

#include <kglobalsettings.h>
#include <kstandarddirs.h>

// Local includes.

#include "statusled.h"
#include "statusled.moc"

namespace Digikam
{

StatusLed::StatusLed(QWidget *parent)
         : QLabel(parent)
{
    setLedColor(Gray);
    setFocusPolicy(QWidget::NoFocus);
}

StatusLed::~StatusLed()
{
}

void StatusLed::setLedColor(LedColor color)
{
    m_color = color;

    QString file;
    switch(m_color)
    {
        case Green:
            file = QString("indicator-green");
            break;

        case Red:
            file = QString("indicator-red");
            break;

        default:
            file = QString("indicator-gray");
            break;
    }

    KGlobal::dirs()->addResourceType(file.ascii(), KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir(file.ascii(), file + QString(".png"));
    setPixmap(QPixmap(directory + file + QString(".png")));
}

StatusLed::LedColor StatusLed::ledColor() const
{
    return m_color;
}

}  // namespace Digikam

