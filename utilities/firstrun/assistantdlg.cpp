/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "assistantdlg.moc"

// KDE includes

#include <klocale.h>

// Locale incudes.

#include "welcomepage.h"
#include "collectionpage.h"
#include "rawpage.h"
#include "metadatapage.h"
#include "previewpage.h"
#include "openfilepage.h"
#include "tooltipspage.h"
#include "startscanpage.h"

namespace Digikam
{

class AssistantDlg::Private
{
public:

    Private() :
        welcomePage(0),
        collectionPage(0),
        rawPage(0),
        metadataPage(0),
        previewPage(0),
        openFilePage(0),
        tooltipsPage(0),
        startScanPage(0)
    {
    }

    WelcomePage*    welcomePage;
    CollectionPage* collectionPage;
    RawPage*        rawPage;
    MetadataPage*   metadataPage;
    PreviewPage*    previewPage;
    OpenFilePage*   openFilePage;
    TooltipsPage*   tooltipsPage;
    StartScanPage*  startScanPage;
};

AssistantDlg::AssistantDlg(QWidget* const parent)
    : KAssistantDialog(parent), d(new Private)
{
    setHelp("firstrundialog.anchor", "digikam");

    d->welcomePage    = new WelcomePage(this);    // First assistant page
    d->collectionPage = new CollectionPage(this);
    d->rawPage        = new RawPage(this);
    d->metadataPage   = new MetadataPage(this);
    d->previewPage    = new PreviewPage(this);
    d->openFilePage   = new OpenFilePage(this);
    d->tooltipsPage   = new TooltipsPage(this);

    // NOTE: Added here new assistant pages...

    d->startScanPage  = new StartScanPage(this);  // Last assistant page

    resize(600, 500);

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotFinishPressed()));
}

AssistantDlg::~AssistantDlg()
{
    delete d;
}

QString AssistantDlg::firstAlbumPath() const
{
    return d->collectionPage->firstAlbumPath();
}

QString AssistantDlg::databasePath() const
{
    return d->collectionPage->databasePath();
}

void AssistantDlg::next()
{
    if (currentPage() == d->collectionPage->page())
    {
        if (!d->collectionPage->checkSettings())
        {
            return;
        }
    }

    KAssistantDialog::next();
}

void AssistantDlg::slotFinishPressed()
{
    // Save settings to rc files.
    d->collectionPage->saveSettings();
    d->rawPage->saveSettings();
    d->metadataPage->saveSettings();
    d->previewPage->saveSettings();
    d->openFilePage->saveSettings();
    d->tooltipsPage->saveSettings();
}

}   // namespace Digikam
