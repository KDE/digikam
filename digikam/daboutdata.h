/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam about data.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>

// KDE includes

#include <kurl.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>

namespace Digikam
{

static inline KLocalizedString digiKamSloganFormated()
{
    return ki18nc("This is the slogan formated string displayed in splashscreen. "
                  "Please translate using short words else the slogan can be truncated.",
                  "<qt><font color=\"white\">"
                  "<b>Manage</b> your <b>photographs</b> like <b>a professional</b> "
                  "with the power of <b>open source</b>."
                  "</font></qt>"
                 );
}

static inline KLocalizedString digiKamSlogan()
{
    return ki18n("Manage your photographs like a professional, "
                 "with the power of open source.");
}

static inline KLocalizedString copyright()
{
    return ki18n("(c) 2002-2009, digiKam developers team");
}

static inline KUrl webProjectUrl()
{
    return KUrl("http://www.digikam.org");
}

static inline void authorsRegistration(KAboutData& aboutData)
{
    aboutData.addAuthor ( ki18n("Caulier Gilles"),
                          ki18n("Main developer and coordinator"),
                          "caulier dot gilles at gmail dot com",
                          "http://www.digikam.org/?q=blog/3");

    aboutData.addAuthor ( ki18n("Marcel Wiesweg"),
                          ki18n("Developer"),
                          "marcel dot wiesweg at gmx dot de",
                          "http://www.digikam.org/?q=blog/8");

    aboutData.addAuthor ( ki18n("Andi Clemens"),
                          ki18n("Developer"),
                          "andi dot clemens at gmx dot net",
                          "http://www.digikam.org/?q=blog/135");

    aboutData.addAuthor ( ki18n("Matthias Welwarsky"),
                          ki18n("Developer"),
                          "matze at welwarsky dot de");

    aboutData.addAuthor ( ki18n("Julien Nardoux"),
                          ki18n("Developer"),
                          "Julien at narboux dot fr");

    aboutData.addAuthor ( ki18n("Julien Pontabry"),
                          ki18n("Developer"),
                          "julien dot pontabry at ulp dot u-strasbg dot fr");

    aboutData.addAuthor ( ki18n("Patrick Spendrin"),
                          ki18n("Developer and Windows port"),
                          "patrick_spendrin at gmx dot de",
                          "http://saroengels.blogspot.com");

    aboutData.addAuthor ( ki18n("Arnd Baecker"),
                          ki18n("Developer"),
                          "arnd dot baecker at web dot de",
                          "http://www.digikam.org/?q=blog/133");

    aboutData.addAuthor ( ki18n("Renchi Raju"),
                          ki18n("Developer (2002-2005)"),
                          "renchi at pooh dot tam dot uiuc dot edu");

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

    aboutData.addAuthor ( ki18n("Francisco J. Cruz"),
                          ki18n("Developer (2005-2006)"),
                          "fj dot cruz at supercable dot es",
                          "http://www.digikam.org/?q=blog/5");

    aboutData.addCredit ( ki18n("Risto Saukonpaa"),
                          ki18n("Design, icons, logo, banner, mockup, beta tester"),
                          "paristo at gmail dot com");

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
                          "gregory dot kokanosky at free dot fr");

    aboutData.addCredit ( ki18n("Gerhard Kulzer"),
                          ki18n("Handbook writer, alpha tester, webmaster"),
                          "gerhard at kulzer dot net");

    aboutData.addCredit ( ki18n("Oliver Doerr"),
                          ki18n("Beta tester"),
                          "oliver at doerr-privat dot de");

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
                          "aseigo at kde dot org");

    aboutData.addCredit ( ki18n("Yves Chaufour"),
                          ki18n("digiKam website, Feedback"),
                          "yves dot chaufour at wanadoo dot fr");

    aboutData.addCredit ( ki18n("Tung Nguyen"),
                          ki18n("Bug reports, feedback and icons"),
                          "ntung at free dot fr");
}

}  // namespace Digikam

#endif // DABOUT_DATA_H
