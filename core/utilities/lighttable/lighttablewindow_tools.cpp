/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam light table - Extra tools
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

void LightTableWindow::slotEditItem(const ImageInfo& info)
{
    ImageWindow* const im = ImageWindow::imageWindow();
    ImageInfoList list    = d->thumbView->allImageInfos();

    im->loadImageInfos(list, info, i18n("Light Table"));

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

void LightTableWindow::slotEditMetadata()
{
    if (d->thumbView->currentInfo().isNull())
    {
        return;
    }

    QUrl url = d->thumbView->currentInfo().fileUrl();

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(QApplication::activeWindow(),
                                                                 QList<QUrl>() << url);
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from file.
    CollectionScanner scanner;
    scanner.scanFile(url.toLocalFile(), CollectionScanner::Rescan);
}

void LightTableWindow::slotEditGeolocation()
{
#ifdef HAVE_MARBLE
    ImageInfoList infos = d->thumbView->allImageInfos();

    if (infos.isEmpty())
    {
        return;
    }

    TagModel* const tagModel                    = new TagModel(AbstractAlbumModel::IgnoreRootAlbum, this);
    TagPropertiesFilterModel* const filterModel = new TagPropertiesFilterModel(this);
    filterModel->setSourceAlbumModel(tagModel);
    filterModel->sort(0);

    QPointer<GeolocationEdit> dialog = new GeolocationEdit(filterModel,
                                                           new DBInfoIface(this, d->thumbView->allUrls()),
                                                           QApplication::activeWindow());
    dialog->setItems(ImageGPS::infosToItems(infos));
    dialog->exec();

    delete dialog;

    // Refresh Database with new metadata from files.
    foreach(const ImageInfo& inf, infos)
    {
        ScanController::instance()->scannedInfo(inf.fileUrl().toLocalFile());
    }
#endif
}

void LightTableWindow::slotPrintCreator()
{
    QPointer<AdvPrintWizard> w = new AdvPrintWizard(this, new DBInfoIface(this, d->thumbView->allUrls()));
    w->exec();
    delete w;
}

void LightTableWindow::slotPresentation()
{
    QPointer<Digikam::PresentationMngr> mngr = new PresentationMngr(this);

    foreach(const ImageInfo& info, d->thumbView->allImageInfos())
    {
        mngr->addFile(info.fileUrl(), info.comment());
        qApp->processEvents();
    }

    mngr->showConfigDialog();
}

void LightTableWindow::slotSlideShowAll()
{
   SlideShowBuilder* const builder = new SlideShowBuilder(d->thumbView->allImageInfos());

   d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                            i18n("Preparing slideshow. Please wait..."));

   connect(builder, SIGNAL(signalComplete(SlideShowSettings)),
           this, SLOT(slotSlideShowBuilderComplete(SlideShowSettings)));

   builder->run();
}

void LightTableWindow::slotLeftSlideShowManualFromCurrent()
{
    slotSlideShowManualFrom(d->previewView->leftImageInfo());
    d->fromLeftPreview = true;
}

void LightTableWindow::slotRightSlideShowManualFromCurrent()
{
    slotSlideShowManualFrom(d->previewView->rightImageInfo());
    d->fromLeftPreview = false;
}

void LightTableWindow::slotSlideShowManualFrom(const ImageInfo& info)
{
   SlideShowBuilder* const builder = new SlideShowBuilder(d->thumbView->allImageInfos());
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
