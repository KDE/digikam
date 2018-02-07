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

#include "imageshackwidget.h"

// Qt includes

#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QStringList>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QPushButton>
#include <QMimeDatabase>
#include <QMimeType>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimageslist.h"
#include "imageshacksession.h"

namespace Digikam
{

ImageShackWidget::ImageShackWidget(QWidget* const parent,
                                   ImageShackSession* const session,
                                   DInfoInterface* const iface,
                                   const QString& toolName)
    : WSSettingsWidget(parent, iface, toolName),
      m_session(session)
{
    m_iface              = iface;
    m_imgList            = imagesList();
    m_headerLbl          = getHeaderLbl();
    m_accountNameLbl     = getUserNameLabel();
    m_chgRegCodeBtn      = getChangeUserBtn();
    m_reloadGalleriesBtn = getReloadBtn();
    m_galleriesCob       = getAlbumsCoB();
    m_progressBar        = progressBar();

    connect(m_reloadGalleriesBtn, SIGNAL(clicked()),
            this, SLOT(slotReloadGalleries()));

    QGroupBox* const tagsBox      = new QGroupBox(QString::fromLatin1(""), getSettingsBox());
    QGridLayout* const tagsLayout = new QGridLayout(tagsBox);

    m_privateImagesChb    = new QCheckBox(tagsBox);
    m_privateImagesChb->setText(i18n("Make private"));
    m_privateImagesChb->setChecked(false);

    m_tagsFld             = new QLineEdit(tagsBox);
    QLabel* const tagsLbl = new QLabel(i18n("Tags (optional):"), tagsBox);

    m_remBarChb           = new QCheckBox(i18n("Remove information bar on thumbnails"));
    m_remBarChb->setChecked(false);

    tagsLayout->addWidget(m_privateImagesChb, 0, 0);
    tagsLayout->addWidget(tagsLbl,            1, 0);
    tagsLayout->addWidget(m_tagsFld,          1, 1);

    addWidgetToSettingsBox(tagsBox);

    getUploadBox()->hide();
    getSizeBox()->hide();

    updateLabels();
}

ImageShackWidget::~ImageShackWidget()
{
}

void ImageShackWidget::updateLabels(const QString& /*name*/, const QString& /*url*/)
{
    if (m_session->loggedIn())
    {
        m_accountNameLbl->setText(m_session->username());
    }
    else
    {
        m_accountNameLbl->clear();
    }
}

void ImageShackWidget::slotGetGalleries(const QStringList &gTexts, const QStringList &gNames)
{
    m_galleriesCob->clear();

    m_galleriesCob->addItem(i18nc("@item:inlistbox", "Add to root folder"),
                            QString::fromLatin1("--add-to-root--"));

    m_galleriesCob->addItem(i18nc("@item:inlistbox", "Create new gallery"),
                            QString::fromLatin1("--new-gallery--"));

    // TODO check if the lists have the same size
    for (int i = 0; i < gTexts.size(); ++i)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "gTexts is "<<gTexts[i] << " gNames is "<<gNames[i];
        m_galleriesCob->addItem(gTexts[i], gNames[i]);
    }

//     slotEnableNewGalleryLE(m_galleriesCob->currentIndex());
}

void ImageShackWidget::slotReloadGalleries()
{
    emit signalReloadGalleries();
}

}  // namespace Digikam
