/* ============================================================
 * File  : main.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-11-22
 * Description :
 *
 * Copyright 2004-2005 by Renchi Raju

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

#include "showfoto.h"

static KCmdLineOptions options[] =
{
    { "+[file(s)]", I18N_NOOP("File(s) to open"), 0 },
        KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    QString Description = i18n("KDE Photo Viewer and editor");

    KAboutData aboutData( "showfoto",
                          I18N_NOOP("showFoto"),
                          "0.2.0",
                          Description.latin1(),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2004-2005, digiKam developers team"),
                          0,
                          "http://digikam.sourceforge.net");

    aboutData.addAuthor ( "Renchi Raju",
                          I18N_NOOP("Developer"),
                          "renchi at pooh.tam.uiuc.edu",
                          "http://digikam.sourceforge.net");

    aboutData.addAuthor ( "Caulier Gilles",
                          I18N_NOOP("Developer"),
                          "caulier dot gilles at free.fr",
                          "http://caulier.gilles.free.fr");

    aboutData.addAuthor ( "Ralf Holzer",
                          I18N_NOOP("Developer"),
                          "kde at ralfhoelzer.com",
                          0);

    aboutData.addAuthor ( "Joern Ahrens",
                          I18N_NOOP("Developer"),
                          "joern dot ahrens at kdemail dot net",
                          0);

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

    ShowFoto *w = new ShowFoto(urlList);
    app.setMainWidget(w);
    w->show();
    
    KGlobal::locale()->insertCatalogue("digikamimageplugins");

    return app.exec();
}

