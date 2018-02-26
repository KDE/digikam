/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam components info dialog.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "componentsinfo.h"

// Qt includes

#include <QApplication>
#include <QString>
#include <QMap>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "applicationsettings.h"
#include "digikam_config.h"
#include "libsinfodlg.h"
#include "dbstatdlg.h"
#include "libopencv.h"

// LibGphoto2 includes

#ifdef HAVE_GPHOTO2

extern "C"
{
#include <gphoto2-version.h>
}

#endif // HAVE_GPHOTO2

// QtAV includes

#ifdef HAVE_MEDIAPLAYER
#    include <QtAV/version.h>

extern "C"
{
#   include <libavformat/version.h>
#   include <libavutil/version.h>
#   include <libavcodec/version.h>
}

#endif // HAVE_MEDIAPLAYER

namespace Digikam
{

void showDigikamComponentsInfo()
{
    // Set digiKam specific components info list.
    QMap<QString, QString> list;

#ifdef HAVE_GPHOTO2
    list.insert(i18n("LibGphoto2"), QLatin1String(gp_library_version(GP_VERSION_SHORT)[0]));
#else
    list.insert(i18n("LibGphoto2 support"), i18n("no"));
#endif /* HAVE_GPHOTO2 */

#ifdef HAVE_KFILEMETADATA
    list.insert(i18n("Baloo support"), i18n("Yes"));
#else
    list.insert(i18n("Baloo support"), i18n("no"));
#endif /* HAVE_KFILEMETADATA */

#ifdef HAVE_AKONADICONTACT
    list.insert(i18n("AkonadiContact support"), i18n("Yes"));
#else
    list.insert(i18n("AkonadiContact support"), i18n("no"));
#endif /* HAVE_AKONADICONTACT */

#ifdef HAVE_MEDIAPLAYER
    list.insert(i18n("Media player support"), i18n("Yes"));
    list.insert(i18n("LibQtAV"),              QString::fromLatin1("%1.%2.%3").arg(QTAV_MAJOR).arg(QTAV_MINOR).arg(QTAV_PATCH));
    list.insert(i18n("LibAVFormat"),          QString::fromLatin1("%1.%2.%3").arg(LIBAVFORMAT_VERSION_MAJOR).arg(LIBAVFORMAT_VERSION_MINOR).arg(LIBAVFORMAT_VERSION_MICRO));
    list.insert(i18n("LibAVCodec"),           QString::fromLatin1("%1.%2.%3").arg(LIBAVCODEC_VERSION_MAJOR).arg(LIBAVCODEC_VERSION_MINOR).arg(LIBAVCODEC_VERSION_MICRO));
    list.insert(i18n("LibAVUtil"),            QString::fromLatin1("%1.%2.%3").arg(LIBAVUTIL_VERSION_MAJOR).arg(LIBAVUTIL_VERSION_MINOR).arg(LIBAVUTIL_VERSION_MICRO));
#else
    list.insert(i18n("Media player support"), i18n("no"));
#endif /* HAVE_MEDIAPLAYER */

#ifdef HAVE_DBUS
    list.insert(i18n("DBus support"), i18n("Yes"));
#else
    list.insert(i18n("DBus support"), i18n("no"));
#endif /* HAVE_DBUS */

#ifdef HAVE_PANORAMA
    list.insert(i18n("Panorama support"), i18n("yes"));
#else
    list.insert(i18n("Panorama support"), i18n("no"));
#endif /* HAVE_PANORAMA */

#ifdef HAVE_HTMLGALLERY
    list.insert(i18n("HTML Gallery support"), i18n("yes"));
#else
    list.insert(i18n("HTML Gallery support"), i18n("no"));
#endif /* HAVE_HTMLGALLERY */

#ifdef HAVE_KCALENDAR
    list.insert(i18n("Calendar support"), i18n("yes"));
#else
    list.insert(i18n("Calendar support"), i18n("no"));
#endif /* HAVE_KCALENDAR */

    list.insert(i18n("LibOpenCV"),    QLatin1String(CV_VERSION));

    // Database Backend information

    QString dbBe = ApplicationSettings::instance()->getDbEngineParameters().databaseType;
    list.insert(i18n("Database backend"), dbBe);

    if (dbBe != QLatin1String("QSQLITE"))
    {
        QString internal = ApplicationSettings::instance()->getDbEngineParameters().internalServer? i18n("Yes") : i18n("No");
        list.insert(i18n("Database internal server"), internal);
    }

    LibsInfoDlg* const dlg = new LibsInfoDlg(qApp->activeWindow());
    dlg->setInfoMap(list);
    dlg->show();
}

void showDigikamDatabaseStat()
{
    DBStatDlg* const dlg = new DBStatDlg(qApp->activeWindow());
    dlg->show();
}

}  // namespace Digikam
