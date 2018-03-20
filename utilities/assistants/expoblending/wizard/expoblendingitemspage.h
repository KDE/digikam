/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#ifndef EXPO_BLENDING_ITEMS_PAGE_H
#define EXPO_BLENDING_ITEMS_PAGE_H

// Local includes

#include "dwizardpage.h"
#include "expoblendingactions.h"

namespace Digikam
{

class ExpoBlendingManager;

class ItemsPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit ItemsPage(ExpoBlendingManager* const mngr, QWizard* const dlg);
    ~ItemsPage();

    QList<QUrl> itemUrls() const;

Q_SIGNALS:

    void signalItemsPageIsValid(bool);

private:

    void setIdentity(const QUrl& url, const QString& identity);

private Q_SLOTS:

    void slotSetupList();
    void slotImageListChanged();
    void slotAddItems(const QList<QUrl>&);
    void slotExpoBlendingAction(const Digikam::ExpoBlendingActionData&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // EXPO_BLENDING_ITEMS_PAGE_H
