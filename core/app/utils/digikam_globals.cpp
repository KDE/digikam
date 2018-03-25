/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "drawdecoder.h"
#include "rawcameradlg.h"
#include "digikam_debug.h"
#include "digikam_config.h"

namespace Digikam
{

QShortcut* defineShortcut(QWidget* const w, const QKeySequence& key, const QObject* receiver, const char* slot)
{
    QShortcut* const s = new QShortcut(w);
    s->setKey(key);
    s->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(s, SIGNAL(activated()), receiver, slot);

    return s;
}

QStringList supportedImageMimeTypes(QIODevice::OpenModeFlag mode, QString& allTypes)
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

    bool tiff = false;
    bool jpeg = false;
#ifdef HAVE_JASPER
    bool jp2k = false;
#endif // HAVE_JASPER

    foreach(const QByteArray& frm, supported)
    {
        if (QString::fromLatin1(frm).contains(QLatin1String("tif"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("tiff"), Qt::CaseInsensitive))
        {
            tiff = true;
            continue;
        }

        if (QString::fromLatin1(frm).contains(QLatin1String("jpg"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpeg"), Qt::CaseInsensitive))
        {
            jpeg = true;
            continue;
        }

#ifdef HAVE_JASPER
        if (QString::fromLatin1(frm).contains(QLatin1String("jp2"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("j2k"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpx"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpc"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("pgx"),  Qt::CaseInsensitive))
        {
            jp2k = true;
            continue;
        }
#endif // HAVE_JASPER

        formats.append(i18n("%1 Image (%2)", QString::fromLatin1(frm).toUpper(), QLatin1String("*.") + QLatin1String(frm)));
        allTypes.append(QString::fromLatin1("*.%1 ").arg(QLatin1String(frm)));
    }

    if (tiff)
    {
        formats.append(i18n("TIFF Image (*.tiff *.tif)"));
        allTypes.append(QLatin1String("*.tiff *.tif "));
    }

    if (jpeg)
    {
        formats.append(i18n("JPEG Image (*.jpg *.jpeg *.jpe)"));
        allTypes.append(QLatin1String("*.jpg *.jpeg *.jpe "));
    }

#ifdef HAVE_JASPER
    if (jp2k)
    {
        formats.append(i18n("JPEG2000 Image (*.jp2 *.j2k *.jpx *.pgx)"));
        allTypes.append(QLatin1String("*.jp2 *.j2k *.jpx *.pgx "));
    }
#endif // HAVE_JASPER

    formats << i18n("Progressive Graphics file (*.pgf)");
    allTypes.append(QLatin1String("*.pgf "));

    if (mode != QIODevice::WriteOnly)
    {
        formats << i18n("Raw Images (%1)", QLatin1String(DRawDecoder::rawFiles()));
        allTypes.append(QLatin1String(DRawDecoder::rawFiles()));
        formats << i18n("All supported files (%1)", allTypes);
    }

    return formats;
}

void showRawCameraList()
{
    RawCameraDlg* const dlg = new RawCameraDlg(qApp->activeWindow());
    dlg->show();
}

QProcessEnvironment adjustedEnvironmentForAppImage()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // If we are running into AppImage bundle, switch env var to the right values.
    if (env.contains(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")) &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH"))  &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS"))   &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_PATH")))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Adjusting environment variables for AppImage bundle";

        env.insert(QLatin1String("LD_LIBRARY_PATH"),
                   env.value(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")));
        env.insert(QLatin1String("QT_PLUGIN_PATH"),
                   env.value(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH")));
        env.insert(QLatin1String("XDG_DATA_DIRS"),
                   env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS")));
        env.insert(QLatin1String("PATH"),
                   env.value(QLatin1String("APPIMAGE_ORIGINAL_PATH")));
    }

    return env;
}

} // namespace Digikam
