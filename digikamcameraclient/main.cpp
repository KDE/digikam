#include "digikamcameraclient.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("Digital camera interface for KDE");

static const char *version = "0.6.3 (using LibKIPI)";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KLocale::setMainCatalogue("digikam");
    
    KAboutData about ("digikamcameraclient",
                      I18N_NOOP("DigikamCameraClient"),
                      version,
		      description,
		      KAboutData::License_GPL,
                      "(C) 2002-2004, Digikam developers team",
		      0,
		      "http://digikam.sourceforge.net",
		      "digikam-users@list.sourceforge.net");

    about.addAuthor ( "Renchi Raju",
                      I18N_NOOP("coordinator and main developer"),
                      "renchi at pooh.tam.uiuc.edu",
                      "http://digikam.sourceforge.net");

    about.addAuthor ( "Caulier Gilles",
                      I18N_NOOP("Developer, translations coordinator, French translations"),
                      "caulier dot gilles at free.fr",
                      "http://caulier.gilles.free.fr");

    about.addCredit ( "Todd Shoemaker",
                      I18N_NOOP("Developer"),
                      "todd at theshoemakers.net",
                      0);

    about.addCredit ( "Gregory Kokanosky",
                      I18N_NOOP("Developer"),
                      "gregory.kokanosky@free.fr",
		      0);

    about.addCredit ( "Rune Laursen",
                      I18N_NOOP("Danish translations"),
                      "runerl at skjoldhoej.dk",
		      0);

    about.addCredit ( "Stefano Rivoir",
                      I18N_NOOP("Italian translations"),
                      "s.rivoir at gts.it",
                      0);

    about.addCredit ( "Jan Toenjes",
                      I18N_NOOP("German translations"),
                      "jan dot toenjes at web.de",
                      0);

    about.addCredit ( "Oliver Doerr",
                      I18N_NOOP("German translations and beta tester"),
                      "oliver at doerr-privat.de",
                      0);

    about.addCredit ( "Quique",
                      I18N_NOOP("Spanish translations"),
                      "quique at sindominio.net",
                      0);

    about.addCredit ( "Marcus Meissner",
                      I18N_NOOP("Czech translations"),
                      "marcus at jet.franken.de",
                      0);

    about.addCredit ( "Janos Tamasi",
                      I18N_NOOP("Hungarian translations"),
                      "janusz at vnet.hu",
                      0);

    about.addCredit ( "Jasper van der Marel",
                      I18N_NOOP("Dutch translations"),
                      "jasper dot van dot der dot marel at wanadoo.nl",
                      0);

    about.addCredit ( "Anna Sawicka",
                      I18N_NOOP("Polish translations"),
                      "ania at kajak.org.pl",
                      0);

    about.addCredit ( "Achim Bohnet",
                      I18N_NOOP("Bugs reports and patchs"),
                      "ach at mpe.mpg.de",
                      0);

    about.addCredit ( "Charles Bouveyron",
                      I18N_NOOP("Beta tester"),
                      "c dot bouveyron at tuxfamily.org",
                      0);

    about.addCredit ( "Richard Groult",
                      I18N_NOOP("Beta tester"),
                      "Richard dot Groult at jalix.org",
                      0);

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    // register ourselves as a dcop client
    app.dcopClient()->registerAs(app.name(), false);

    DigikamCameraClient *client = new DigikamCameraClient;

    return app.exec();
}
