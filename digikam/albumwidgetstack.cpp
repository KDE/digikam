/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright 2006 by Gilles Caulier
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
 
#include <qvbox.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <kstandarddirs.h>
#include <klocale.h>
#include <kcursor.h>
#include <kdialogbase.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kapplication.h>

// Local includes.

#include "version.h"
#include "themeengine.h"
#include "imagepreviewwidget.h"
#include "albumiconview.h"
#include "albumwidgetstack.h"
#include "albumwidgetstack.moc"

namespace Digikam
{

class AlbumWidgetStackPriv
{

public:

    AlbumWidgetStackPriv()
    {
        buttonsArea        = 0;
        previewItemWidget  = 0;
        previewAlbumWidget = 0;
        backButton         = 0;
        editButton         = 0;
        htmlView           = 0;
    }

    QPushButton        *backButton;
    QPushButton        *editButton;

    QWidget            *buttonsArea;

    KHTMLPart          *htmlView;

    ImagePreviewWidget *previewItemWidget;

    AlbumIconView      *previewAlbumWidget;
};

AlbumWidgetStack::AlbumWidgetStack(QWidget *parent)
                : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new AlbumWidgetStackPriv;

    // -- Album icon view -----------------------------------------------
    d->previewAlbumWidget = new AlbumIconView(this);

    // -- Picture preview -----------------------------------------------

    QVBox *previewArea   = new QVBox(this);
    d->previewItemWidget = new ImagePreviewWidget(previewArea);
    d->buttonsArea       = new QWidget(previewArea);
    QHBoxLayout *hlay    = new QHBoxLayout(d->buttonsArea);
    d->backButton        = new QPushButton(i18n("Back to Album"), d->buttonsArea);
    d->editButton        = new QPushButton(i18n("Edit..."), d->buttonsArea);
    previewArea->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    previewArea->setMargin(0);
    previewArea->setLineWidth(1);

    hlay->setMargin(KDialogBase::marginHint());
    hlay->addStretch(1);
    hlay->addWidget(d->backButton);
    hlay->addStretch(10);
    hlay->addWidget(d->editButton);
    hlay->addStretch(1);

    // -- HTML help view -----------------------------------------------

    d->htmlView = new KHTMLPart(this);
    d->htmlView->widget()->setFocusPolicy(WheelFocus);
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    d->htmlView->setPluginsEnabled(false);
    d->htmlView->setJScriptEnabled(false); // just make this explicit.
    d->htmlView->setJavaEnabled(false);    // just make this explicit.
    d->htmlView->setMetaRefreshEnabled(false);
    d->htmlView->setURLCursor(KCursor::handCursor());

    QString location = locate("data", "digikam/about/main.html");
    QString content  = fileToString(location);
    content = content.arg( locate( "data", "digikam/about/kde_infopage.css" ) );
    content = content.arg( "" );
    
    d->htmlView->begin(KURL( location ));
    
    QString fontSize = QString::number( 12 );
    QString appTitle = i18n("digiKam");
    QString catchPhrase = "";
    QString quickDescription = i18n("A Photo-Management Application for KDE");
    d->htmlView->write(content.arg(fontSize)
                              .arg(appTitle)
                              .arg(catchPhrase)
                              .arg(quickDescription)
                              .arg(infoPage()));
    d->htmlView->end();
    d->htmlView->show();

    // -- Stack widgets ------------------------------------------------

    addWidget(previewArea,           PreviewItemMode);
    addWidget(d->previewAlbumWidget, PreviewAlbumMode);
    addWidget(d->htmlView->view(),   HtmlViewMode);

    setPreviewMode(PreviewAlbumMode);

    // -----------------------------------------------------------------

    connect(d->backButton, SIGNAL( clicked() ),
            this, SIGNAL( backToAlbumSignal() ) );
             
    connect(d->editButton, SIGNAL( clicked() ),
            this, SIGNAL( editImageSignal() ) );          
             
    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));                

    connect(d->previewItemWidget, SIGNAL( previewStarted() ),
            this, SLOT( slotPreviewStarted() ) );          
    
    connect(d->previewItemWidget, SIGNAL( previewComplete() ),
            this, SLOT( slotPreviewComplete() ) );          
    
    connect(d->previewItemWidget, SIGNAL( previewFailed() ),
            this, SLOT( slotPreviewFailed() ) );    

    connect(d->htmlView->browserExtension(),
            SIGNAL(openURLRequest(const KURL &, const KParts::URLArgs &)),
            this, SLOT(slotUrlOpen(const KURL &)));      
}

AlbumWidgetStack::~AlbumWidgetStack()
{
    delete d;
}

void AlbumWidgetStack::slotUrlOpen(const KURL &url)
{
    KApplication::kApplication()->invokeBrowser(url.url());
}

void AlbumWidgetStack::slotThemeChanged()
{
    d->buttonsArea->setPaletteBackgroundColor(ThemeEngine::instance()->baseColor());
}

AlbumIconView* AlbumWidgetStack::albumIconView()
{
    return d->previewAlbumWidget;
}

ImagePreviewWidget* AlbumWidgetStack::imagePreviewWidget()
{
    return d->previewItemWidget;
}

void AlbumWidgetStack::setPreviewItem(const QString& path)
{
    if (path.isNull())
        slotPreviewFailed();
    
    visibleWidget()->setFocus();
    d->previewItemWidget->setImagePath(path);
}

int AlbumWidgetStack::previewMode(void)
{
    return id(visibleWidget());
}

void AlbumWidgetStack::setPreviewMode(int mode)
{
    if (mode != PreviewAlbumMode && mode != PreviewItemMode && mode != HtmlViewMode)
        return;

    raiseWidget(mode);
    visibleWidget()->setFocus();
}

void AlbumWidgetStack::slotPreviewStarted()
{
    d->backButton->setEnabled(false);
    d->editButton->setEnabled(false);
}

void AlbumWidgetStack::slotPreviewComplete()
{
    d->backButton->setEnabled(true);
    d->editButton->setEnabled(true);
}

void AlbumWidgetStack::slotPreviewFailed()
{
    d->backButton->setEnabled(true);
    d->editButton->setEnabled(false);
}

QString AlbumWidgetStack::infoPage()
{
    QString info =
        i18n("%1: digiKam version; %2: help:// URL; %3: homepage URL; "
        "%4: prior KMail version; %5: prior KDE version; "
        "%6: generated list of new features; "
        "%7: First-time user text (only shown on first start); "
        "%8: generated list of important changes; "
        "--- end of comment ---",
        "<h2 style='margin-top: 0px;'>Welcome to digiKam %1</h2><p>digiKam is a "
        "photo-management program for the K Desktop Environment. "
        "It is designed to organize your digital photograhs on your computer."
        "</p>\n"
        "<ul><li>digiKam has many powerful features which are described in the "
        "<a href=\"%2\">documentation</a></li>\n"
        "<li>The <a href=\"%3\">digiKam homepage</A> offers information about "
        "new versions of digiKam</li></ul>\n"
        "%8\n" // important changes
        "<p>Some of the new features in this release of digiKam include "
        "(compared to digiKam %4):</p>\n"
        "<ul>\n%5</ul>\n"
        "%6\n"
        "<p>We hope that you will enjoy digiKam.</p>\n"
        "<p>Thank you,</p>\n"
        "<p style='margin-bottom: 0px'>&nbsp; &nbsp; The digiKam Team</p>")
        .arg(digikam_version)            // current digiKam version
        .arg("help:/digikam/index.html") // digiKam help:// URL
        .arg("http://www.digikam.org/")  // digiKam homepage URL
        .arg("0.8.2");                   // previous digiKam release.
    
    QStringList newFeatures;
    newFeatures << i18n("16 bits/color/pixels image support");
    newFeatures << i18n("Full color management support");
    newFeatures << i18n("Makernote and IPTC metadata support");
    newFeatures << i18n("Geolocalization of photograph");
    newFeatures << i18n("...TODO: add more informations here...");

    QString featureItems;
    for ( uint i = 0 ; i < newFeatures.count() ; i++ )
        featureItems += i18n("<li>%1</li>\n").arg( newFeatures[i] );
    
    info = info.arg( featureItems );
    
    // Add first-time user text (only shown on first start).
    info = info.arg( QString::null ); 

    // Generated list of important changes    
    info = info.arg( QString::null ); 
    
    return info;
}

QCString AlbumWidgetStack::fileToString(const QString &aFileName)
{
    QCString result;
    QFileInfo info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile file(aFileName);
    
    if (aFileName.isEmpty() || len <= 0 || 
        !info.exists() || info.isDir() || !info.isReadable() ||
        !file.open(IO_Raw|IO_ReadOnly)) 
        return QCString();
    
    result.resize(len + 2);
    readLen = file.readBlock(result.data(), len);
    if (1 && result[len-1]!='\n')
    {
        result[len++] = '\n';
        readLen++;
    }
    result[len] = '\0';
    
    if (readLen < len)
        return QCString();
    
    return result;
}

}  // namespace Digikam


