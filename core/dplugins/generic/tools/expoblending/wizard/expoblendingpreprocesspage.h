/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_EXPO_BLENDING_PREPROCESS_PAGE_H
#define DIGIKAM_EXPO_BLENDING_PREPROCESS_PAGE_H

// Local includes

#include "dwizardpage.h"
#include "expoblendingactions.h"

using namespace Digikam;

namespace GenericExpoBlendingPlugin
{

class ExpoBlendingManager;

class ExpoBlendingPreProcessPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit ExpoBlendingPreProcessPage(ExpoBlendingManager* const mngr, QWizard* const dlg);
    ~ExpoBlendingPreProcessPage();

    void process();
    void cancel();

Q_SIGNALS:

    void signalPreProcessed(const ExpoBlendingItemUrlsMap&);

private Q_SLOTS:

    void slotProgressTimerDone();
    void slotExpoBlendingAction(const GenericExpoBlendingPlugin::ExpoBlendingActionData&);

private:

    void resetTitle();

private:

    class Private;
    Private* const d;
};

} // namespace GenericExpoBlendingPlugin

#endif // DIGIKAM_EXPO_BLENDING_PREPROCESS_PAGE_H
