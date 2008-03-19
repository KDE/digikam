/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-07-28
 * Description : main program from digiKam
 * 
 * Copyright (C) 2002-2006 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>

// KDE includes
 
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kimageio.h>
#include <ktip.h>
#include <kdeversion.h>
#include <kmessagebox.h>

// KIPI Includes.

#include <libkipi/interface.h>

// Libkexiv2 includes.

#include <libkexiv2/kexiv2.h>

// Libkdcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/dcrawbinary.h>

// C Ansi includes.

extern "C"
{
#include <gphoto2-version.h>
#include <png.h>
}

// Local includes.

#include "version.h"
#include "albummanager.h"
#include "databaseaccess.h"
#include "databaseparameters.h"
#include "scancontroller.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "digikamapp.h"
#include "digikamfirstrun.h"

int main(int argc, char *argv[])
{
    QString DcrawVer    = KDcrawIface::DcrawBinary::internalVersion();

    QString Exiv2Ver    = KExiv2Iface::KExiv2::Exiv2Version();

    QString Gphoto2Ver  = QString(gp_library_version(GP_VERSION_SHORT)[0]);

    QString XmpSupport  = KExiv2Iface::KExiv2::supportXmp() ? I18N_NOOP("yes") : I18N_NOOP("no");

    QString libInfo     = QString(I18N_NOOP("Using Kipi library version %1")).arg(KIPI::Interface::version()) +
                          QString("\n") + 
                          QString(I18N_NOOP("Using KDcraw library version %1")).arg(KDcrawIface::KDcraw::version()) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using Dcraw program version %1")).arg(DcrawVer) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using PNG library version %1")).arg(PNG_LIBPNG_VER_STRING) +
                          QString("\n") + 
                          QString(I18N_NOOP("Using Gphoto2 library version %1")).arg(Gphoto2Ver) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using KExiv2 library version %1")).arg(KExiv2Iface::KExiv2::version()) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using Exiv2 library version %1")).arg(Exiv2Ver) +
                          QString("\n") +                           
                          QString(I18N_NOOP("XMP support available: %1")).arg(XmpSupport);

    KAboutData aboutData( "digikam", 0, 
                          ki18n("digiKam"),
                          digikam_version,        
                          ki18n("A Photo-Management Application for KDE"),
                          KAboutData::License_GPL,
                          ki18n("(c) 2002-2008, digiKam developers team"),
                          KLocalizedString(),
                          "http://www.digikam.org");

    aboutData.setOtherText(ki18n(libInfo.toLatin1()));

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
                          "renchi at pooh.tam.uiuc.edu");

    aboutData.addAuthor ( ki18n("Joern Ahrens"),
                          ki18n("Developer (2004-2005)"),
                          "kde at jokele dot de",
                          "http://www.digikam.org/?q=blog/1");

    aboutData.addAuthor ( ki18n("Tom Albers"),
                          ki18n("Developer (2004-2005)"),
                          "tomalbers at kde dot nl",
                          "http://www.omat.nl/drupal/?q=blog/1");

    aboutData.addAuthor ( ki18n("Ralf Holzer"),
                          ki18n("Developer (2004)"),
                          "kde at ralfhoelzer dot com");

    aboutData.addCredit ( ki18n("Mikolaj Machowski"),
                          ki18n("Bug reports and patches"),
                          "mikmach at wp dot pl");

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

    aboutData.addCredit ( ki18n("Todd Shoemaker"),
                          ki18n("Developer"),
                          "todd at theshoemakers dot net");

    aboutData.addCredit ( ki18n("Gregory Kokanosky"),
                          ki18n("Developer"),
                          "gregory dot kokanosky at free.fr");

    aboutData.addCredit ( ki18n("Rune Laursen"),
                          ki18n("Danish translations"),
                          "runerl at skjoldhoej dot dk");

    aboutData.addCredit ( ki18n("Stefano Rivoir"),
                          ki18n("Italian translations"),
                          "s dot rivoir at gts dot it");

    aboutData.addCredit ( ki18n("Jan Toenjes"),
                          ki18n("German translations"),
                          "jan dot toenjes at web dot de");

    aboutData.addCredit ( ki18n("Oliver Doerr"),
                          ki18n("German translations and beta tester"),
                          "oliver at doerr-privat dot de");

    aboutData.addCredit ( ki18n("Quique"),
                          ki18n("Spanish translations"),
                          "quique at sindominio dot net");

    aboutData.addCredit ( ki18n("Marcus Meissner"),
                          ki18n("Czech translations"),
                          "marcus at jet dot franken dot de");

    aboutData.addCredit ( ki18n("Janos Tamasi"),
                          ki18n("Hungarian translations"),
                          "janusz at vnet dot hu");

    aboutData.addCredit ( ki18n("Jasper van der Marel"),
                          ki18n("Dutch translations"),
                          "jasper dot van dot der dot marel at wanadoo dot nl");

    aboutData.addCredit ( ki18n("Anna Sawicka"),
                          ki18n("Polish translations"),
                          "ania at kajak dot org dot pl");

    aboutData.addCredit ( ki18n("Charles Bouveyron"),
                          ki18n("Beta tester"),
                          "c dot bouveyron at tuxfamily dot org");

    aboutData.addCredit ( ki18n("Richard Groult"),
                          ki18n("Plugin contributor and beta tester"),
                          "Richard dot Groult at jalix dot org");
                                                    
    aboutData.addCredit ( ki18n("Richard Taylor"),
                          ki18n("Feedback and patches. Handbook writer"),
                          "rjt-digicam at thegrindstone dot me dot uk");

    aboutData.addCredit ( ki18n("Hans Karlsson"),
                          ki18n("digiKam website banner and application icons"),
                          "karlsson dot h at home dot se");
    
    aboutData.addCredit ( ki18n("Aaron Seigo"),
                          ki18n("Various usability fixes and general application polishing"),
                          "aseigo at kde.org");

    aboutData.addCredit ( ki18n("Yves Chaufour"),
                          ki18n("digiKam website, Feedback"),
                          "yves dot chaufour at wanadoo dot fr");

    aboutData.addCredit ( ki18n("Tung Nguyen"),
                          ki18n("Bug reports, feedback and icons"),
                          "ntung at free dot fr");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("detect-camera", ki18n("Automatically detect and open camera"));
    options.add("download-from <path>", ki18n("Open camera dialog at <path>"));
    options.add("album-root <path>", ki18n("Start digikam with the album root <path>"));
    KCmdLineArgs::addCmdLineOptions( options ); 

    KApplication app;

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("General Settings");
    QString version = group.readEntry("Version", QString());

    group = config->group("Album Settings");
    QString dbPath = group.readEntry("Database File Path", QString());
    QString albumPath = group.readEntry("Album Path", QString());
    // 0.9 legacy
    if (dbPath.isEmpty())
        dbPath = albumPath;

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    // TEMPORARY SOLUTION
    bool priorityAlbumPath = false;
    if (args && args->isSet("album-root"))
    {
        priorityAlbumPath = true;
        albumPath = args->getOption("album-root");
    }

    QFileInfo dirInfo(dbPath);

    // version 0.6 was the version when the new Albums Library
    // storage was implemented
    if (version.startsWith("0.5") ||
        !dirInfo.exists() || 
        !dirInfo.isDir())
    {
        // Run the first run
        Digikam::DigikamFirstRun *firstRun = new Digikam::DigikamFirstRun();
        app.setTopWidget(firstRun);
        firstRun->show();
        return app.exec();
    }

    // initialize database
    Digikam::AlbumManager* man = Digikam::AlbumManager::instance();
    if (!man->setDatabase(dbPath, priorityAlbumPath))
        return 1;

    // ensure we have one album root
    if (Digikam::CollectionManager::instance()->allLocations().isEmpty())
    {
        Digikam::CollectionManager::instance()->addLocation(albumPath);
    }

    Digikam::DigikamApp *digikam = new Digikam::DigikamApp();

    app.setTopWidget(digikam);
    digikam->show();

    if (args && args->isSet("detect-camera"))
        digikam->autoDetect();
    else if (args && args->isSet("download-from"))
        digikam->downloadFrom(args->getOption("download-from"));

    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");

    KGlobal::locale()->insertCatalog("kipiplugins");

    KTipDialog::showMultiTip(0, tipsFiles, false);

    return app.exec();
}
