/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "imageshackwidget_p.h"

namespace Digikam
{

ImageShackWidget::ImageShackWidget(QWidget* const parent,
                                   ImageShackSession* const session,
                                   DInfoInterface* const iface,
                                   const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName),
      d(new Private)
{
    d->session            = session;
    d->iface              = iface;
    d->imgList            = imagesList();
    d->headerLbl          = getHeaderLbl();
    d->accountNameLbl     = getUserNameLabel();
    d->chgRegCodeBtn      = getChangeUserBtn();
    d->reloadGalleriesBtn = getReloadBtn();
    d->galleriesCob       = getAlbumsCoB();
    d->progressBar        = progressBar();

    connect(d->reloadGalleriesBtn, SIGNAL(clicked()),
            this, SLOT(slotReloadGalleries()));

    QGroupBox* const tagsBox      = new QGroupBox(QString::fromLatin1(""), getSettingsBox());
    QGridLayout* const tagsLayout = new QGridLayout(tagsBox);

    d->privateImagesChb    = new QCheckBox(tagsBox);
    d->privateImagesChb->setText(i18n("Make private"));
    d->privateImagesChb->setChecked(false);

    d->tagsFld             = new QLineEdit(tagsBox);
    QLabel* const tagsLbl = new QLabel(i18n("Tags (optional):"), tagsBox);

    d->remBarChb           = new QCheckBox(i18n("Remove information bar on thumbnails"));
    d->remBarChb->setChecked(false);

    tagsLayout->addWidget(d->privateImagesChb, 0, 0);
    tagsLayout->addWidget(tagsLbl,            1, 0);
    tagsLayout->addWidget(d->tagsFld,          1, 1);

    addWidgetToSettingsBox(tagsBox);

    getUploadBox()->hide();
    getSizeBox()->hide();

    updateLabels();
}

ImageShackWidget::~ImageShackWidget()
{
    delete d;
}

void ImageShackWidget::updateLabels(const QString& /*name*/, const QString& /*url*/)
{
    if (d->session->loggedIn())
    {
        d->accountNameLbl->setText(d->session->username());
    }
    else
    {
        d->accountNameLbl->clear();
    }
}

void ImageShackWidget::slotGetGalleries(const QStringList &gTexts, const QStringList &gNames)
{
    d->galleriesCob->clear();

    d->galleriesCob->addItem(i18nc("@item:inlistbox", "Add to root folder"),
                            QString::fromLatin1("--add-to-root--"));

    d->galleriesCob->addItem(i18nc("@item:inlistbox", "Create new gallery"),
                            QString::fromLatin1("--new-gallery--"));

    // TODO check if the lists have the same size
    for (int i = 0; i < gTexts.size(); ++i)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "gTexts is "<<gTexts[i] << " gNames is "<<gNames[i];
        d->galleriesCob->addItem(gTexts[i], gNames[i]);
    }

//     slotEnableNewGalleryLE(d->galleriesCob->currentIndex());
}

void ImageShackWidget::slotReloadGalleries()
{
    emit signalReloadGalleries();
}

}  // namespace Digikam
