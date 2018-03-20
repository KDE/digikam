/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "expoblendingwizard.h"

// Qt includes

#include <QDesktopWidget>
#include <QApplication>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Locale incudes

#include "expoblendingintropage.h"
#include "expoblendingitemspage.h"
#include "expoblendinglastpage.h"
#include "expoblendingmanager.h"
#include "expoblendingpreprocesspage.h"

namespace Digikam
{

class ExpoBlendingWizard::Private
{
public:

    explicit Private()
      : mngr(0),
        introPage(0),
        itemsPage(0),
        preProcessingPage(0),
        lastPage(0),
        preProcessed(false)
    {
    }

    ExpoBlendingManager*        mngr;

    ExpoBlendingIntroPage*      introPage;
    ItemsPage*                  itemsPage;
    ExpoBlendingPreProcessPage* preProcessingPage;
    ExpoBlendingLastPage*       lastPage;

    bool                        preProcessed;
};

ExpoBlendingWizard::ExpoBlendingWizard(ExpoBlendingManager* const mngr, QWidget* const parent)
    : DWizardDlg(parent, QLatin1String("ExpoBlending Dialog")),
      d(new Private)
{
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Stacked Images Tool"));

    d->mngr              = mngr;
    d->introPage         = new ExpoBlendingIntroPage(d->mngr, this);
    d->itemsPage         = new ItemsPage(d->mngr, this);
    d->preProcessingPage = new ExpoBlendingPreProcessPage(d->mngr, this);
    d->lastPage          = new ExpoBlendingLastPage(d->mngr, this);

    // ---------------------------------------------------------------

    connect(d->introPage, SIGNAL(signalExpoBlendingIntroPageIsValid(bool)),
            this, SLOT(slotExpoBlendingIntroPageIsValid(bool)));

    connect(d->itemsPage, SIGNAL(signalItemsPageIsValid(bool)),
            this, SLOT(slotItemsPageIsValid(bool)));

    connect(d->preProcessingPage, SIGNAL(signalPreProcessed(ExpoBlendingItemUrlsMap)),
            this, SLOT(slotPreProcessed(ExpoBlendingItemUrlsMap)));

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotCurrentIdChanged(int)));

    d->introPage->setComplete(d->introPage->binariesFound());
}

ExpoBlendingWizard::~ExpoBlendingWizard()
{
    delete d;
}

ExpoBlendingManager* ExpoBlendingWizard::manager() const
{
    return d->mngr;
}

QList<QUrl> ExpoBlendingWizard::itemUrls() const
{
    return d->itemsPage->itemUrls();
}

bool ExpoBlendingWizard::validateCurrentPage()
{
    if (currentPage() == d->itemsPage)
    {
        d->mngr->setItemsList(d->itemsPage->itemUrls());
    }
    else if (currentPage() == d->preProcessingPage && !d->preProcessed)
    {
        // Do not give access to Next button during alignment process.
        d->preProcessingPage->setComplete(false);
        d->preProcessingPage->process();
        d->preProcessed = true;
        // Next is handled with signals/slots
        return false;
    }

    return true;
}

void ExpoBlendingWizard::slotCurrentIdChanged(int id)
{
    if (page(id) != d->lastPage && d->preProcessed)
    {
        d->preProcessed = false;
        d->preProcessingPage->cancel();
        d->preProcessingPage->setComplete(true);
    }
}

void ExpoBlendingWizard::slotExpoBlendingIntroPageIsValid(bool binariesFound)
{
    d->introPage->setComplete(binariesFound);
}

void ExpoBlendingWizard::slotPreProcessed(const ExpoBlendingItemUrlsMap& map)
{
    if (map.isEmpty())
    {
        // pre-processing failed.
        d->preProcessingPage->setComplete(false);
        d->preProcessed = false;
    }
    else
    {
        // pre-processing Done.
        d->mngr->setPreProcessedMap(map);
        next();
    }
}

void ExpoBlendingWizard::slotItemsPageIsValid(bool valid)
{
    d->itemsPage->setComplete(valid);
}

} // namespace Digikam
