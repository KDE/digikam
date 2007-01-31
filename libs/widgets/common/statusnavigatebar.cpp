/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-01-30
 * Description : a button bar to navigate between album items
 *               using status bar.
 * 
 * Copyright 2007 by Gilles Caulier
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
#include <qtoolbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <kiconloader.h>
#include <klocale.h>
 
// Local includes

#include "statusnavigatebar.h"
#include "statusnavigatebar.moc"

namespace Digikam
{

class StatusNavigateBarPriv
{
public:

    StatusNavigateBarPriv()
    {
        firstButton = 0;
        prevButton  = 0;
        nextButton  = 0;
        lastButton  = 0;
        itemType    = StatusNavigateBar::ItemCurrent;
    }

    int          itemType;

    QToolButton *firstButton;
    QToolButton *prevButton;
    QToolButton *nextButton;
    QToolButton *lastButton;
};

StatusNavigateBar::StatusNavigateBar(QWidget *parent)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new StatusNavigateBarPriv;
    
    QHBoxLayout *lay = new QHBoxLayout(this);
    
    d->firstButton = new QToolButton(this);
    d->firstButton->setAutoRaise(true); 
    d->firstButton->setPixmap(SmallIcon("start"));
    QToolTip::add(d->firstButton, i18n("Go to the first item"));
    
    d->prevButton = new QToolButton(this);
    d->prevButton->setAutoRaise(true); 
    d->prevButton->setPixmap(SmallIcon("back"));
    QToolTip::add(d->prevButton, i18n("Go to the previous item"));
 
    d->nextButton = new QToolButton(this);
    d->nextButton->setAutoRaise(true); 
    d->nextButton->setPixmap(SmallIcon("forward"));
    QToolTip::add(d->nextButton, i18n("Go to the next item"));

    d->lastButton = new QToolButton(this);
    d->lastButton->setAutoRaise(true); 
    d->lastButton->setPixmap(SmallIcon("finish"));
    QToolTip::add(d->lastButton, i18n("Go to the last item"));
    
    lay->addWidget(d->firstButton);
    lay->addWidget(d->prevButton);
    lay->addWidget(d->nextButton);
    lay->addWidget(d->lastButton);
    
    connect(d->firstButton, SIGNAL(clicked()),
            this, SIGNAL(signalFirstItem()));
    
    connect(d->prevButton, SIGNAL(clicked()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->nextButton, SIGNAL(clicked()),
            this, SIGNAL(signalNextItem()));
    
    connect(d->lastButton, SIGNAL(clicked()),
            this, SIGNAL(signalLastItem()));
}      

StatusNavigateBar::~StatusNavigateBar()
{
    delete d;
}

void StatusNavigateBar::setNavigateBarState(bool hasPrev, bool hasNext)
{
    if (hasPrev && hasNext)
        setButtonsState(ItemCurrent);
    else if (!hasPrev && hasNext)
        setButtonsState(ItemFirst);
    else if (hasPrev && !hasNext)
        setButtonsState(ItemLast);
    else
        setButtonsState(NoNavigation);
}

void StatusNavigateBar::setButtonsState(int itemType)
{
    d->itemType = itemType;

    if (d->itemType == ItemFirst)
    {
       d->firstButton->setEnabled(false);
       d->prevButton->setEnabled(false);
       d->nextButton->setEnabled(true);
       d->lastButton->setEnabled(true);
    }
    else if (d->itemType == ItemLast)
    {
       d->firstButton->setEnabled(true);
       d->prevButton->setEnabled(true);
       d->nextButton->setEnabled(false);
       d->lastButton->setEnabled(false);
    }
    else if (d->itemType == ItemCurrent)
    {
       d->firstButton->setEnabled(true);
       d->prevButton->setEnabled(true);
       d->nextButton->setEnabled(true);
       d->lastButton->setEnabled(true);
    }
    else if (d->itemType == NoNavigation)
    {
       d->firstButton->setEnabled(false);
       d->prevButton->setEnabled(false);
       d->nextButton->setEnabled(false);
       d->lastButton->setEnabled(false);
    }
}

int StatusNavigateBar::getButtonsState()
{
    return (d->itemType);
}

}  // namespace Digikam

