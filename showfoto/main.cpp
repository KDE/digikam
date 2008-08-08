/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : showfoto is a stand alone version of image
 *               editor with no support of digiKam database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kimageio.h>

// Libkexiv2 includes.

#include <libkexiv2/kexiv2.h>
#include <libkexiv2/version.h>

// Libkdcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/dcrawbinary.h>

// C Ansi includes.

extern "C"
{
#include <png.h>
}

// Local includes.

#include "daboutdata.h"
#include "showfoto.h"

static KCmdLineOptions options[] =
{
    { "+[file(s) or folder(s)]", I18N_NOOP("File(s) or folder(s) to open"), 0 },
    KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    QString DcrawVer    = KDcrawIface::DcrawBinary::internalVersion();

    QString Exiv2Ver    = KExiv2Iface::KExiv2::Exiv2Version();

    QString Kexiv2Ver;

#if KEXIV2_VERSION <= 0x000106
    Kexiv2Ver = QString(kexiv2_version);
#else
    Kexiv2Ver = KExiv2Iface::KExiv2::version();
#endif

    QString libInfo     = QString(I18N_NOOP("Using KExiv2 library version %1")).arg(Kexiv2Ver) +
                          QString("\n") +
                          QString(I18N_NOOP("Using Exiv2 library version %1")).arg(Exiv2Ver) +
                          QString("\n") +
                          QString(I18N_NOOP("Using KDcraw library version %1")).arg(KDcrawIface::KDcraw::version()) +
                          QString("\n") +
                          QString(I18N_NOOP("Using Dcraw program version %1")).arg(DcrawVer) +
                          QString("\n") +
                          QString(I18N_NOOP("Using PNG library version %1")).arg(PNG_LIBPNG_VER_STRING);

    QString Description = Digikam::showFotoDescription();

    KAboutData aboutData( "showfoto",
                          I18N_NOOP("showFoto"),
                          showfoto_version,
                          Description.latin1(),
                          KAboutData::License_GPL,
                          Digikam::copyright(),
                          0,
                          Digikam::webProjectUrl());

    aboutData.setOtherText(libInfo.latin1());

    Digikam::authorsRegistration(aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );

    KGlobal::locale()->setMainCatalogue( "digikam" );

    KApplication app;
    KImageIO::registerFormats();

    KURL::List urlList;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for(int i = 0; i < args->count(); i++)
    {
        urlList.append(args->url(i));
    }
    args->clear();

    ShowFoto::ShowFoto *w = new ShowFoto::ShowFoto(urlList);
    app.setMainWidget(w);
    w->show();

    return app.exec();

    delete w;
}
