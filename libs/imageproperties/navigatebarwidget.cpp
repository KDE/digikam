/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-07-07
 * Description : A button bar to navigate between album items
 * 
 * Copyright 2005-2006 by Gilles Caulier
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
 
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <ksqueezedtextlabel.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>
 
// Local includes

#include "imagepropertiessidebardb.h"
#include "navigatebarwidget.h"

namespace Digikam
{

class NavigateBarWidgetPriv
{
public:

    NavigateBarWidgetPriv()
    {
        filename    = 0;
        firstButton = 0;
        prevButton  = 0;
        nextButton  = 0;
        lastButton  = 0;
    }

    QPushButton        *firstButton;
    QPushButton        *prevButton;
    QPushButton        *nextButton;
    QPushButton        *lastButton;
    
    KSqueezedTextLabel *filename;
};

NavigateBarWidget::NavigateBarWidget(QWidget *parent, bool show)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new NavigateBarWidgetPriv;
    
    QHBoxLayout *lay = new QHBoxLayout(this);
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    
    d->firstButton = new QPushButton( this );
    d->firstButton->setPixmap( iconLoader->loadIcon( "start", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( d->firstButton, i18n( "Go to the first item" ) );
    
    d->prevButton = new QPushButton( this );
    d->prevButton->setPixmap( iconLoader->loadIcon( "back", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( d->prevButton, i18n( "Go to the previous item" ) );
 
    d->nextButton = new QPushButton( this );
    d->nextButton->setPixmap( iconLoader->loadIcon( "forward", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( d->nextButton, i18n( "Go to the next item" ) );

    d->lastButton = new QPushButton( this );
    d->lastButton->setPixmap( iconLoader->loadIcon( "finish", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( d->lastButton, i18n( "Go to the last item" ) );
    
    d->filename = new KSqueezedTextLabel( this );
    
    lay->addWidget(d->firstButton);
    lay->addWidget(d->prevButton);
    lay->addWidget(d->nextButton);
    lay->addWidget(d->lastButton);
    lay->addSpacing( KDialog::spacingHint() );
    lay->addWidget(d->filename);

    if (!show) hide();
    
    connect(d->firstButton, SIGNAL(clicked()),
            this, SIGNAL(signalFirstItem()));
    
    connect(d->prevButton, SIGNAL(clicked()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->nextButton, SIGNAL(clicked()),
            this, SIGNAL(signalNextItem()));
    
    connect(d->lastButton, SIGNAL(clicked()),
            this, SIGNAL(signalLastItem()));
}      

NavigateBarWidget::~NavigateBarWidget()
{
    delete d;
}

void NavigateBarWidget::setFileName(QString filename)
{
    d->filename->setText( filename );
}

QString NavigateBarWidget::getFileName()
{
    return ( d->filename->text() );
}

void NavigateBarWidget::setButtonsState(int itemType)
{
    if (itemType == ItemFirst)
    {
       d->firstButton->setEnabled(false);
       d->prevButton->setEnabled(false);
       d->nextButton->setEnabled(true);
       d->lastButton->setEnabled(true);
    }
    else if (itemType == ItemLast)
    {
       d->firstButton->setEnabled(true);
       d->prevButton->setEnabled(true);
       d->nextButton->setEnabled(false);
       d->lastButton->setEnabled(false);
    }
    else 
    {
       d->firstButton->setEnabled(true);
       d->prevButton->setEnabled(true);
       d->nextButton->setEnabled(true);
       d->lastButton->setEnabled(true);
    }
}

}  // namespace Digikam

#include "navigatebarwidget.moc"
