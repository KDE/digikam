/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-30
 * Description : a button bar to navigate between album items
 *               using status bar.
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "statusnavigatebar.moc"

// Qt includes

#include <QToolButton>
#include <QHBoxLayout>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{

class StatusNavigateBar::StatusNavigateBarPriv
{
public:

    StatusNavigateBarPriv() :
        itemType(StatusNavigateBar::ItemCurrent),
        firstButton(0),
        prevButton(0),
        nextButton(0),
        lastButton(0)
    {
    }

    int          itemType;

    QToolButton* firstButton;
    QToolButton* prevButton;
    QToolButton* nextButton;
    QToolButton* lastButton;
};

StatusNavigateBar::StatusNavigateBar(QWidget* parent)
    : QWidget(parent), d(new StatusNavigateBarPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* lay = new QHBoxLayout(this);

    d->firstButton = new QToolButton(this);
    d->firstButton->setFocusPolicy(Qt::NoFocus);
    d->firstButton->setAutoRaise(true);
    d->firstButton->setIcon(SmallIcon("go-first"));
    d->firstButton->setToolTip( i18n("Go to the first item"));

    d->prevButton = new QToolButton(this);
    d->prevButton->setFocusPolicy(Qt::NoFocus);
    d->prevButton->setAutoRaise(true);
    d->prevButton->setIcon(SmallIcon("go-previous"));
    d->prevButton->setToolTip( i18n("Go to the previous item"));

    d->nextButton = new QToolButton(this);
    d->nextButton->setFocusPolicy(Qt::NoFocus);
    d->nextButton->setAutoRaise(true);
    d->nextButton->setIcon(SmallIcon("go-next"));
    d->nextButton->setToolTip( i18n("Go to the next item"));

    d->lastButton = new QToolButton(this);
    d->lastButton->setFocusPolicy(Qt::NoFocus);
    d->lastButton->setAutoRaise(true);
    d->lastButton->setIcon(SmallIcon("go-last"));
    d->lastButton->setToolTip( i18n("Go to the last item"));

    lay->addWidget(d->firstButton);
    lay->addWidget(d->prevButton);
    lay->addWidget(d->nextButton);
    lay->addWidget(d->lastButton);
    lay->setMargin(0);
    lay->setSpacing(0);

    // ----------------------------------------------------------------

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
    {
        setButtonsState(ItemCurrent);
    }
    else if (!hasPrev && hasNext)
    {
        setButtonsState(ItemFirst);
    }
    else if (hasPrev && !hasNext)
    {
        setButtonsState(ItemLast);
    }
    else
    {
        setButtonsState(NoNavigation);
    }
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

int StatusNavigateBar::getButtonsState() const
{
    return (d->itemType);
}

}  // namespace Digikam
