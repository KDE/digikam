/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "libsinfodlg.h"
#include "libsinfodlg.moc"

// Qt includes

#include <QStringList>
#include <QString>
#include <QTreeWidget>
#include <QHeaderView>

// KDE includes

#include <klocale.h>
#include <kapplication.h>
#include <kaboutdata.h>

#include "config-digikam.h"
#ifdef HAVE_MARBLEWIDGET
#include <marble/global.h>
using namespace Marble;
#endif // HAVE_MARBLEWIDGET

// Libkexiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// C ANSI includes

#ifndef Q_CC_MSVC
extern "C"
{
#endif
#include <jasper/jas_version.h>
#include <png.h>
#include <tiffvers.h>
#include <lcms.h>

// Avoid Warnings under Win32
#undef HAVE_STDLIB_H
#undef HAVE_STDDEF_H
#include <jpeglib.h>

#ifndef Q_CC_MSVC
}
#endif

// Local includes

#include "daboutdata.h"
#include "greycstorationiface.h"
#include "pgfutils.h"

namespace Digikam
{

LibsInfoDlg::LibsInfoDlg(QWidget *parent)
           : InfoDlg(parent)
{
    setCaption(i18n("Shared Libraries and Components Information"));
    // --------------------------------------------------------
    // By default set a list of common components information used by Showfoto and digiKam.

    QMap<QString, QString> list;
    list.insert(i18n("LibQt"),                       qVersion());
    list.insert(i18n("LibKDE"),                      KDE::versionString());
    list.insert(i18n("LibKdcraw"),                   KDcrawIface::KDcraw::version());
#if KDCRAW_VERSION < 0x000400
    list.insert(i18n("Dcraw program"),               KDcrawIface::DcrawBinary::internalVersion());
#else
    list.insert(i18n("LibRaw"),                      KDcrawIface::KDcraw::librawVersion());
#endif

#if KDCRAW_VERSION >= 0x000500
    list.insert(i18n("Parallelized demosaicing"),    KDcrawIface::KDcraw::librawUseGomp() ?
                                                     i18n("Yes") : i18n("No"));
#endif
    list.insert(i18n("LibKExiv2"),                   KExiv2Iface::KExiv2::version());
    list.insert(i18n("LibExiv2"),                    KExiv2Iface::KExiv2::Exiv2Version());
    list.insert(i18n("Exiv2 supports XMP metadata"), KExiv2Iface::KExiv2::supportXmp() ?
                                                     i18n("Yes") : i18n("No"));
#if KEXIV2_VERSION >= 0x000300
    list.insert(i18n("Exiv2 can write to Jpeg"),     KExiv2Iface::KExiv2::supportMetadataWritting("image/jpeg") ?
                                                     i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Tiff"),     KExiv2Iface::KExiv2::supportMetadataWritting("image/tiff") ?
                                                     i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Png"),      KExiv2Iface::KExiv2::supportMetadataWritting("image/png") ?
                                                     i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Jp2"),      KExiv2Iface::KExiv2::supportMetadataWritting("image/jp2") ?
                                                     i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Pgf"),      KExiv2Iface::KExiv2::supportMetadataWritting("image/pgf") ?
                                                     i18n("Yes") : i18n("No"));
#endif

    list.insert(i18n("LibPNG"),                      QString(PNG_LIBPNG_VER_STRING));
    list.insert(i18n("LibTIFF"),                     QString(TIFFLIB_VERSION_STR).replace('\n', ' '));
    list.insert(i18n("LibJPEG"),                     QString::number(JPEG_LIB_VERSION));
    list.insert(i18n("LibJasper"),                   QString(jas_getversion()));
    list.insert(i18n("LibCImg"),                     GreycstorationIface::cimgVersionString());
    list.insert(i18n("LibLCMS"),                     QString::number(LCMS_VERSION));
    list.insert(i18n("LibPGF"),                      libPGFVersion());

#ifdef HAVE_MARBLEWIDGET
    list.insert(i18n("Marble widget"),               QString(MARBLE_VERSION_STRING));
#endif //HAVE_MARBLEWIDGET

    listView()->setHeaderLabels(QStringList() << i18n("Component") << i18n("Info"));
    setInfoMap(list);
}

LibsInfoDlg::~LibsInfoDlg()
{
}

}  // namespace Digikam
