/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam image editor - extra tools
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagewindow.h"
#include "imagewindow_p.h"

namespace Digikam
{

void ImageWindow::slideShow(SlideShowSettings& settings)
{
    settings.exifRotate = MetaEngineSettings::instance()->settings().exifRotate;

    if (!d->imageInfoModel->isEmpty())
    {
        // We have started image editor from Album GUI. we get picture comments from database.

        foreach (const ItemInfo& info, d->imageInfoModel->imageInfos())
        {
            settings.fileList << info.fileUrl();
        }
    }

/*
    else
    {
        // We have started image editor from Camera GUI. we get picture comments from metadata.

        settings.fileList   = d->urlList;
    }
*/

    QPointer<Digikam::SlideShow> slide = new SlideShow(new DBInfoIface(this, d->thumbBar->allUrls(), ApplicationSettings::Slideshow), settings);
    TagsActionMngr::defaultManager()->registerActionsToWidget(slide);

    if (settings.startWithCurrent)
    {
        slide->setCurrentItem(d->currentUrl());
    }

    connect(slide, SIGNAL(signalRatingChanged(QUrl,int)),
            this, SLOT(slotRatingChanged(QUrl,int)));

    connect(slide, SIGNAL(signalColorLabelChanged(QUrl,int)),
            this, SLOT(slotColorLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalPickLabelChanged(QUrl,int)),
            this, SLOT(slotPickLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalToggleTag(QUrl,int)),
            this, SLOT(slotToggleTag(QUrl,int)));

    slide->show();
}

} // namespace Digikam
