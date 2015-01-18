/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikam_globals.h"

// Qt includes

#include <QObject>
#include <QList>
#include <QImageReader>
#include <QImageWriter>
#include <QByteArray>
#include <QShortcut>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

#pragma message("this should be moved to some common place instead of app/utils..")
QShortcut* defineShortcut(QWidget* const w, const QKeySequence& key, const QObject* receiver, const char* slot)
{
    QShortcut* const s = new QShortcut(w);
    s->setKey(key);
    s->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(s, SIGNAL(activated()), receiver, slot);

    return s;
}

QStringList supportedImageMimeTypes(QIODevice::OpenModeFlag mode)
{
    QStringList       formats;
    QList<QByteArray> supported;

    switch(mode)
    {
        case QIODevice::ReadOnly:
            supported = QImageReader::supportedImageFormats();
            break;
        case QIODevice::WriteOnly:
            supported = QImageWriter::supportedImageFormats();
            break;
        case QIODevice::ReadWrite:
            supported = QImageWriter::supportedImageFormats() + QImageReader::supportedImageFormats();
            break;
        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unsupported mode!";
            break;
    }

    Q_FOREACH(QByteArray frm, supported)
        formats.append(QString("*.") + QString::fromLatin1(frm) + QString("|%1 Image").arg(QString::fromLatin1(frm).toUpper()));

    return formats;
}

} // namespace Digikam
