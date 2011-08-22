/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam components info dialog.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COMPONENTS_INFO_H
#define COMPONENTS_INFO_H

// Qt includes

#include <QString>
#include <QMap>

// KDE includes

#include <klocale.h>
#include <kapplication.h>

// LibKIPI includes

#include <libkipi/interface.h>

// LibKface includes

#include <libkface/face.h>

// Local includes

#include "config-digikam.h"
#include "libsinfodlg.h"
#include "rawcameradlg.h"
#include "dbstatdlg.h"
#include "albumsettings.h"

#ifdef HAVE_GPHOTO2

// LibGphoto2 includes

extern "C"
{
#include <gphoto2-version.h>
}

#endif /* HAVE_GPHOTO2 */

namespace Digikam
{

static inline void showDigikamComponentsInfo()
{
    // Set digiKam specific components info list.
    QMap<QString, QString> list;

#ifdef HAVE_GPHOTO2
    list.insert(i18n("LibGphoto2"), QString(gp_library_version(GP_VERSION_SHORT)[0]));
#endif /* HAVE_GPHOTO2 */

    list.insert(i18n("LibKipi"),    KIPI::Interface::version());
    list.insert(i18n("LibOpenCV"),  KFaceIface::LibOpenCVVersion());
    list.insert(i18n("Libface"),    KFaceIface::LibFaceVersion());
    list.insert(i18n("LibKface"),   KFaceIface::version());

    // Database Backend information

    QString dbBe = AlbumSettings::instance()->getImgDatabaseType();
    list.insert(i18n("Database backend"), dbBe);

    if (dbBe != QString("QSQLITE"))
    {
        QString internal = AlbumSettings::instance()->getInternalDatabaseServer() ? i18n("Yes") : i18n("No");
        list.insert(i18n("Database internal server"), internal);
    }

    LibsInfoDlg* dlg = new LibsInfoDlg(kapp->activeWindow());
    dlg->setInfoMap(list);
    dlg->show();
}

static inline void showDigikamDatabaseStat()
{
    DBStatDlg* dlg = new DBStatDlg(kapp->activeWindow());
    dlg->show();
}

static inline void showRawCameraList()
{
    RawCameraDlg* dlg = new RawCameraDlg(kapp->activeWindow());
    dlg->show();
}

}  // namespace Digikam

#endif // COMPONENTS_INFO_H
