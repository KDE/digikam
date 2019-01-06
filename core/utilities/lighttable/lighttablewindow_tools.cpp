/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam light table - Extra tools
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablewindow.h"
#include "lighttablewindow_p.h"

namespace Digikam
{

void LightTableWindow::slotEditItem()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotEditItem(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotEditItem(const ItemInfo& info)
{
    ImageWindow* const im = ImageWindow::imageWindow();
    ItemInfoList list    = d->thumbView->allItemInfos();

    im->loadItemInfos(list, info, i18n("Light Table"));

    if (im->isHidden())
    {
        im->show();
    }
    else
    {
        im->raise();
    }

    im->setFocus();
}

void LightTableWindow::slotPresentation()
{
    QPointer<Digikam::PresentationMngr> mngr = new PresentationMngr(this);

    foreach (const ItemInfo& info, d->thumbView->allItemInfos())
    {
        mngr->addFile(info.fileUrl(), info.comment());
        qApp->processEvents();
    }

    mngr->showConfigDialog();
}

void LightTableWindow::slotSlideShowAll()
{
   SlideShowBuilder* const builder = new SlideShowBuilder(d->thumbView->allItemInfos());

   d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                            i18n("Preparing slideshow. Please wait..."));

   connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
           this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

   builder->run();
}

void LightTableWindow::slotLeftSlideShowManualFromCurrent()
{
    slotSlideShowManualFrom(d->previewView->leftItemInfo());
    d->fromLeftPreview = true;
}

void LightTableWindow::slotRightSlideShowManualFromCurrent()
{
    slotSlideShowManualFrom(d->previewView->rightItemInfo());
    d->fromLeftPreview = false;
}

void LightTableWindow::slotSlideShowManualFrom(const ItemInfo& info)
{
   SlideShowBuilder* const builder = new SlideShowBuilder(d->thumbView->allItemInfos());
   builder->setOverrideStartFrom(info);
   builder->setAutoPlayEnabled(false);

   d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                            i18n("Preparing slideshow. Please wait..."));

   connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
           this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

   builder->run();
}

void LightTableWindow::slotSlideShowBuilderComplete(const SlideShowSettings& settings)
{
    QPointer<Digikam::SlideShow> slide = new SlideShow(settings);
    slide->setInfoInterface(new DBInfoIface(this, QList<QUrl>()));
    TagsActionMngr::defaultManager()->registerActionsToWidget(slide);

    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode, QString());
    slotRefreshStatusBar();

    if (settings.imageUrl.isValid())
    {
        slide->setCurrentItem(settings.imageUrl);
    }
    else if (settings.startWithCurrent)
    {
        slide->setCurrentItem(d->thumbView->currentInfo().fileUrl());
    }

    connect(slide, SIGNAL(signalRatingChanged(QUrl,int)),
            d->thumbView, SLOT(slotRatingChanged(QUrl,int)));

    connect(slide, SIGNAL(signalColorLabelChanged(QUrl,int)),
            d->thumbView, SLOT(slotColorLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalPickLabelChanged(QUrl,int)),
            d->thumbView, SLOT(slotPickLabelChanged(QUrl,int)));

    connect(slide, SIGNAL(signalToggleTag(QUrl,int)),
            d->thumbView, SLOT(slotToggleTag(QUrl,int)));

    connect(slide, SIGNAL(signalLastItemUrl(QUrl)),
            this, SLOT(slotSlideShowLastItemUrl(QUrl)));

    slide->show();
}

void LightTableWindow::slotSlideShowLastItemUrl(const QUrl& url)
{
    if (d->fromLeftPreview && !d->navigateByPairAction->isChecked())
    {
        d->thumbView->blockSignals(true);
        d->thumbView->setCurrentUrl(url);
        d->thumbView->blockSignals(false);
        slotSetItemLeft();
    }
    else
    {
        d->thumbView->setCurrentUrl(url);
    }
}

} // namespace Digikam
