/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam about data.
 *
 * Copyright (C) 2008-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "daboutdata.moc"

// Qt includes

#include <QString>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kicon.h>
#include <klocale.h>
#include <kxmlguiwindow.h>
#include <ktoolinvocation.h>

// Local includes

#include "componentsinfo.h"

namespace Digikam
{

DAboutData::DAboutData(KXmlGuiWindow* const parent)
    : QObject(parent)
{
}

DAboutData::~DAboutData()
{
}

void DAboutData::registerHelpActions()
{
    KXmlGuiWindow* const kwin          = dynamic_cast<KXmlGuiWindow*>(parent());

    KAction* const rawCameraListAction = new KAction(KIcon("kdcraw"), i18n("Supported RAW Cameras"), kwin);
    connect(rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    kwin->actionCollection()->addAction("help_rawcameralist", rawCameraListAction);

    KAction* const donateMoneyAction   = new KAction(KIcon("internet-web-browser"), i18n("Donate..."), kwin);
    connect(donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    kwin->actionCollection()->addAction("help_donatemoney", donateMoneyAction);

    KAction* const contributeAction    = new KAction(KIcon("internet-web-browser"), i18n("Contribute..."), kwin);
    connect(contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    kwin->actionCollection()->addAction("help_contribute", contributeAction);
}

void DAboutData::slotRawCameraList()
{
    showRawCameraList();
}

void DAboutData::slotDonateMoney()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=donation");
}

void DAboutData::slotContribute()
{
    KToolInvocation::invokeBrowser("http://www.digikam.org/?q=contrib");
}

KLocalizedString DAboutData::digiKamSloganFormated()
{
    return ki18nc("This is the slogan formated string displayed in splashscreen. "
                  "Please translate using short words else the slogan can be truncated.",
                  "<qt><font color=\"white\">"
                  "<b>Manage</b> your <b>photographs</b> like <b>a professional</b> "
                  "with the power of <b>open source</b>"
                  "</font></qt>"
                 );
}

KLocalizedString DAboutData::digiKamSlogan()
{
    return ki18n("Manage your photographs like a professional, "
                 "with the power of open source");
}

KLocalizedString DAboutData::copyright()
{
    return ki18n("(c) 2002-2015, digiKam developers team");
}

KUrl DAboutData::webProjectUrl()
{
    return KUrl("http://www.digikam.org");
}

void DAboutData::authorsRegistration(KAboutData& aboutData)
{
    // -- Core team --------------------------------------------------------------

    aboutData.addAuthor ( ki18n("Caulier Gilles"),
                          ki18n("Coordinator, Developer, and Mentoring"),
                          "caulier dot gilles at gmail dot com",
                          "https://plus.google.com/+GillesCaulier"
                        );

    aboutData.addAuthor ( ki18n("Marcel Wiesweg"),
                          ki18n("Developer and Mentoring"),
                          "marcel dot wiesweg at gmx dot de",
                          "https://www.facebook.com/marcel.wiesweg"
                        );

    aboutData.addAuthor ( ki18n("Teemu Rytilahti"),
                          ki18n("Developer"),
                          "tpr at iki dot fi",
                          "https://plus.google.com/u/0/105136119348505864693"
                        );

    aboutData.addAuthor ( ki18n("Michael G. Hansen"),
                          ki18n("Developer and Mentoring"),
                          "mike at mghansen dot de",
                          "http://www.mghansen.de"
                        );

    aboutData.addAuthor ( ki18n("Maik Qualmann"),
                          ki18n("Developer"),
                          "metzpinguin at gmail dot com",
                          "https://plus.google.com/u/0/107171232114475191915"
                        );

    // -- Contributors -----------------------------------------------------------

    aboutData.addAuthor ( ki18n("Matthias Welwarsky"),
                          ki18n("Developer"),
                          "matze at welwarsky dot de",
                          "https://plus.google.com/s/Matthias%20Welwarsky"
                        );

    aboutData.addAuthor ( ki18n("Julien Narboux"),
                          ki18n("Developer"),
                          "Julien at narboux dot fr",
                          "https://plus.google.com/+JulienNarboux"
                        );

    aboutData.addAuthor ( ki18n("Ananta Palani"),
                          ki18n("Windows Port and Release Manager"),
                          "anantapalani at gmail dot com",
                          "https://plus.google.com/u/0/+AnantaPalani"
                        );

    aboutData.addAuthor ( ki18n("Nicolas Lécureuil"),
                          ki18n("Releases Manager"),
                          "neoclust dot kde at gmail dot com",
                          "https://plus.google.com/u/0/111733995327568706391"
                        );

    // -- Students ---------------------------------------------------------------

    aboutData.addCredit ( ki18n("Veaceslav Munteanu"),
                          ki18n("Tags Manager"),
                          "veaceslav dot munteanu90 at gmail dot com",
                          "https://plus.google.com/114906808699351374523"
                        );

    aboutData.addCredit ( ki18n("Mohamed Anwer"),
                          ki18n("Model/View Port of Showfoto Thumbbar"),
                          "mohammed dot ahmed dot anwer at gmail dot com",
                          "https://plus.google.com/106020792892118847381"
                        );

    aboutData.addCredit ( ki18n("Yiou Wang"),
                          ki18n("Model/View Port of Image Editor Canvas"),
                          "geow812 at gmail dot com",
                          "https://plus.google.com/101883964009694930513"
                        );

    aboutData.addCredit ( ki18n("Gowtham Ashok"),
                          ki18n("Image Quality Sorter"),
                          "gwty93 at gmail dot com",
                          "https://plus.google.com/u/0/113235187016472722859"
                        );

    aboutData.addCredit ( ki18n("Aditya Bhatt"),
                          ki18n("Face Detection"),
                          "aditya at bhatts dot org",
                          "https://twitter.com/aditya_bhatt"
                        );

    aboutData.addCredit ( ki18n("Martin Klapetek"),
                          ki18n("Non-destructive image editing"),
                          "martin dot klapetek at gmail dot com",
                          "https://plus.google.com/u/0/101026761070865237619"
                        );

    aboutData.addCredit ( ki18n("Gabriel Voicu"),
                          ki18n("Reverse Geo-Coding"),
                          "ping dot gabi at gmail dot com",
                          "https://plus.google.com/u/0/101476692615103604273"
                        );

    aboutData.addCredit ( ki18n("Mahesh Hegde"),
                          ki18n("Face Recognition"),
                          "maheshmhegade at gmail dot com",
                          "https://plus.google.com/113704327590506304403"
                        );

    aboutData.addCredit ( ki18n("Pankaj Kumar"),
                          ki18n("Multi-core Support in Batch Queue Manager and Mentoring"),
                          "me at panks dot me",
                          "https://plus.google.com/114958890691877878308"
                        );

    aboutData.addCredit ( ki18n("Smit Mehta"),
                          ki18n("UPnP / DLNA Export tool and Mentoring"),
                          "smit dot tmeh at gmail dot com",
                          "https://plus.google.com/u/0/113404087048256151794"
                        );

    aboutData.addCredit ( ki18n("Islam Wazery"),
                          ki18n("Model/View port of Import Tool and Mentoring"),
                          "wazery at ubuntu dot com",
                          "https://plus.google.com/u/0/114444774108176364727"
                        );

    aboutData.addCredit ( ki18n("Abhinav Badola"),
                          ki18n("Video Metadata Support and Mentoring"),
                          "mail dot abu dot to at gmail dot com",
                          "https://plus.google.com/u/0/107198225472060439855"
                        );

    aboutData.addCredit ( ki18n("Benjamin Girault"),
                          ki18n("Panorama Tool and Mentoring"),
                          "benjamin dot girault at gmail dot com",
                          "https://plus.google.com/u/0/109282675370620103497"
                        );

    aboutData.addCredit ( ki18n("Victor Dodon"),
                          ki18n("XML based GUI port of Libkipi"),
                          "dodonvictor at gmail dot com",
                          "https://plus.google.com/u/0/107198225472060439855"
                        );

    aboutData.addCredit ( ki18n("Sayantan Datta"),
                          ki18n("Auto Noise Reduction"),
                          "sayantan dot knz at gmail dot com",
                          "https://plus.google.com/100302360459800439676"
                        );

    // -- Former contributors ----------------------------------------------------

    aboutData.addAuthor ( ki18n("Andi Clemens"),
                          ki18n("Advance Rename tool and developer"),
                          "andi dot clemens at gmail dot com",
                          "https://plus.google.com/110531606986594589135"
                        );

    aboutData.addAuthor ( ki18n("Patrick Spendrin"),
                          ki18n("Developer and Windows port"),
                          "patrick_spendrin at gmx dot de",
                          "https://plus.google.com/u/0/107813275713575797754"
                        );

    aboutData.addCredit ( ki18n("Francesco Riosa"),
                          ki18n("LCMS2 library port"),
                          "francesco plus kde at pnpitalia dot it",
                          "https://plus.google.com/u/0/113237307210359236747"
                        );

    aboutData.addCredit ( ki18n("Johannes Wienke"),
                          ki18n("Developer"),
                          "languitar at semipol dot de",
                          "https://www.facebook.com/languitar"
                        );

    aboutData.addAuthor ( ki18n("Julien Pontabry"),
                          ki18n("Developer"),
                          "julien dot pontabry at ulp dot u-strasbg dot fr",
                          "https://www.facebook.com/julien.pontabry"
                        );

    aboutData.addAuthor ( ki18n("Arnd Baecker"),
                          ki18n("Developer"),
                          "arnd dot baecker at web dot de"
                        );

    aboutData.addAuthor ( ki18n("Francisco J. Cruz"),
                          ki18n("Color Management"),
                          "fj dot cruz at supercable dot es",
                          "https://plus.google.com/u/0/+FranciscoJCruz"
                        );

    aboutData.addCredit ( ki18n("Pieter Edelman"),
                          ki18n("Developer"),
                          "p dot edelman at gmx dot net",
                          "https://www.facebook.com/pieter.edelman"
                        );

    aboutData.addCredit ( ki18n("Holger Foerster"),
                          ki18n("MySQL interface"),
                          "hamsi2k at freenet dot de"
                        );

    aboutData.addCredit ( ki18n("Risto Saukonpaa"),
                          ki18n("Design, icons, logo, banner, mockup, beta tester"),
                          "paristo at gmail dot com"
                        );

    aboutData.addCredit ( ki18n("Mikolaj Machowski"),
                          ki18n("Bug reports and patches"),
                          "mikmach at wp dot pl",
                          "https://www.facebook.com/mikolaj.machowski"
                        );

    aboutData.addCredit ( ki18n("Achim Bohnet"),
                          ki18n("Bug reports and patches"),
                          "ach at mpe dot mpg dot de",
                          "https://www.facebook.com/achim.bohnet"
                        );

    aboutData.addCredit ( ki18n("Luka Renko"),
                          ki18n("Developer"),
                          "lure at kubuntu dot org",
                          "https://www.facebook.com/luka.renko"
                        );

    aboutData.addCredit ( ki18n("Angelo Naselli"),
                          ki18n("Developer"),
                          "a dot naselli at libero dot it",
                          "https://plus.google.com/u/0/s/Angelo%20Naselli"
                        );

    aboutData.addCredit ( ki18n("Fabien Salvi"),
                          ki18n("Webmaster"),
                          "fabien dot ubuntu at gmail dot com"
                        );

    aboutData.addCredit ( ki18n("Todd Shoemaker"),
                          ki18n("Developer"),
                          "todd at theshoemakers dot net"
                        );

    aboutData.addCredit ( ki18n("Gerhard Kulzer"),
                          ki18n("Handbook writer, alpha tester, webmaster"),
                          "gerhard at kulzer dot net",
                          "https://plus.google.com/u/0/+GerhardKulzer"
                        );

    aboutData.addCredit ( ki18n("Oliver Doerr"),
                          ki18n("Beta tester"),
                          "oliver at doerr-privat dot de"
                        );

    aboutData.addCredit ( ki18n("Charles Bouveyron"),
                          ki18n("Beta tester"),
                          "c dot bouveyron at tuxfamily dot org"
                        );

    aboutData.addCredit ( ki18n("Richard Taylor"),
                          ki18n("Feedback and patches. Handbook writer"),
                          "rjt-digicam at thegrindstone dot me dot uk"
                        );

    aboutData.addCredit ( ki18n("Hans Karlsson"),
                          ki18n("digiKam website banner and application icons"),
                          "karlsson dot h at home dot se"
                        );

    aboutData.addCredit ( ki18n("Aaron Seigo"),
                          ki18n("Various usability fixes and general application polishing"),
                          "aseigo at kde dot org",
                          "https://plus.google.com/u/0/+AaronSeigo"
                        );

    aboutData.addCredit ( ki18n("Yves Chaufour"),
                          ki18n("digiKam website, Feedback"),
                          "yves dot chaufour at wanadoo dot fr"
                        );

    aboutData.addCredit ( ki18n("Tung Nguyen"),
                          ki18n("Bug reports, feedback and icons"),
                          "ntung at free dot fr"
                        );

    // -- Former Members ---------------------------------------------------------

    aboutData.addAuthor ( ki18n("Renchi Raju"),
                          ki18n("Developer (2002-2005)"),
                          "renchi dot raju at gmail dot com"
                          "https://www.facebook.com/renchi.raju"
                        );

    aboutData.addAuthor ( ki18n("Joern Ahrens"),
                          ki18n("Developer (2004-2005)"),
                          "kde at jokele dot de",
                          "http://www.jokele.de/"
                        );

    aboutData.addAuthor ( ki18n("Tom Albers"),
                          ki18n("Developer (2004-2005)"),
                          "tomalbers at kde dot nl",
                          "https://plus.google.com/u/0/+TomAlbers"
                        );

    aboutData.addAuthor ( ki18n("Ralf Holzer"),
                          ki18n("Developer (2004)"),
                          "kde at ralfhoelzer dot com"
                        );
}

}  // namespace Digikam
