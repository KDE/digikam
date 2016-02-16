/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a plugin to blend bracketed images.
 *
 * Copyright (C) 2009-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importwizarddlg.h"

// Qt includes

#include <QDesktopWidget>
#include <QApplication>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Locale incudes

#include "intropage.h"
#include "itemspage.h"
#include "lastpage.h"
#include "expoblendingmanager.h"
#include "preprocessingpage.h"

namespace Digikam
{

struct ImportWizardDlg::ImportWizardDlgPriv
{
    ImportWizardDlgPriv()
        : mngr(0),
          introPage(0),
          itemsPage(0),
          preProcessingPage(0),
          lastPage(0),
          preProcessed(false)
    {
    }

    ExpoBlendingManager* mngr;

    IntroPage*           introPage;
    ItemsPage*           itemsPage;
    PreProcessingPage*   preProcessingPage;
    LastPage*            lastPage;

    bool                 preProcessed;
};

ImportWizardDlg::ImportWizardDlg(ExpoBlendingManager* const mngr, QWidget* const parent)
    : QWizard(parent),
      d(new ImportWizardDlgPriv)
{
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Exposure Blending Import Wizard"));
    setWizardStyle(QWizard::ClassicStyle);
    setButtonLayout(QList<QWizard::WizardButton>() << QWizard::BackButton
                                                   << QWizard::CancelButton
                                                   << QWizard::NextButton
                                                   << QWizard::FinishButton);
    d->mngr              = mngr;
    d->introPage         = new IntroPage(d->mngr, this);
    d->itemsPage         = new ItemsPage(d->mngr, this);
    d->preProcessingPage = new PreProcessingPage(d->mngr, this);
    d->lastPage          = new LastPage(d->mngr, this);

    // ---------------------------------------------------------------

    QDesktopWidget* const desktop = QApplication::desktop();
    int screen                    = desktop->screenNumber();
    QRect srect                   = desktop->availableGeometry(screen);
    resize(800 <= srect.width()  ? 800 : srect.width(),
           750 <= srect.height() ? 750 : srect.height());

    // ---------------------------------------------------------------

    connect(d->introPage, SIGNAL(signalIntroPageIsValid(bool)),
            this, SLOT(slotIntroPageIsValid(bool)));

    connect(d->itemsPage, SIGNAL(signalItemsPageIsValid(bool)),
            this, SLOT(slotItemsPageIsValid(bool)));

    connect(d->preProcessingPage, SIGNAL(signalPreProcessed(ItemUrlsMap)),
            this, SLOT(slotPreProcessed(ItemUrlsMap)));

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotCurrentIdChanged(int)));

    d->introPage->setComplete(d->introPage->binariesFound());
}

ImportWizardDlg::~ImportWizardDlg()
{
    delete d;
}

ExpoBlendingManager* ImportWizardDlg::manager() const
{
    return d->mngr;
}

QList<QUrl> ImportWizardDlg::itemUrls() const
{
    return d->itemsPage->itemUrls();
}

bool ImportWizardDlg::validateCurrentPage()
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

void ImportWizardDlg::slotCurrentIdChanged(int id)
{
    if (page(id) != d->lastPage && d->preProcessed)
    {
        d->preProcessed = false;
        d->preProcessingPage->cancel();
        d->preProcessingPage->setComplete(true);
    }
}

void ImportWizardDlg::slotIntroPageIsValid(bool binariesFound)
{
    d->introPage->setComplete(binariesFound);
}

void ImportWizardDlg::slotPreProcessed(const ItemUrlsMap& map)
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

void ImportWizardDlg::slotItemsPageIsValid(bool valid)
{
    d->itemsPage->setComplete(valid);
}

} // namespace Digikam
