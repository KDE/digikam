/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : showFoto is a stand alone version of image
 *               editor with no support of digiKam database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kimageio.h>
#include <klocale.h>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>
#include <libkexiv2/version.h>

// Local includes

#include "daboutdata.h"
#include "showfoto.h"
#include "version.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    KAboutData aboutData("showfoto", "digikam",
                         ki18n("showFoto"),
                         digiKamVersion().toAscii(),  // NOTE: showFoto version = digiKam version
                         DAboutData::digiKamSlogan(),
                         KAboutData::License_GPL,
                         DAboutData::copyright(),
                         additionalInformation(),
                         DAboutData::webProjectUrl().url().toUtf8());

    DAboutData::authorsRegistration(aboutData);

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[file(s) or folder(s)]", ki18n("File(s) or folder(s) to open"));
    KCmdLineArgs::addCmdLineOptions( options );

    KExiv2Iface::KExiv2::initializeExiv2();

    KApplication app;

    KUrl::List urlList;
    KCmdLineArgs* const args = KCmdLineArgs::parsedArgs();

    for (int i = 0; i < args->count(); ++i)
    {
        urlList.append(args->url(i));
    }

    args->clear();

    ShowFoto::ShowFoto* const w = new ShowFoto::ShowFoto(urlList);
    app.setTopWidget(w);
    w->show();

    KGlobal::locale()->setMainCatalog("digikam");
    KGlobal::locale()->insertCatalog("libkdcraw");
    KGlobal::locale()->insertCatalog("libkexiv2");

    int ret = app.exec();

    KExiv2Iface::KExiv2::cleanupExiv2();

    return ret;
}
