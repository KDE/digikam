/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "assistantdlg.h"
#include "assistantdlg.moc"

// KDE includes

#include <klocale.h>

// Locale incudes.

#include "welcomepage.h"
#include "collectionpage.h"

namespace Digikam
{

class AssistantDlgPriv
{
public:

    AssistantDlgPriv()
    {
        welcomePage    = 0;
        collectionPage = 0;
    }

    WelcomePage    *welcomePage;
    CollectionPage *collectionPage;
};

AssistantDlg::AssistantDlg(QWidget* parent)
            : KAssistantDialog(parent), d(new AssistantDlgPriv)
{
    setHelp("firstrundialog.anchor", "digikam");

    d->welcomePage    = new WelcomePage(this);
    d->collectionPage = new CollectionPage(this);

    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*, KPageWidgetItem*)),
            this, SLOT(slotPageChanged(KPageWidgetItem*, KPageWidgetItem*)));
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
        if (!d->collectionPage->applySettings())
            return;
    }

    KAssistantDialog::next();
}

void AssistantDlg::slotPageChanged(KPageWidgetItem* /*current*/, KPageWidgetItem* /*before*/)
{
    
}

}   // namespace Digikam
