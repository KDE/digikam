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

#include "daboutdata.h"

// Qt includes

#include <QString>
#include <QIcon>
#include <QDesktopServices>
#include <QAction>

// KDE includes

#include <kactioncollection.h>
#include <kxmlguiwindow.h>
#include <klocalizedstring.h>

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

    QAction * const rawCameraListAction = new QAction(QIcon::fromTheme("kdcraw"), i18n("Supported RAW Cameras"), kwin);
    connect(rawCameraListAction, SIGNAL(triggered()), this, SLOT(slotRawCameraList()));
    kwin->actionCollection()->addAction("help_rawcameralist", rawCameraListAction);

    QAction * const donateMoneyAction   = new QAction(QIcon::fromTheme("internet-web-browser"), i18n("Donate..."), kwin);
    connect(donateMoneyAction, SIGNAL(triggered()), this, SLOT(slotDonateMoney()));
    kwin->actionCollection()->addAction("help_donatemoney", donateMoneyAction);

    QAction * const contributeAction    = new QAction(QIcon::fromTheme("internet-web-browser"), i18n("Contribute..."), kwin);
    connect(contributeAction, SIGNAL(triggered()), this, SLOT(slotContribute()));
    kwin->actionCollection()->addAction("help_contribute", contributeAction);
}

void DAboutData::slotRawCameraList()
{
    showRawCameraList();
}

void DAboutData::slotDonateMoney()
{
    QDesktopServices::openUrl(QUrl("http://www.digikam.org/?q=donation"));
}

void DAboutData::slotContribute()
{
    QDesktopServices::openUrl(QUrl("http://www.digikam.org/?q=contrib"));
}

const QString DAboutData::digiKamSloganFormated()
{
    return i18nc("This is the slogan formated string displayed in splashscreen. "
                  "Please translate using short words else the slogan can be truncated.",
                  "<qt><font color=\"white\">"
                  "<b>Manage</b> your <b>photographs</b> like <b>a professional</b> "
                  "with the power of <b>open source</b>"
                  "</font></qt>"
                 );
}

const QString DAboutData::digiKamSlogan()
{
    return i18n("Manage your photographs like a professional, "
                "with the power of open source");
}

const QString DAboutData::copyright()
{
    return i18n("(c) 2002-2015, digiKam developers team");
}

const QUrl DAboutData::webProjectUrl()
{
    return QUrl("http://www.digikam.org");
}

void DAboutData::authorsRegistration(KAboutData& aboutData)
{
    // -- Core team --------------------------------------------------------------

    aboutData.addAuthor ( ki18n("Caulier Gilles").toString(),
                          ki18n("Coordinator, Developer, and Mentoring").toString(),
                          "caulier dot gilles at gmail dot com",
                          "https://plus.google.com/+GillesCaulier"
                        );

    aboutData.addAuthor ( ki18n("Marcel Wiesweg").toString(),
                          ki18n("Developer and Mentoring").toString(),
                          "marcel dot wiesweg at gmx dot de",
                          "https://www.facebook.com/marcel.wiesweg"
                        );

    aboutData.addAuthor ( ki18n("Michael G. Hansen").toString(),
                          ki18n("Developer and Mentoring").toString(),
                          "mike at mghansen dot de",
                          "http://www.mghansen.de"
                        );

    aboutData.addAuthor ( ki18n("Andi Clemens").toString(),
                          ki18n("Developer").toString(),
                          "andi dot clemens at gmail dot com",
                          "https://plus.google.com/110531606986594589135"
                        );

    // -- Contributors -----------------------------------------------------------

    aboutData.addAuthor ( ki18n("Teemu Rytilahti").toString(),
                          ki18n("Developer").toString(),
                          "tpr at iki dot fi",
                          "https://plus.google.com/u/0/105136119348505864693"
                        );

    aboutData.addAuthor ( ki18n("Matthias Welwarsky").toString(),
                          ki18n("Developer").toString(),
                          "matze at welwarsky dot de",
                          "https://plus.google.com/s/Matthias%20Welwarsky"
                        );

    aboutData.addAuthor ( ki18n("Julien Narboux").toString(),
                          ki18n("Developer").toString(),
                          "Julien at narboux dot fr",
                          "https://plus.google.com/+JulienNarboux"
                        );

    aboutData.addAuthor ( ki18n("Ananta Palani").toString(),
                          ki18n("Windows Port and Release Manager").toString(),
                          "anantapalani at gmail dot com",
                          "https://plus.google.com/u/0/+AnantaPalani"
                        );

    aboutData.addAuthor ( ki18n("Nicolas Lécureuil").toString(),
                          ki18n("Releases Manager").toString(),
                          "neoclust dot kde at gmail dot com",
                          "https://plus.google.com/u/0/111733995327568706391"
                        );

    // -- Students ---------------------------------------------------------------

    aboutData.addCredit ( ki18n("Veaceslav Munteanu").toString(),
                          ki18n("Tags Manager").toString(),
                          "veaceslav dot munteanu90 at gmail dot com",
                          "https://plus.google.com/114906808699351374523"
                        );

    aboutData.addCredit ( ki18n("Mohamed Anwer").toString(),
                          ki18n("Model/View Port of Showfoto Thumbbar").toString(),
                          "mohammed dot ahmed dot anwer at gmail dot com",
                          "https://plus.google.com/106020792892118847381"
                        );

    aboutData.addCredit ( ki18n("Yiou Wang").toString(),
                          ki18n("Model/View Port of Image Editor Canvas").toString(),
                          "geow812 at gmail dot com",
                          "https://plus.google.com/101883964009694930513"
                        );

    aboutData.addCredit ( ki18n("Gowtham Ashok").toString(),
                          ki18n("Image Quality Sorter").toString(),
                          "gwty93 at gmail dot com",
                          "https://plus.google.com/u/0/113235187016472722859"
                        );

    aboutData.addCredit ( ki18n("Aditya Bhatt").toString(),
                          ki18n("Face Detection").toString(),
                          "aditya at bhatts dot org",
                          "https://twitter.com/aditya_bhatt"
                        );

    aboutData.addCredit ( ki18n("Martin Klapetek").toString(),
                          ki18n("Non-destructive image editing").toString(),
                          "martin dot klapetek at gmail dot com",
                          "https://plus.google.com/u/0/101026761070865237619"
                        );

    aboutData.addCredit ( ki18n("Gabriel Voicu").toString(),
                          ki18n("Reverse Geo-Coding").toString(),
                          "ping dot gabi at gmail dot com",
                          "https://plus.google.com/u/0/101476692615103604273"
                        );

    aboutData.addCredit ( ki18n("Mahesh Hegde").toString(),
                          ki18n("Face Recognition").toString(),
                          "maheshmhegade at gmail dot com",
                          "https://plus.google.com/113704327590506304403"
                        );

    aboutData.addCredit ( ki18n("Pankaj Kumar").toString(),
                          ki18n("Multi-core Support in Batch Queue Manager and Mentoring").toString(),
                          "me at panks dot me",
                          "https://plus.google.com/114958890691877878308"
                        );

    aboutData.addCredit ( ki18n("Smit Mehta").toString(),
                          ki18n("UPnP / DLNA Export tool and Mentoring").toString(),
                          "smit dot tmeh at gmail dot com",
                          "https://plus.google.com/u/0/113404087048256151794"
                        );

    aboutData.addCredit ( ki18n("Islam Wazery").toString(),
                          ki18n("Model/View port of Import Tool and Mentoring").toString(),
                          "wazery at ubuntu dot com",
                          "https://plus.google.com/u/0/114444774108176364727"
                        );

    aboutData.addCredit ( ki18n("Abhinav Badola").toString(),
                          ki18n("Video Metadata Support and Mentoring").toString(),
                          "mail dot abu dot to at gmail dot com",
                          "https://plus.google.com/u/0/107198225472060439855"
                        );

    aboutData.addCredit ( ki18n("Benjamin Girault").toString(),
                          ki18n("Panorama Tool and Mentoring").toString(),
                          "benjamin dot girault at gmail dot com",
                          "https://plus.google.com/u/0/109282675370620103497"
                        );

    aboutData.addCredit ( ki18n("Victor Dodon").toString(),
                          ki18n("XML based GUI port of Libkipi").toString(),
                          "dodonvictor at gmail dot com",
                          "https://plus.google.com/u/0/107198225472060439855"
                        );

    aboutData.addCredit ( ki18n("Sayantan Datta").toString(),
                          ki18n("Auto Noise Reduction").toString(),
                          "sayantan dot knz at gmail dot com",
                          "https://plus.google.com/100302360459800439676"
                        );

    // -- Former contributors ----------------------------------------------------

    aboutData.addAuthor ( ki18n("Patrick Spendrin").toString(),
                          ki18n("Developer and Windows port").toString(),
                          "patrick_spendrin at gmx dot de",
                          "https://plus.google.com/u/0/107813275713575797754"
                        );

    aboutData.addCredit ( ki18n("Francesco Riosa").toString(),
                          ki18n("LCMS2 library port").toString(),
                          "francesco plus kde at pnpitalia dot it",
                          "https://plus.google.com/u/0/113237307210359236747"
                        );

    aboutData.addCredit ( ki18n("Johannes Wienke").toString(),
                          ki18n("Developer").toString(),
                          "languitar at semipol dot de",
                          "https://www.facebook.com/languitar"
                        );

    aboutData.addAuthor ( ki18n("Julien Pontabry").toString(),
                          ki18n("Developer").toString(),
                          "julien dot pontabry at ulp dot u-strasbg dot fr",
                          "https://www.facebook.com/julien.pontabry"
                        );

    aboutData.addAuthor ( ki18n("Arnd Baecker").toString(),
                          ki18n("Developer").toString(),
                          "arnd dot baecker at web dot de"
                        );

    aboutData.addAuthor ( ki18n("Francisco J. Cruz").toString(),
                          ki18n("Color Management").toString(),
                          "fj dot cruz at supercable dot es",
                          "https://plus.google.com/u/0/+FranciscoJCruz"
                        );

    aboutData.addCredit ( ki18n("Pieter Edelman").toString(),
                          ki18n("Developer").toString(),
                          "p dot edelman at gmx dot net",
                          "https://www.facebook.com/pieter.edelman"
                        );

    aboutData.addCredit ( ki18n("Holger Foerster").toString(),
                          ki18n("MySQL interface").toString(),
                          "hamsi2k at freenet dot de"
                        );

    aboutData.addCredit ( ki18n("Risto Saukonpaa").toString(),
                          ki18n("Design, icons, logo, banner, mockup, beta tester").toString(),
                          "paristo at gmail dot com"
                        );

    aboutData.addCredit ( ki18n("Mikolaj Machowski").toString(),
                          ki18n("Bug reports and patches").toString(),
                          "mikmach at wp dot pl",
                          "https://www.facebook.com/mikolaj.machowski"
                        );

    aboutData.addCredit ( ki18n("Achim Bohnet").toString(),
                          ki18n("Bug reports and patches").toString(),
                          "ach at mpe dot mpg dot de",
                          "https://www.facebook.com/achim.bohnet"
                        );

    aboutData.addCredit ( ki18n("Luka Renko").toString(),
                          ki18n("Developer").toString(),
                          "lure at kubuntu dot org",
                          "https://www.facebook.com/luka.renko"
                        );

    aboutData.addCredit ( ki18n("Angelo Naselli").toString(),
                          ki18n("Developer").toString(),
                          "a dot naselli at libero dot it",
                          "https://plus.google.com/u/0/s/Angelo%20Naselli"
                        );

    aboutData.addCredit ( ki18n("Fabien Salvi").toString(),
                          ki18n("Webmaster").toString(),
                          "fabien dot ubuntu at gmail dot com"
                        );

    aboutData.addCredit ( ki18n("Todd Shoemaker").toString(),
                          ki18n("Developer").toString(),
                          "todd at theshoemakers dot net"
                        );

    aboutData.addCredit ( ki18n("Gerhard Kulzer").toString(),
                          ki18n("Handbook writer, alpha tester, webmaster").toString(),
                          "gerhard at kulzer dot net",
                          "https://plus.google.com/u/0/+GerhardKulzer"
                        );

    aboutData.addCredit ( ki18n("Oliver Doerr").toString(),
                          ki18n("Beta tester").toString(),
                          "oliver at doerr-privat dot de"
                        );

    aboutData.addCredit ( ki18n("Charles Bouveyron").toString(),
                          ki18n("Beta tester").toString(),
                          "c dot bouveyron at tuxfamily dot org"
                        );

    aboutData.addCredit ( ki18n("Richard Taylor").toString(),
                          ki18n("Feedback and patches. Handbook writer").toString(),
                          "rjt-digicam at thegrindstone dot me dot uk"
                        );

    aboutData.addCredit ( ki18n("Hans Karlsson").toString(),
                          ki18n("digiKam website banner and application icons").toString(),
                          "karlsson dot h at home dot se"
                        );

    aboutData.addCredit ( ki18n("Aaron Seigo").toString(),
                          ki18n("Various usability fixes and general application polishing").toString(),
                          "aseigo at kde dot org",
                          "https://plus.google.com/u/0/+AaronSeigo"
                        );

    aboutData.addCredit ( ki18n("Yves Chaufour").toString(),
                          ki18n("digiKam website, Feedback").toString(),
                          "yves dot chaufour at wanadoo dot fr"
                        );

    aboutData.addCredit ( ki18n("Tung Nguyen").toString(),
                          ki18n("Bug reports, feedback and icons").toString(),
                          "ntung at free dot fr"
                        );

    // -- Former Members ---------------------------------------------------------

    aboutData.addAuthor ( ki18n("Renchi Raju").toString(),
                          ki18n("Developer (2002-2005)").toString(),
                          "renchi dot raju at gmail dot com"
                          "https://www.facebook.com/renchi.raju"
                        );

    aboutData.addAuthor ( ki18n("Joern Ahrens").toString(),
                          ki18n("Developer (2004-2005)").toString(),
                          "kde at jokele dot de",
                          "http://www.jokele.de/"
                        );

    aboutData.addAuthor ( ki18n("Tom Albers").toString(),
                          ki18n("Developer (2004-2005)").toString(),
                          "tomalbers at kde dot nl",
                          "https://plus.google.com/u/0/+TomAlbers"
                        );

    aboutData.addAuthor ( ki18n("Ralf Holzer").toString(),
                          ki18n("Developer (2004)").toString(),
                          "kde at ralfhoelzer dot com"
                        );
}

}  // namespace Digikam
