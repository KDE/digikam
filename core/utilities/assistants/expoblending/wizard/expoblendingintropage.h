/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef EXPO_BLENDING_INTRO_PAGE_H
#define EXPO_BLENDING_INTRO_PAGE_H

// Local includes

#include "dwizardpage.h"
#include "expoblendingmanager.h"

namespace Digikam
{

class ExpoBlendingIntroPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit ExpoBlendingIntroPage(ExpoBlendingManager* const mngr, QWizard* const dlg);
    ~ExpoBlendingIntroPage();

    bool binariesFound();

Q_SIGNALS:

    void signalExpoBlendingIntroPageIsValid(bool);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // EXPO_BLENDING_INTRO_PAGE_H
