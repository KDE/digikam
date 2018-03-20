/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef PANO_PREVIEW_PAGE_H
#define PANO_PREVIEW_PAGE_H

// Local includes

#include "dwizardpage.h"
#include "panoactions.h"

class QMutexLocker;

namespace Digikam
{

class PanoManager;

class PanoPreviewPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit PanoPreviewPage(PanoManager* const mngr, QWizard* const dlg);
    ~PanoPreviewPage();

private:

    void computePreview();
    void startStitching();

    void preInitializePage();
    void initializePage();
    bool validatePage();
    void cleanupPage();
    void cleanupPage(QMutexLocker& lock);

Q_SIGNALS:

    void signalPreviewFinished();
    void signalStitchingFinished();

private Q_SLOTS:

    void slotCancel();
    void slotStartStitching();
    void slotPanoAction(const Digikam::PanoActionData&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // PANO_PREVIEW_PAGE_H
