/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QStringList>
#include <QString>
#include <QTreeWidget>

// KDE includes

#include <kdeversion.h>
#include <klocale.h>

// Libkexiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Libkgeomap includes

#include <libkgeomap/kgeomap_widget.h>

// C ANSI includes

#ifndef Q_CC_MSVC
extern "C"
{
#endif
#include <jasper/jas_version.h>
#include <png.h>
#include <tiffvers.h>

    // Avoid Warnings under Win32
#undef HAVE_STDLIB_H
#undef HAVE_STDDEF_H
#include <jpeglib.h>

#ifndef Q_CC_MSVC
}
#endif // Q_CC_MSVC

// Local includes

#include "greycstorationfilter.h"
#include "pgfutils.h"
#include "digikam-lcms.h"

#ifndef USE_EXT_LIBLENSFUN
#   ifdef HAVE_GLIB2
#       include "lensfun.h"
#   endif // HAVE_GLIB2
#endif // USE_EXT_LIBLENSFUN

using namespace KExiv2Iface;
using namespace KDcrawIface;
using namespace KGeoMap;

namespace Digikam
{

LibsInfoDlg::LibsInfoDlg(QWidget* const parent)
    : InfoDlg(parent)
{
    setCaption(i18n("Shared Libraries and Components Information"));
    // --------------------------------------------------------
    // By default set a list of common components information used by Showfoto and digiKam.

    QMap<QString, QString> list;
    list.insert(i18n("LibQt"),                       qVersion());
    list.insert(i18n("LibKDE"),                      KDE::versionString());
    list.insert(i18n("LibKdcraw"),                   KDcraw::version());
    list.insert(i18n("LibRaw"),                      KDcraw::librawVersion());

#if KDCRAW_VERSION >= 0x000500
    list.insert(i18n("Parallelized demosaicing"),    KDcraw::librawUseGomp() ?
                i18n("Yes") : i18n("No"));
#endif
    list.insert(i18n("LibKExiv2"),                   KExiv2::version());
    list.insert(i18n("LibExiv2"),                    KExiv2::Exiv2Version());
    list.insert(i18n("Exiv2 supports XMP metadata"), KExiv2::supportXmp() ?
                i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Jpeg"),     KExiv2::supportMetadataWritting("image/jpeg") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Tiff"),     KExiv2::supportMetadataWritting("image/tiff") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Png"),      KExiv2::supportMetadataWritting("image/png") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Jp2"),      KExiv2::supportMetadataWritting("image/jp2") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18n("Exiv2 can write to Pgf"),      KExiv2::supportMetadataWritting("image/pgf") ?
                i18n("Yes") : i18n("No"));

#ifndef USE_EXT_LIBLENSFUN
#   ifdef HAVE_GLIB2
    list.insert(i18n("LibLensFun"),                  i18n("%1.%2.%3-%4 - internal library",
                LF_VERSION_MAJOR,
                LF_VERSION_MINOR,
                LF_VERSION_MICRO,
                LF_VERSION_BUGFIX));
#   endif // HAVE_GLIB2
#else
    list.insert(i18n("LibLensFun"),                  i18n("external shared library"));
#endif // USE_EXT_LIBLENSFUN

#ifndef USE_EXT_LIBLQR
#   ifdef HAVE_GLIB2
    list.insert(i18n("LibLqr"),                      i18n("internal library"));
#   endif // HAVE_GLIB2
#else
    list.insert(i18n("LibLqr"),                      i18n("external shared library"));
#endif // USE_EXT_LIBLQR

    list.insert(i18n("LibPNG"),                      QString(PNG_LIBPNG_VER_STRING));
    list.insert(i18n("LibTIFF"),                     QString(TIFFLIB_VERSION_STR).replace('\n', ' '));
    list.insert(i18n("LibJPEG"),                     QString::number(JPEG_LIB_VERSION));
    list.insert(i18n("LibJasper"),                   QString(jas_getversion()));
    list.insert(i18n("LibCImg"),                     GreycstorationFilter::cimgVersionString());
    list.insert(i18n("LibLCMS"),                     QString::number(LCMS_VERSION));
    list.insert(i18n("LibKGeoMap"),                  KGeoMapWidget::version());
    list.insert(i18n("Marble Widget"),               KGeoMapWidget::MarbleWidgetVersion());

#ifdef USE_EXT_LIBPGF
    list.insert(i18n("LibPGF"),                      QString("%1 - %2").arg(PGFUtils::libPGFVersion()).arg(i18n("external shared library")));
#else
    list.insert(i18n("LibPGF"),                      QString("%1 - %2").arg(PGFUtils::libPGFVersion()).arg(i18n("internal library")));
#endif // USE_EXT_LIBPGF

    list.insert(i18n("Parallelized PGF codec"),      PGFUtils::libPGFUseOpenMP() ? i18n("Yes") : i18n("No"));

#ifdef USE_EXT_LIBCLAPACK
    list.insert(i18n("LibClapack"),                  i18n("external shared library"));
#else
    list.insert(i18n("LibClapack"),                  i18n("internal library"));
#endif // USE_EXT_LIBCLAPACK

    listView()->setHeaderLabels(QStringList() << i18n("Component") << i18n("Info"));
    setInfoMap(list);
}

LibsInfoDlg::~LibsInfoDlg()
{
}

}  // namespace Digikam
