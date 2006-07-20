/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-07-07
 * Description : a nice banner widget.
 * 
 * Copyright 2005 by Gilles Caulier
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
 
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qframe.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kurllabel.h>
#include <kstandarddirs.h>
 
// Local includes

#include "bannerwidget.h"

namespace DigikamImagePlugins
{

BannerWidget::BannerWidget(QWidget *parent, QString title)
            : QFrame(parent)
{
    QString directory;
    setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    
    KURLLabel *pixmapLabelLeft = new KURLLabel( this );
    pixmapLabelLeft->setText(QString::null);
    pixmapLabelLeft->setURL("http://extragear.kde.org/apps/digikamimageplugins");
    pixmapLabelLeft->setScaledContents( false );
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    QToolTip::add(pixmapLabelLeft, i18n("Visit DigikamImagePlugins project website"));
    layout->addWidget( pixmapLabelLeft );
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", 
                                     KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    
    QLabel *labelTitle = new QLabel( title, this );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    
    KURLLabel *pixmapLabelRight = new KURLLabel( this );
    pixmapLabelRight->setText(QString::null);
    pixmapLabelRight->setURL("http://www.digikam.org");
    pixmapLabelRight->setScaledContents( false );
    pixmapLabelRight->setPaletteBackgroundColor( QColor(201, 208, 255) );
    QToolTip::add(pixmapLabelRight, i18n("Visit digiKam project website"));
    layout->addWidget( pixmapLabelRight );
    KGlobal::dirs()->addResourceType("digikamlogo", 
                                     KGlobal::dirs()->kde_default("data") +
                                     "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("digikamlogo", "digikamlogo.png");
    
    pixmapLabelRight->setPixmap( QPixmap( directory + "digikamlogo.png" ) );
    
    // -------------------------------------------------------------
    
    connect(pixmapLabelLeft, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processURL(const QString&)));

    connect(pixmapLabelRight, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processURL(const QString&)));                        
}      

BannerWidget::~BannerWidget()
{
}

void BannerWidget::processURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

}  // namespace DigikamImagePlugins

#include "bannerwidget.moc"
