/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : showFoto is a stand alone version of image
 *               editor with no support of digiKam database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kimageio.h>

// Libkexiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Local includes

#include "version.h"
#include "daboutdata.h"
#include "showfoto.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData("showfoto", "digikam",
                         ki18n("showFoto"),
                         digiKamVersion().toAscii(),  // NOTE: showFoto version = digiKam version
                         Digikam::digiKamSlogan(),
                         KAboutData::License_GPL,
                         Digikam::copyright(),
                         KLocalizedString(),
                         Digikam::webProjectUrl().url().toUtf8());

    Digikam::authorsRegistration(aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[file(s) or folder(s)]", ki18n("File(s) or folder(s) to open"));
    KCmdLineArgs::addCmdLineOptions( options );

#if KEXIV2_VERSION >= 0x000300
    KExiv2Iface::KExiv2::initializeExiv2();
#endif

    KApplication app;

    KUrl::List urlList;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for(int i = 0; i < args->count(); ++i)
    {
        urlList.append(args->url(i));
    }
    args->clear();

    ShowFoto::ShowFoto *w = new ShowFoto::ShowFoto(urlList);
    app.setTopWidget(w);
    w->show();

    KGlobal::locale()->setMainCatalog("digikam");
    KGlobal::locale()->insertCatalog("libkdcraw");

    int ret = app.exec();

#if KEXIV2_VERSION >= 0x000300
    KExiv2Iface::KExiv2::cleanupExiv2();
#endif

    return ret;
}
