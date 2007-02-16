/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-07-07
 * Description : a navigate bar with text 
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

// KDE includes.

#include <ksqueezedtextlabel.h>
#include <kdialogbase.h>
#include <klocale.h>
 
// Local includes

#include "statusnavigatebar.h"
#include "navigatebarwidget.h"
#include "navigatebarwidget.moc"

namespace Digikam
{

class NavigateBarWidgetPriv
{
public:

    NavigateBarWidgetPriv()
    {
        filename = 0;
        navBar   = 0;
    }
    
    KSqueezedTextLabel *filename;

    StatusNavigateBar  *navBar;
};

NavigateBarWidget::NavigateBarWidget(QWidget *parent, bool show)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new NavigateBarWidgetPriv;
    
    QHBoxLayout *lay = new QHBoxLayout(this);
    d->navBar   = new StatusNavigateBar(this);    
    d->filename = new KSqueezedTextLabel(this);
    
    lay->addWidget(d->navBar);
    lay->addSpacing( KDialog::spacingHint() );
    lay->addWidget(d->filename);

    if (!show) hide();
    
    connect(d->navBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
    
    connect(d->navBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->navBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));
    
    connect(d->navBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
}      

NavigateBarWidget::~NavigateBarWidget()
{
    delete d;
}

void NavigateBarWidget::setFileName(QString filename)
{
    d->filename->setText(filename);
}

QString NavigateBarWidget::getFileName()
{
    return (d->filename->text());
}

void NavigateBarWidget::setButtonsState(int itemType)
{
    d->navBar->setButtonsState(itemType);
}

int NavigateBarWidget::getButtonsState()
{
    return (d->navBar->getButtonsState());
}

}  // namespace Digikam

