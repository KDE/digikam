/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef PANO_PRE_PROCESS_PAGE_H
#define PANO_PRE_PROCESS_PAGE_H

// Local includes

#include "dwizardpage.h"
#include "panoactions.h"

namespace Digikam
{

class PanoManager;

class PanoPreProcessPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit PanoPreProcessPage(PanoManager* const mngr, QWizard* const dlg);
    ~PanoPreProcessPage();

private:

    void process();
    void initializePage();
    bool validatePage();
    void cleanupPage();

Q_SIGNALS:

    void signalPreProcessed();

private Q_SLOTS:

    void slotProgressTimerDone();
    void slotPanoAction(const Digikam::PanoActionData&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PANO_PRE_PROCESS_PAGE_H
