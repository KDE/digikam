/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-11-22
 * Description :
 *
 * Copyright 2004-2006 by Renchi Raju, Gilles Caulier
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

// Local includes.

#include "showfoto.h"

static KCmdLineOptions options[] =
{
    { "+[file(s)]", I18N_NOOP("File(s) to open"), 0 },
    KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    QString Description = i18n("KDE Photo Viewer and Editor");

    KAboutData aboutData( "showfoto",
                          I18N_NOOP("showFoto"),
                          "0.4.0",
                          Description.latin1(),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2004-2006, digiKam developers team"),
                          0,
                          "http://www.digikam.org");

    aboutData.addAuthor ( "Caulier Gilles",
                          I18N_NOOP("Main developer and coordinator"),
                          "caulier dot gilles at kdemail dot net",
                          "http://www.digikam.org/?q=blog/3");

    aboutData.addAuthor ( "Marcel Wiesweg",
                          I18N_NOOP("Developer"),
                          "marcel dot wiesweg at gmx dot de",
                          "http://www.digikam.org/?q=blog/8");

    aboutData.addAuthor ( "Francisco J. Cruz",
                          I18N_NOOP("Developer"),
                          "fj dot cruz at supercable dot es",
                          "http://www.digikam.org/?q=blog/5");

    aboutData.addAuthor ( "Renchi Raju",
                          I18N_NOOP("Developer"),
                          "renchi at pooh.tam.uiuc.edu",
                          0);
    
    aboutData.addAuthor ( "Ralf Holzer",
                          I18N_NOOP("Developer"),
                          "kde at ralfhoelzer.com",
                          0);

    aboutData.addAuthor ( "Joern Ahrens",
                          I18N_NOOP("Developer"),
                          "joern dot ahrens at kdemail dot net",
                          "http://www.digikam.org/?q=blog/1");
    
    aboutData.addAuthor ( "Tom Albers",
                          I18N_NOOP("Developer"),
                          "tomalbers at kde.nl",
                          "http://www.omat.nl/drupal/?q=blog/1");
                          
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
    
    KGlobal::locale()->insertCatalogue("digikamimageplugins");

    return app.exec();

    delete w;
}

