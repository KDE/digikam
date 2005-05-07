/* ============================================================
 * Authors: Renchi Raju <renchi at pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date   : 2002-07-28
 * Description : 
 * 
 * Copyright 2002-2004 by Renchi Raju and Gilles Caulier
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

extern "C"
{
#include <locale.h>
}

// Local includes.

#include "version.h"
#include "scanlib.h"
#include "albummanager.h"
#include "digikamapp.h"
#include "digikamfirstrun.h"

// TODO: Only for testing purposes for 0.8 development. Remove
// for production version
static bool copyDBFile(const QString& albumLibraryPath)
{
    QFile sFile(albumLibraryPath + "/digikam.db");
    QFile dFile(albumLibraryPath + "/digikam-testing.db");

    if (!sFile.exists() || dFile.exists())
        return true;

    if (!sFile.open(IO_ReadOnly))
        return false;
    
    if (!dFile.open(IO_WriteOnly))
    {
        sFile.close();
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);
    char buffer[MAX_IPC_SIZE];

    Q_LONG len;
    while ((len = sFile.readBlock(buffer, MAX_IPC_SIZE)) != 0)
    {
        if (len == -1 || dFile.writeBlock(buffer, (Q_ULONG)len) == -1)
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();
    
    return true;
}

static KCmdLineOptions options[] =
{
    { "detect-camera", I18N_NOOP("Automatically detect and open camera"), 0 },
    KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    QString description = i18n("A Photo-Management Application for KDE") + "\n" + 
                          i18n("Using Kipi library version %1").arg(kipi_version);
    
    KAboutData aboutData( "digikam", 
                          I18N_NOOP("digiKam"),
                          digikam_version,        
                          description.latin1(),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2002-2005, Digikam developers team"),
                          0,
                          "http://digikam.sourceforge.net");

    aboutData.addAuthor ( "Renchi Raju",
                          I18N_NOOP("Main coordinator and developer"),
                          "renchi at pooh.tam.uiuc.edu",
                          "http://digikam.sourceforge.net");

    aboutData.addAuthor ( "Caulier Gilles",
                          I18N_NOOP("Developer, co-coordinator, French translations"),
                          "caulier dot gilles at free.fr",
                          "http://caulier.gilles.free.fr");

    aboutData.addAuthor ( "Joern Ahrens",
                          I18N_NOOP("Developer"),
                          "kde at jokele.de",
                          0);

    aboutData.addAuthor ( "Ralf Holzer",
                          I18N_NOOP("Developer"),
                          "kde at ralfhoelzer.com",
                          0);
    
    aboutData.addCredit ( "Todd Shoemaker",
                          I18N_NOOP("Developer"),
                          "todd at theshoemakers.net",
                          0);

    aboutData.addCredit ( "Gregory Kokanosky",
                          I18N_NOOP("Developer"),
                          "gregory dot kokanosky at free.fr",
                          0);

    aboutData.addCredit ( "Rune Laursen",
                          I18N_NOOP("Danish translations"),
                          "runerl at skjoldhoej.dk",
                          0);

    aboutData.addCredit ( "Stefano Rivoir",
                          I18N_NOOP("Italian translations"),
                          "s dot rivoir at gts.it",
                          0);

    aboutData.addCredit ( "Jan Toenjes",
                          I18N_NOOP("German translations"),
                          "jan dot toenjes at web.de",
                          0);

    aboutData.addCredit ( "Oliver Doerr",
                          I18N_NOOP("German translations and beta tester"),
                          "oliver at doerr-privat.de",
                          0);

    aboutData.addCredit ( "Quique",
                          I18N_NOOP("Spanish translations"),
                          "quique at sindominio.net",
                          0);

    aboutData.addCredit ( "Marcus Meissner",
                          I18N_NOOP("Czech translations"),
                          "marcus at jet.franken.de",
                          0);

    aboutData.addCredit ( "Janos Tamasi",
                          I18N_NOOP("Hungarian translations"),
                          "janusz at vnet.hu",
                          0);

    aboutData.addCredit ( "Jasper van der Marel",
                          I18N_NOOP("Dutch translations"),
                          "jasper dot van dot der dot marel at wanadoo.nl",
                          0);

    aboutData.addCredit ( "Anna Sawicka",
                          I18N_NOOP("Polish translations"),
                          "ania at kajak.org.pl",
                          0);

    aboutData.addCredit ( "Achim Bohnet",
                          I18N_NOOP("Bug reports and patches"),
                          "ach at mpe.mpg.de",
                          0);

    aboutData.addCredit ( "Charles Bouveyron",
                          I18N_NOOP("Beta tester"),
                          "c dot bouveyron at tuxfamily.org",
                          0);

    aboutData.addCredit ( "Richard Groult",
                          I18N_NOOP("Plugin contributor and beta tester"),
                          "Richard dot Groult at jalix.org",
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
                          "yves dot chaufour at wanadoo.fr",
                          0);

    aboutData.addCredit ( "Tom Albers",
                          I18N_NOOP("digiKam bugs.kde.org frontman, patches and feedback"),
                          "tomalbers at kde.nl",
                          0);
    
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); 

    KApplication app;

    bool detectCamera = false;
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (args && args->isSet("detect-camera"))
    {
        detectCamera = true;
    }
    
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
        DigikamFirstRun *firstRun = new DigikamFirstRun(config);
        app.setMainWidget(firstRun);
        firstRun->show();
        return app.exec();
    }

    // copy the db to a new temp file. we will use this copied db for testing
    // purposes in 0.8 development. just a safety precautions for developers
    // working on their main photo library

    if (!copyDBFile(albumPath))
    {
        KMessageBox::error(0, i18n("Failed to copy database file "
                                   "to temporary one."));
        return 0;
    }


    AlbumManager* man = new AlbumManager();
    man->setLibraryPath(albumPath);

    ScanLib sLib;
    if(config->readBoolEntry("Scan At Start", true))
        sLib.findMissingItems();
    sLib.updateItemsWithoutDate();

    // Register image formats (especially for TIFF )
    KImageIO::registerFormats();

    DigikamApp *digikam = new DigikamApp(detectCamera);

    app.setMainWidget(digikam);
    digikam->show();

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
