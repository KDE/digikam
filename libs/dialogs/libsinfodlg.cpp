/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog
 *
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "digikam_config.h"

// Qt includes

#include <QStringList>
#include <QString>
#include <QTreeWidget>
#include <QThreadPool>

// KDE includes

#include <kcoreaddons_version.h>
#include <klocalizedstring.h>

// Libkexiv2 includes

#include <libkexiv2_version.h>
#include <kexiv2.h>

// Libkdcraw includes

#include <libkdcraw_version.h>
#include <kdcraw.h>

#ifdef HAVE_KGEOMAP
// Libkgeomap includes

#include <kgeomap_widget.h>

using namespace KGeoMap;
#endif // HAVE_KGEOMAP

// C ANSI includes

#ifndef Q_CC_MSVC
extern "C"
{
#endif

#ifdef HAVE_JASPER
#include <jasper/jas_version.h>
#endif // HAVE_JASPER

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

#ifdef HAVE_LENSFUN
#include "lensfuniface.h"
#endif // HAVE_LENSFUN

using namespace KExiv2Iface;
using namespace KDcrawIface;

namespace Digikam
{

LibsInfoDlg::LibsInfoDlg(QWidget* const parent)
    : InfoDlg(parent)
{
    setWindowTitle(i18n("Shared Libraries and Components Information"));

    // --------------------------------------------------------
    // By default set a list of common components information used by Showfoto and digiKam.

    static const char* CONTEXT = "Component information, see help->components";
    static const QString SUPPORTED_YES = i18nc("component is supported/available",     "Yes");
    static const QString SUPPORTED_NO  = i18nc("component is not available/supported", "No");

    QMap<QString, QString> list;
    list.insert(i18nc(CONTEXT, "Qt"),                          qVersion());
    list.insert(i18nc(CONTEXT, "KCoreAddons"),                 QString(KCOREADDONS_VERSION_STRING));
    list.insert(i18nc(CONTEXT, "KDcraw"),                      KDcraw::version());
    list.insert(i18nc(CONTEXT, "LibRaw"),                      KDcraw::librawVersion());

#if KDCRAW_VERSION >= 0x000500
    list.insert(i18nc(CONTEXT, "Parallelized demosaicing"),    checkTriState(KDcraw::librawUseGomp()));
#endif

#if KDCRAW_VERSION >= 0x020400
    list.insert(i18nc(CONTEXT, "Demosaic GPL2 pack support"),  checkTriState(KDcraw::librawUseGPL2DemosaicPack()));
    list.insert(i18nc(CONTEXT, "Demosaic GPL3 pack support"),  checkTriState(KDcraw::librawUseGPL3DemosaicPack()));
#endif

#if KDCRAW_VERSION >= 0x020200
    list.insert(i18nc(CONTEXT, "RawSpeed codec support"),      checkTriState(KDcraw::librawUseRawSpeed()));
#endif

#ifdef HAVE_EIGEN3
    list.insert(i18nc(CONTEXT, "Eigen"),                       QString(EIGEN3_VERSION_STRING));
#else
    list.insert(i18nc(CONTEXT, "Eigen support"),               SUPPORTED_NO);
#endif // HAVE_EIGEN3

    list.insert(i18nc(CONTEXT, "KExiv2"),                      KExiv2::version());
    list.insert(i18nc(CONTEXT, "Exiv2"),                       KExiv2::Exiv2Version());
    list.insert(i18nc(CONTEXT, "Exiv2 supports XMP metadata"), KExiv2::supportXmp() ?
                i18n("Yes") : i18n("No"));
    list.insert(i18nc(CONTEXT, "Exiv2 can write to Jpeg"),     KExiv2::supportMetadataWritting("image/jpeg") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18nc(CONTEXT, "Exiv2 can write to Tiff"),     KExiv2::supportMetadataWritting("image/tiff") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18nc(CONTEXT, "Exiv2 can write to Png"),      KExiv2::supportMetadataWritting("image/png") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18nc(CONTEXT, "Exiv2 can write to Jp2"),      KExiv2::supportMetadataWritting("image/jp2") ?
                i18n("Yes") : i18n("No"));
    list.insert(i18nc(CONTEXT, "Exiv2 can write to Pgf"),      KExiv2::supportMetadataWritting("image/pgf") ?
                i18n("Yes") : i18n("No"));

#ifdef HAVE_LENSFUN
    list.insert(i18nc(CONTEXT, "LensFun"),                     LensFunIface::lensFunVersion());
#else
    list.insert(i18nc(CONTEXT, "LensFun support"),             SUPPORTED_NO);
#endif // HAVE_LENSFUN

#ifdef HAVE_LIBLQR_1
    list.insert(i18nc(CONTEXT, "LibLqr support"),              SUPPORTED_YES);
#else
    list.insert(i18nc(CONTEXT, "LibLqr support"),              SUPPORTED_NO);
#endif // HAVE_LIBLQR_1

    list.insert(i18nc(CONTEXT, "LibPNG"),                      QString(PNG_LIBPNG_VER_STRING));
    list.insert(i18nc(CONTEXT, "LibTIFF"),                     QString(TIFFLIB_VERSION_STR).replace('\n', ' '));
    list.insert(i18nc(CONTEXT, "LibJPEG"),                     QString::number(JPEG_LIB_VERSION));
    list.insert(i18nc(CONTEXT, "LibCImg"),                     GreycstorationFilter::cimgVersionString());
    list.insert(i18nc(CONTEXT, "LibLCMS"),                     QString::number(LCMS_VERSION));
    list.insert(i18nc(CONTEXT, "LibPGF"),                      PGFUtils::libPGFVersion());

#ifdef HAVE_JASPER
    list.insert(i18nc(CONTEXT, "LibJasper"),                   QString(jas_getversion()));
#else
    list.insert(i18nc(CONTEXT, "LibJasper support"),           SUPPORTED_NO);
#endif // HAVE_JASPER

#ifdef HAVE_KGEOMAP
    list.insert(i18nc(CONTEXT, "KGeoMap"),                     KGeoMapWidget::version());
    list.insert(i18nc(CONTEXT, "Marble"),                      KGeoMapWidget::MarbleWidgetVersion());
#else
    list.insert(i18nc(CONTEXT, "KGeoMap support"),             SUPPORTED_NO);
#endif // HAVE_KGEOMAP

// TODO add sqlite versions here? Could be useful for debugging sqlite problems..

    int nbcore = QThreadPool::globalInstance()->maxThreadCount();
    list.insert(i18ncp(CONTEXT, "CPU core", "CPU cores", nbcore), QString("%1").arg(nbcore));

    listView()->setHeaderLabels(QStringList() << i18nc("Name of the component", "Component") << i18nc("Is supported / version of the component", "Info"));
    setInfoMap(list);
}

LibsInfoDlg::~LibsInfoDlg()
{
}

QString LibsInfoDlg::checkTriState(int value) const
{
    switch(value)
    {
        case true:
            return i18n("Yes");
        case false:
            return i18n("No");
        default:
            return i18n("Unknown");
    }
}

}  // namespace Digikam
