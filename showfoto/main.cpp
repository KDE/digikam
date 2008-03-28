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

int main(int argc, char *argv[])
{
    QString DcrawVer    = KDcrawIface::DcrawBinary::internalVersion();

    QString Exiv2Ver    = KExiv2Iface::KExiv2::Exiv2Version();

    QString XmpSupport  = KExiv2Iface::KExiv2::supportXmp() ? I18N_NOOP("yes") : I18N_NOOP("no");

    KLocalizedString libInfo = ki18n("Using KDcraw library version %2\n"
                                     "Using Dcraw program version %3\n"
                                     "Using PNG library version %4\n"
                                     "Using KExiv2 library version %6\n"
                                     "Using Exiv2 library version %7\n"
                                     "XMP support available: %8")
                               .subs(KDcrawIface::KDcraw::version())
                               .subs(DcrawVer)
                               .subs(PNG_LIBPNG_VER_STRING)
                               .subs(KExiv2Iface::KExiv2::version())
                               .subs(Exiv2Ver)
                               .subs(XmpSupport);

    KAboutData aboutData( "showfoto", 0,
                          ki18n("showFoto"),
                          "0.10.0",
                          ki18n("KDE Photo Viewer and Editor"),
                          KAboutData::License_GPL,
                          ki18n("(c) 2004-2008, digiKam developers team"),
                          KLocalizedString(),
                          "http://www.digikam.org");

    aboutData.setOtherText(libInfo);

    aboutData.addAuthor ( ki18n("Caulier Gilles"),
                          ki18n("Main developer and coordinator"),
                          "caulier dot gilles at gmail dot com",
                          "http://www.digikam.org/?q=blog/3");

    aboutData.addAuthor ( ki18n("Marcel Wiesweg"),
                          ki18n("Developer"),
                          "marcel dot wiesweg at gmx dot de",
                          "http://www.digikam.org/?q=blog/8");

    aboutData.addAuthor ( ki18n("Arnd Baecker"),
                          ki18n("Developer"),
                          "arnd dot baecker at web dot de",
                          "http://www.digikam.org/?q=blog/133");

    aboutData.addAuthor ( ki18n("Francisco J. Cruz"),
                          ki18n("Developer"),
                          "fj dot cruz at supercable dot es",
                          "http://www.digikam.org/?q=blog/5");

    aboutData.addAuthor ( ki18n("Renchi Raju"),
                          ki18n("Developer (2002-2005)"),
                          "renchi at pooh dot tam dot uiuc dot edu");

    aboutData.addAuthor ( ki18n("Joern Ahrens (2004-2005)"),
                          ki18n("Developer"),
                          "joern dot ahrens at kdemail dot net",
                          "http://www.digikam.org/?q=blog/1");

    aboutData.addAuthor ( ki18n("Tom Albers"),
                          ki18n("Developer (2004-2005)"),
                          "tomalbers at kde dot nl",
                          "http://www.omat.nl/drupal/?q=blog/1");

    aboutData.addAuthor ( ki18n("Ralf Holzer"),
                          ki18n("Developer (2004)"),
                          "kde at ralfhoelzer dot com");

    aboutData.addCredit ( ki18n("Achim Bohnet"),
                          ki18n("Bug reports and patches"),
                          "ach at mpe dot mpg dot de");

    aboutData.addCredit ( ki18n("Luka Renko"),
                          ki18n("Developer"),
                          "lure at kubuntu dot org");

    aboutData.addCredit ( ki18n("Angelo Naselli"),
                          ki18n("Developer"),
                          "a dot naselli at libero dot it");

    aboutData.addCredit ( ki18n("Fabien Salvi"),
                          ki18n("Webmaster"),
                          "fabien dot ubuntu at gmail dot com");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[file(s) or folder(s)]", ki18n("File(s) or folder(s) to open"));
    KCmdLineArgs::addCmdLineOptions( options );

    KGlobal::locale()->setMainCatalog( "digikam" );

    KApplication app;

    KUrl::List urlList;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    for(int i = 0; i < args->count(); i++)
    {
        urlList.append(args->url(i));
    }
    args->clear();

    ShowFoto::ShowFoto *w = new ShowFoto::ShowFoto(urlList);
    app.setTopWidget(w);
    w->show();

    return app.exec();

    delete w;
}
