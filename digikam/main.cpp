/* ============================================================
 * Authors: Renchi Raju <renchi at pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2002-07-28
 * Description : main program from digiKam
 * 
 * Copyright 2002-2006 by Renchi Raju and Gilles Caulier
 * Copyright 2007 by Gilles Caulier
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

// Qt includes.

#include <qstring.h>
#include <qstringlist.h>
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

#include <libkipi/version.h>

// libkexiv2 includes.

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// C Ansi includes.

extern "C"
{
#include <gphoto2-version.h>
#include <png.h>
}

// Local includes.

#include "version.h"
#include "albumdb.h"
#include "albummanager.h"
#include "digikamapp.h"
#include "digikamfirstrun.h"

static KCmdLineOptions options[] =
{
    { "detect-camera", I18N_NOOP("Automatically detect and open camera"), 0 },
    { "download-from <path>", I18N_NOOP("Open camera dialog at <path>"), 0 },
    KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    QString Exiv2Ver    = KExiv2Iface::KExiv2::Exiv2Version();

    QString Gphoto2Ver  = QString(gp_library_version(GP_VERSION_SHORT)[0]);

    QString libInfo     = QString(I18N_NOOP("Using Kipi library version %1")).arg(kipi_version) +
                          QString("\n") + 
                          QString(I18N_NOOP("Using KExiv2 library version %1")).arg(kexiv2_version) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using Exiv2 library version %1")).arg(Exiv2Ver) +
                          QString("\n") +                           
                          QString(I18N_NOOP("Using PNG library version %1")).arg(PNG_LIBPNG_VER_STRING) +
                          QString("\n") + 
                          QString(I18N_NOOP("Using Gphoto2 library version %1")).arg(Gphoto2Ver);

    QString description = QString(I18N_NOOP("A Photo-Management Application for KDE"));
    
    KAboutData aboutData( "digikam", 
                          I18N_NOOP("digiKam"),
                          digikam_version,        
                          description.latin1(),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2002-2007, digiKam developers team"),
                          0,
                          "http://www.digikam.org");

    aboutData.setOtherText(libInfo.latin1());

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
                          "kde at ralfhoelzer dot com",
                          0);

    aboutData.addAuthor ( "Joern Ahrens",
                          I18N_NOOP("Developer"),
                          "kde at jokele dot de",
                          "http://www.digikam.org/?q=blog/1");

    aboutData.addAuthor ( "Tom Albers",
                          I18N_NOOP("Developer"),
                          "tomalbers at kde dot nl",
                          "http://www.omat.nl/drupal/?q=blog/1");

    aboutData.addCredit ( "Achim Bohnet",
                          I18N_NOOP("Bug reports and patches"),
                          "ach at mpe dot mpg dot de",
                          0);

    aboutData.addCredit ( "Luka Renko",
                          I18N_NOOP("Developer"),
                          "lure at kubuntu dot org",
                          0);

    aboutData.addCredit ( "Fabien Salvi",
                          I18N_NOOP("Webmaster"),
                          "fabien dot ubuntu at gmail dot com",
                          0);

    aboutData.addCredit ( "Todd Shoemaker",
                          I18N_NOOP("Developer"),
                          "todd at theshoemakers dot net",
                          0);

    aboutData.addCredit ( "Gregory Kokanosky",
                          I18N_NOOP("Developer"),
                          "gregory dot kokanosky at free.fr",
                          0);

    aboutData.addCredit ( "Rune Laursen",
                          I18N_NOOP("Danish translations"),
                          "runerl at skjoldhoej dot dk",
                          0);

    aboutData.addCredit ( "Stefano Rivoir",
                          I18N_NOOP("Italian translations"),
                          "s dot rivoir at gts dot it",
                          0);

    aboutData.addCredit ( "Jan Toenjes",
                          I18N_NOOP("German translations"),
                          "jan dot toenjes at web dot de",
                          0);

    aboutData.addCredit ( "Oliver Doerr",
                          I18N_NOOP("German translations and beta tester"),
                          "oliver at doerr-privat dot de",
                          0);

    aboutData.addCredit ( "Quique",
                          I18N_NOOP("Spanish translations"),
                          "quique at sindominio dot net",
                          0);

    aboutData.addCredit ( "Marcus Meissner",
                          I18N_NOOP("Czech translations"),
                          "marcus at jet dot franken dot de",
                          0);

    aboutData.addCredit ( "Janos Tamasi",
                          I18N_NOOP("Hungarian translations"),
                          "janusz at vnet dot hu",
                          0);

    aboutData.addCredit ( "Jasper van der Marel",
                          I18N_NOOP("Dutch translations"),
                          "jasper dot van dot der dot marel at wanadoo dot nl",
                          0);

    aboutData.addCredit ( "Anna Sawicka",
                          I18N_NOOP("Polish translations"),
                          "ania at kajak dot org dot pl",
                          0);

    aboutData.addCredit ( "Charles Bouveyron",
                          I18N_NOOP("Beta tester"),
                          "c dot bouveyron at tuxfamily dot org",
                          0);

    aboutData.addCredit ( "Richard Groult",
                          I18N_NOOP("Plugin contributor and beta tester"),
                          "Richard dot Groult at jalix dot org",
                          0);
                                                    
    aboutData.addCredit ( "Richard Taylor",
                          I18N_NOOP("Feedback and patches. Handbook writer"),
                          "rjt-digicam at thegrindstone dot me dot uk",
                          0);

    aboutData.addCredit ( "Hans Karlsson",
                          I18N_NOOP("digiKam website banner and application icons"),
                          "karlsson dot h at home dot se",
                          0);
    
    aboutData.addCredit ( "Aaron Seigo",
                          I18N_NOOP("Various usability fixes and general application polishing"),
                          "aseigo at kde.org",
                          0);

    aboutData.addCredit ( "Yves Chaufour",
                          I18N_NOOP("digiKam website, Feedback"),
                          "yves dot chaufour at wanadoo dot fr",
                          0);

    aboutData.addCredit ( "Tung Nguyen",
                          I18N_NOOP("Bug reports, feedback and icons"),
                          "ntung at free dot fr",
                          0);

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); 

    KApplication app;

    KConfig* config = KGlobal::config();
    config->setGroup("General Settings");
    QString version = config->readEntry("Version");

    config->setGroup("Album Settings");
    QString albumPath = config->readPathEntry("Album Path");
    QFileInfo dirInfo(albumPath);

    // version 0.6 was the version when the new Albums Library
    // storage was implemented
    
    if (version.startsWith("0.5") ||
        !dirInfo.exists() || 
        !dirInfo.isDir())
    {
        // Run the first run
        Digikam::DigikamFirstRun *firstRun = new Digikam::DigikamFirstRun(config);
        app.setMainWidget(firstRun);
        firstRun->show();
        return app.exec();
    }

    Digikam::AlbumManager* man = new Digikam::AlbumManager();
    man->setLibraryPath(albumPath);

    // Register image formats (especially for TIFF )
    KImageIO::registerFormats();

    Digikam::DigikamApp *digikam = new Digikam::DigikamApp();

    app.setMainWidget(digikam);
    digikam->show();
    
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (args && args->isSet("detect-camera"))
        digikam->autoDetect();
    else if (args && args->isSet("download-from"))
        digikam->downloadFrom(args->getOption("download-from"));

#if KDE_IS_VERSION(3,2,0)
    QStringList tipsFiles;
    tipsFiles.append("digikam/tips");
    tipsFiles.append("kipi/tips");
    tipsFiles.append("digikamimageplugins/tips");

    KGlobal::locale()->insertCatalogue("kipiplugins");
    KGlobal::locale()->insertCatalogue("digikamimageplugins");
    
    KTipDialog::showMultiTip(0, tipsFiles, false);
#else
    KTipDialog::showTip(0, "digikam/tips", false);
#endif

    return app.exec();
}
