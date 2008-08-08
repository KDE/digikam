/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-08
 * Description : digiKam about data.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef DABOUT_DATA_H
#define DABOUT_DATA_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>


static const char digikam_version[]     = "0.9.5-svn";
static const char showfoto_version[]    = "0.9.0-svn";


namespace Digikam
{
static inline const char* digiKamDescription()
{
    return I18N_NOOP("A Photo-Management Application for KDE");
}

static inline const char* showFotoDescription()
{
    return I18N_NOOP("KDE Photo Viewer and Editor");
}

static inline const char* themeDesignerDescription()
{
    return I18N_NOOP("A Color Theme Designer for digiKam");
}

static inline const char* copyright()
{
    return I18N_NOOP("(c) 2002-2008, digiKam developers team");
}

static inline const char* webProjectUrl()
{
    return "http://www.digikam.org";
}

static inline void authorsRegistration(KAboutData& aboutData)
{
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

    aboutData.addAuthor ( "Andi Clemens",
            I18N_NOOP("Developer"),
            "andi dot clemens at gmx dot net",
            "http://www.digikam.org/?q=blog/135");

    aboutData.addAuthor ( "Francisco J. Cruz",
            I18N_NOOP("Developer"),
            "fj dot cruz at supercable dot es",
            "http://www.digikam.org/?q=blog/5");

    aboutData.addAuthor ( "Renchi Raju",
            I18N_NOOP("Developer (2002-2005)"),
            "renchi at pooh dot tam dot uiuc dot edu",
            0);

    aboutData.addAuthor ( "Joern Ahrens",
            I18N_NOOP("Developer (2004-2005)"),
            "kde at jokele dot de",
            "http://www.digikam.org/?q=blog/1");

    aboutData.addAuthor ( "Tom Albers",
            I18N_NOOP("Developer (2004-2005)"),
            "tomalbers at kde dot nl",
            "http://www.omat.nl/drupal/?q=blog/1");

    aboutData.addAuthor ( "Ralf Holzer (2004)",
            I18N_NOOP("Developer"),
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

    aboutData.addCredit ( "Todd Shoemaker",
            I18N_NOOP("Developer"),
            "todd at theshoemakers dot net",
            0);

    aboutData.addCredit ( "Gregory Kokanosky",
            I18N_NOOP("Developer"),
            "gregory dot kokanosky at free dot fr",
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
            "aseigo at kde dot org",
            0);

    aboutData.addCredit ( "Yves Chaufour",
            I18N_NOOP("digiKam website, Feedback"),
            "yves dot chaufour at wanadoo dot fr",
            0);

    aboutData.addCredit ( "Tung Nguyen",
            I18N_NOOP("Bug reports, feedback and icons"),
            "ntung at free dot fr",
            0);
}

}  // namespace Digikam

#endif // DABOUT_DATA_H
