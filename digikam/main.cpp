//////////////////////////////////////////////////////////////////////////////
//
//    MAIN.CPP
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes.

#include <qstringlist.h>
#include <qfileinfo.h>

// KDE includes
 
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kimageio.h>
#include <ktip.h>
#include <dcopclient.h>

// Local includes.

#include "splashscreen.h"
#include "digikamapp.h"
#include "digikamfirstrun.h"

static const char *description = I18N_NOOP("A Photo-Management Application for KDE");

static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

    KAboutData aboutData( "digikam", 
                          I18N_NOOP("Digikam"),
                          "0.6.3",
                          description,
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2002-2004, Digikam developers team"),
                          0,
                          "http://digikam.sourceforge.net",
                          "digikam-users@list.sourceforge.net");

    aboutData.addAuthor ( "Renchi Raju",
                          I18N_NOOP("Main coordinator and developer"),
                          "renchi at pooh.tam.uiuc.edu",
                          "http://digikam.sourceforge.net");

    aboutData.addAuthor ( "Caulier Gilles",
                          I18N_NOOP("Developer, co-coordinator, French translations"),
                          "caulier dot gilles at free.fr",
                          "http://caulier.gilles.free.fr");

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
                          I18N_NOOP("Bugs reports and patchs"),
                          "ach at mpe.mpg.de",
                          0);

    aboutData.addCredit ( "Charles Bouveyron",
                          I18N_NOOP("Beta tester"),
                          "c dot bouveyron at tuxfamily.org",
                          0);

    aboutData.addCredit ( "Richard Groult",
                          I18N_NOOP("Plugins contributor and beta tester"),
                          "Richard dot Groult at jalix.org",
                          0);

    aboutData.addCredit ( "Ralf Holzer",
                          I18N_NOOP("Developer"),
                          "ralf at well.com",
                          0);
                                                    
    aboutData.addCredit ( "Richard Taylor",
                          I18N_NOOP("Feedback and patchs"),
                          "r dot taylor at bcs.org.uk",
                          0);
                          
    aboutData.setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"),
                            I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;

    // get our DCOP client and attach so that we may use it
    DCOPClient *client = app.dcopClient();
    client->attach();
    app.dcopClient()->registerAs(app.name(), false);

    double currentVersion = 0.6;

    KConfig* config = KGlobal::config();
    config->setGroup("General Settings");
    double version = config->readDoubleNumEntry("Version");

    config->setGroup("Album Settings");
    QString albumPath = config->readPathEntry("Album Path");
    QFileInfo dirInfo(albumPath);

    if (version < currentVersion || !dirInfo.exists()
        || !dirInfo.isDir())
    {
        // Run the first run
        DigikamFirstRun *firstRun = new DigikamFirstRun(config);
        app.setMainWidget(firstRun);
        firstRun->show();
        return app.exec();
    }
    
    // Register image formats (especially for TIFF )
    KImageIO::registerFormats();

    SplashScreen *splash = new SplashScreen();

    DigikamApp *digikam = new DigikamApp();

    app.setMainWidget(digikam);
    digikam->show();

    splash->finish( digikam );

    KTipDialog::showTip("digikam/tips");

    return app.exec();
}
