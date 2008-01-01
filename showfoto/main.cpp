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

// Libkdcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/dcrawbinary.h>

// C Ansi includes.

extern "C"
{
#include <png.h>
}

// Local includes.

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

    QString libInfo     = QString(I18N_NOOP("Using KExiv2 library version %1")).arg(KExiv2Iface::KExiv2::version()) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using Exiv2 library version %1")).arg(Exiv2Ver) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using KDcraw library version %1")).arg(KDcrawIface::KDcraw::version()) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using Dcraw program version %1")).arg(DcrawVer) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using PNG library version %1")).arg(PNG_LIBPNG_VER_STRING);

    QString Description = i18n("KDE Photo Viewer and Editor");

    KAboutData aboutData( "showfoto",
                          I18N_NOOP("showFoto"),
                          "0.8.0",
                          Description.latin1(),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2004-2008, digiKam developers team"),
                          0,
                          "http://www.digikam.org");

    aboutData.setOtherText(libInfo.latin1());

    aboutData.addAuthor ( "Caulier Gilles",
                          I18N_NOOP("Main developer and coordinator"),
                          "caulier dot gilles at gmail dot com",
                          "http://www.digikam.org/?q=blog/3");

    aboutData.addAuthor ( "Marcel Wiesweg",
                          I18N_NOOP("Developer"),
                          "marcel dot wiesweg at gmx dot de",
                          "http://www.digikam.org/?q=blog/8");

    aboutData.addAuthor ( "Arnd Baecker",
                          I18N_NOOP("Developer"),
                          "arnd dot baecker at web dot de",
                          "http://www.digikam.org/?q=blog/133");

    aboutData.addAuthor ( "Francisco J. Cruz",
                          I18N_NOOP("Developer"),
                          "fj dot cruz at supercable dot es",
                          "http://www.digikam.org/?q=blog/5");

    aboutData.addAuthor ( "Renchi Raju",
                          I18N_NOOP("Developer (2002-2005"),
                          "renchi at pooh dot tam dot uiuc dot edu",
                          0);
    
    aboutData.addAuthor ( "Joern Ahrens",
                          I18N_NOOP("Developer (2004-2005)"),
                          "joern dot ahrens at kdemail dot net",
                          "http://www.digikam.org/?q=blog/1");
    
    aboutData.addAuthor ( "Tom Albers",
                          I18N_NOOP("Developer (2004-2005)"),
                          "tomalbers at kde dot nl",
                          "http://www.omat.nl/drupal/?q=blog/1");

    aboutData.addAuthor ( "Ralf Holzer",
                          I18N_NOOP("Developer (2004)"),
                          "kde at ralfhoelzer dot com",
                          0);

    aboutData.addCredit ( "Mikolaj Machowski",
                          I18N_NOOP("Bug reports and patches"),
                          "mikmach at wp dot pl",
                          0);

    aboutData.addCredit ( "Achim Bohnet",
                          I18N_NOOP("Bug reports and patches"),
                          "ach at mpe dot mpg dot de",
                          0);

    aboutData.addCredit ( "Luka Renko",
                          I18N_NOOP("Developer"),
                          "lure at kubuntu dot org",
                          0);

    aboutData.addCredit ( "Angelo Naselli",
                          I18N_NOOP("Developer"),
                          "anaselli at linux dot it",
                          0);

    aboutData.addCredit ( "Fabien Salvi",
                          I18N_NOOP("Webmaster"),
                          "fabien dot ubuntu at gmail dot com",
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

    ShowFoto::ShowFoto *w = new ShowFoto::ShowFoto(urlList);
    app.setMainWidget(w);
    w->show();
    
    return app.exec();

    delete w;
}
