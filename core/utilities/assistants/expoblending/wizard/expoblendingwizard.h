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

#ifndef EXPOBLENDINGWIZARD_H
#define EXPOBLENDINGWIZARD_H

// Qt includes

#include <QString>
#include <QWidget>

// Local includes

#include "dwizarddlg.h"
#include "expoblendingactions.h"

namespace Digikam
{
class ExpoBlendingManager;

class ExpoBlendingWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit ExpoBlendingWizard(ExpoBlendingManager* const mngr, QWidget* const parent = 0);
    ~ExpoBlendingWizard();

    QList<QUrl> itemUrls() const;

    ExpoBlendingManager* manager() const;

    virtual bool validateCurrentPage();

private Q_SLOTS:

    void slotCurrentIdChanged(int);
    void slotExpoBlendingIntroPageIsValid(bool);
    void slotItemsPageIsValid(bool);
    void slotPreProcessed(const ExpoBlendingItemUrlsMap&);

private:

    class Private;
    Private* const d;
};

}   // namespace Digikam

#endif /* EXPOBLENDINGWIZARD_H */
