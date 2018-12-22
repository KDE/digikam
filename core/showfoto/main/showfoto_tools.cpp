/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - extra tools
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

#include "showfoto.h"
#include "showfoto_p.h"

namespace ShowFoto
{

void ShowFoto::slideShow(Digikam::SlideShowSettings& settings)
{
    if (!d->thumbBar->showfotoItemInfos().size())
    {
        return;
    }

    settings.exifRotate = Digikam::MetaEngineSettings::instance()->settings().exifRotate;
    settings.fileList   = d->thumbBar->urls();
    int   i             = 0;
    float cnt           = settings.fileList.count();
    m_cancelSlideShow   = false;
    Digikam::DMetadata meta;

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::CancelProgressBarMode,
                                    i18n("Preparing slideshow. Please wait..."));

    for (QList<QUrl>::ConstIterator it = settings.fileList.constBegin() ;
         !m_cancelSlideShow && (it != settings.fileList.constEnd()) ; ++it)
    {
        Digikam::SlidePictureInfo pictInfo;
        meta.load((*it).toLocalFile());
        pictInfo.comment   = meta.getItemComments()[QLatin1String("x-default")].caption;
        pictInfo.photoInfo = meta.getPhotographInformation();
        settings.pictInfoMap.insert(*it, pictInfo);

        m_nameLabel->setProgressValue((int)((i++/cnt)*100.0f));
        qApp->processEvents();
    }

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::TextMode, QString());

    if (!m_cancelSlideShow)
    {
        QPointer<Digikam::SlideShow> slide = new Digikam::SlideShow(settings);

        if (settings.startWithCurrent)
        {
            slide->setCurrentItem(d->thumbBar->currentUrl());
        }

        slide->show();
    }
}

void ShowFoto::slotPresentation()
{
    QPointer<Digikam::PresentationMngr> mngr = new Digikam::PresentationMngr(this);
    Digikam::DMetadata meta;

    QList<QUrl> urlList = d->thumbBar->urls();
    int i               = 0;
    float cnt           = urlList.count();
    m_cancelSlideShow   = false;

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::CancelProgressBarMode,
                                    i18n("Preparing presentation. Please wait..."));

    for (QList<QUrl>::ConstIterator it = urlList.constBegin() ;
         !m_cancelSlideShow && (it != urlList.constEnd()) ; ++it)
    {
        meta.load((*it).toLocalFile());
        mngr->addFile(*it, meta.getItemComments()[QLatin1String("x-default")].caption);

        m_nameLabel->setProgressValue((int)((i++/cnt)*100.0f));
        qApp->processEvents();
    }

    m_nameLabel->setProgressBarMode(Digikam::StatusProgressBar::TextMode, QString());
    mngr->showConfigDialog();
}

void ShowFoto::slotFilePrint()
{
    printImage(d->thumbBar->currentUrl());
}

void ShowFoto::slotEditGeolocation()
{
#ifdef HAVE_MARBLE
    QList<QUrl> infos = d->thumbBar->urls();

    if (infos.isEmpty())
    {
        return;
    }

    QPointer<GeolocationEdit> dialog = new GeolocationEdit(0, new DMetaInfoIface(this, d->thumbBar->urls()), this);
    dialog->setImages(infos);
    dialog->exec();

    delete dialog;

    // Update image information everywhere.
    slotChanged();
#endif
}

void ShowFoto::slotTimeAdjust()
{
    QList<QUrl> urls = d->thumbBar->urls();

    if (urls.isEmpty())
        return;

    QPointer<TimeAdjustDialog> dialog = new TimeAdjustDialog(this, new DMetaInfoIface(this, urls));
    dialog->exec();

    delete dialog;

    // Update image information everywhere.
    slotChanged();
}

void ShowFoto::slotEditMetadata()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        return;
    }

    QPointer<MetadataEditDialog> dialog = new MetadataEditDialog(this, QList<QUrl>() << d->thumbBar->currentInfo().url);
    dialog->exec();

    delete dialog;

    // Update image information everywhere.
    slotChanged();
}

void ShowFoto::slotPrintCreator()
{
    QPointer<AdvPrintWizard> w = new AdvPrintWizard(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
}

} // namespace ShowFoto
