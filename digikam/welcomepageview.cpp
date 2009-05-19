/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "welcomepageview.h"
#include "welcomepageview.moc"

// Qt includes

#include <QTimer>
#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <klocale.h>
#include <kshortcut.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <ktoolinvocation.h>
#include <kurl.h>

// Local includes

#include "daboutdata.h"
#include "version.h"
#include "themeengine.h"

namespace Digikam
{

WelcomePageView::WelcomePageView(QWidget* parent)
               : KHTMLPart(parent)
{
    m_infoPageCssFile = 0;

    widget()->setFocusPolicy(Qt::WheelFocus);
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    setPluginsEnabled(false);
    setJScriptEnabled(false); // just make this explicit.
    setJavaEnabled(false);    // just make this explicit.
    setMetaRefreshEnabled(false);
    setURLCursor(Qt::PointingHandCursor);
    view()->adjustSize();

    // Disable some KHTMLPart actions as they break predefined digiKam actions.
    // We can re-assign the disabled actions later if we ever need to.
    disablePredefinedActions();

    // ------------------------------------------------------------

    connect(browserExtension(), SIGNAL(openUrlRequest(const KUrl&, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&)),
            this, SLOT(slotUrlOpen(const KUrl&)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    QTimer::singleShot(0, this, SLOT(slotThemeChanged()));
}

WelcomePageView::~WelcomePageView()
{
    if (m_infoPageCssFile)
        m_infoPageCssFile->remove();
    delete m_infoPageCssFile;
}

void WelcomePageView::slotUrlOpen(const KUrl& url)
{
   KToolInvocation::invokeBrowser(url.url());
}

QString WelcomePageView::infoPage()
{
    QStringList newFeatures;
    newFeatures << i18n("Designed from the ground-up for KDE4, using KDE4 technology:"
                        "<ul><li>Hardware handling with KDE4's Solid interface;</li>"
                        "<li>More comprehensive multimedia file handling using KDE4's Phonon interface;</li>"
                        "<li>Easy Geolocation with KDE4's Marble interface.</li></ul>");
    newFeatures << i18n("XMP metadata support;");
    newFeatures << i18n("TIFF/EP RAW metadata editing;");
    newFeatures << i18n("Customizable file storage for the digiKam database, supporting remote albums;");
    newFeatures << i18n("Support of multiple root album paths (no more importing into one giant album);");
    newFeatures << i18n("Thumbnail-bar integration for easy navigation and editing;");
    newFeatures << i18n("Supports the latest camera RAW files;");
    newFeatures << i18n("New/revamped tools:"
                        "<ul><li>Batch queue manager;</li>"
                        "<li>Revamped camera import wizard;</li>"
                        "<li>LensFun integration: auto-correction of lens distortion;</li>"
                        "<li>Fuzzy searches based on hand-drawn sketches;</li>"
                        "<li>Advanced searches using image meta-information, such as keywords and dates;</li>"
                        "<li>User-friendly map searching that gives you the power to search for global photo locations;</li>"
                        "<li>Advanced searches for duplicate and similar images;</li></ul>");
    newFeatures << i18n("...and much more.");


    QString featureItems;
    for (int i = 0 ; i < newFeatures.count() ; ++i)
        featureItems += i18n("<li>%1</li>\n", newFeatures[i] );

    QString info =
        i18nc(
        "%1: current digiKam version; "
        "%2: digiKam help:// Url; "
        "%3: digiKam homepage Url; "
        "%4: prior digiKam version; "
        "%5: prior KDE version; "
        "%6: generated list of new features; "
        "%7: generated list of important changes; "
        "--- end of comment ---",

        "<h2 style='margin-top: 0px;'>"
        "Welcome to digiKam %1"
        "</h2>"

        "<p>digiKam is an open source photo management program "
        "designed to import, organize, enhance, search and export your "
        "digital images to and from your computer.</p>"

        "<p>Currently, you are in the Album view mode of digiKam. Albums are the places "
        "where your files are stored, and are identical to the folders "
        "on your hard disk.</p>"

        "<p>"
            "<ul>"
                "<li>"
                    "digiKam has many powerful features which are described in the "
                    "<a href=\"%2\">documentation</a>"
                "</li>"
                "<li>"
                    "The <a href=\"%3\">digiKam homepage</A> provides information about "
                    "new versions of digiKam."
                "</li>"
            "</ul>"
        "</p>"

        "<p>%7</p>"

        "<p>Some of the new features in this release of digiKam include "
            "(compared to digiKam %4):</p>"

        "<p>"
            "<ul>%5</ul>"
        "</p>"

        "<p>%6</p>"

        "<p>We hope that you will enjoy digiKam.</p>"

        "<p>Thank you,</p>"

        "<p style='margin-bottom: 0px; margin-left:20px;'>The digiKam team</p>",

    QString(digikam_version),            // %1 : current digiKam version
    "help:/digikam/index.html",          // %2 : digiKam help:// Url
    webProjectUrl().url(),               // %3 : digiKam homepage Url
    "0.9.4",                             // %4 : prior digiKam version
    featureItems,                        // %5 : prior KDE version
    QString(),                           // %6 : generated list of new features
    QString());                          // %7 : previous digiKam release.

    return info;
}

QByteArray WelcomePageView::fileToString(const QString& aFileName)
{
    QByteArray   result;
    QFileInfo    info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile        file(aFileName);

    if (aFileName.isEmpty() || len <= 0 ||
        !info.exists() || info.isDir() || !info.isReadable() ||
        !file.open(QIODevice::Unbuffered|QIODevice::ReadOnly))
        return QByteArray();

    result.resize(len + 2);
    readLen = file.read(result.data(), len);
    if (1 && result[len-1]!='\n')
    {
        result[len++] = '\n';
        ++readLen;
    }
    result[len] = '\0';

    if (readLen < len)
        return QByteArray();

    return result;
}

void WelcomePageView::updateInfoPageCss()
{
    QColor background = ThemeEngine::instance()->baseColor();
    QColor text       = ThemeEngine::instance()->textRegColor();
    QColor highlight  = ThemeEngine::instance()->textSpecialRegColor();

    QString infoPageCss  = fileToString(KStandardDirs::locate("data", "digikam/about/infopage.css"));
    infoPageCss          = infoPageCss
                           .arg(background.name())                                                     // %1
                           .arg(text.name())                                                           // %2
                           .arg(highlight.name())                                                      // %3
                           .arg(highlight.name())                                                      // %4
                           .arg(highlight.name())                                                      // %5
                           .arg(background.name())                                                     // %6
                           .arg(background.name())                                                     // %7
                           .arg(KStandardDirs::locate("data", "digikam/about/top-middle.png"))         // %8
                           .arg(KStandardDirs::locate("data", "digikam/about/top-left-digikam.png"))   // %9
                           .arg(KStandardDirs::locate("data", "digikam/about/box-top-left.png"))       // %10
                           .arg(KStandardDirs::locate("data", "digikam/about/box-top-right.png"))      // %11
                           .arg(KStandardDirs::locate("data", "digikam/about/box-top-middle.png"))     // %12
                           .arg(KStandardDirs::locate("data", "digikam/about/box-middle-left.png"))    // %13
                           .arg(background.name())                                                     // %14
                           .arg(KStandardDirs::locate("data", "digikam/about/box-middle-right.png"))   // %15
                           .arg(KStandardDirs::locate("data", "digikam/about/box-bottom-left.png"))    // %16
                           .arg(KStandardDirs::locate("data", "digikam/about/box-bottom-right.png"))   // %17
                           .arg(KStandardDirs::locate("data", "digikam/about/box-bottom-middle.png")); // %18

    m_infoPageCssFile->open();
    QFile file(m_infoPageCssFile->fileName());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << infoPageCss;
    file.close();
}

void WelcomePageView::slotThemeChanged()
{
    if (m_infoPageCssFile)
        m_infoPageCssFile->remove();
    delete m_infoPageCssFile;

    m_infoPageCssFile = new KTemporaryFile;
    m_infoPageCssFile->setSuffix(".css");
    m_infoPageCssFile->setAutoRemove(false);

    updateInfoPageCss();

    QString fontSize         = QString::number(12);
    QString appTitle         = i18n("digiKam");
    QString slogan           = digiKamSlogan().toString();
    QString locationHtml     = KStandardDirs::locate("data", "digikam/about/main.html");
    QString locationRtl      = KStandardDirs::locate("data", "digikam/about/infopage_rtl.css" );
    QString rtl              = kapp->isRightToLeft() ? QString("@import \"%1\";" ).arg(locationRtl)
                                                     : QString();
    QString locationCss      = m_infoPageCssFile->fileName();

    begin(KUrl(locationHtml));

    QString content = fileToString(locationHtml);
    content         = content.arg(locationCss)        // %1
                             .arg(rtl)                // %2
                             .arg(fontSize)           // %3
                             .arg(appTitle)           // %4
                             .arg(slogan)             // %5
                             .arg(infoPage());        // %6

    write(content);
    end();
    show();
}

void WelcomePageView::disablePredefinedActions()
{
    KAction* findAction = qobject_cast<KAction*>(actionCollection()->action("find"));
    if (findAction)
        findAction->setShortcut(KShortcut());
    else
        kDebug(50003) << "failed to remove the shortcut of khtml's find action" << endl;

    KAction* findNextAction = qobject_cast<KAction*>(actionCollection()->action("findNext"));
    if (findNextAction)
        findNextAction->setShortcut(KShortcut());
    else
        kDebug(50003) << "failed to remove the shortcut of khtml's findNext action" << endl;

    KAction* findPreviousAction = qobject_cast<KAction*>(actionCollection()->action("findPrevious"));
    if (findPreviousAction)
        findPreviousAction->setShortcut(KShortcut());
    else
        kDebug(50003) << "failed to remove the shortcut of khtml's findPrevious action" << endl;

    KAction* selectAllAction = qobject_cast<KAction*>(actionCollection()->action("selectAll"));
    if (selectAllAction)
        selectAllAction->setShortcut(KShortcut());
    else
        kDebug(50003) << "failed to remove the shortcut of khtml's selectAll action" << endl;
}

}  // namespace Digikam
