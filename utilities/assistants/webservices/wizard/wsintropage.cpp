/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "wsintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QIcon>
#include <QGroupBox>
#include <QGridLayout>
#include <QRadioButton>
#include <QButtonGroup>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "wswizard.h"
#include "wssettings.h"

namespace Digikam
{

class WSIntroPage::Private
{
public:

    Private(QWizard* const dialog)
      : imageGetOption(0),
        hbox(0),
        wizard(0),
        iface(0),
        btnGrp(0),
        flickrBtn(0),
        dropboxBtn(0),
        imgurBtn(0)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    QComboBox*        imageGetOption;
    DHBox*            hbox;
    WSWizard*         wizard;
    DInfoInterface*   iface;
    QButtonGroup*     btnGrp;
    QRadioButton*     flickrBtn;
    QRadioButton*     dropboxBtn;
    QRadioButton*     imgurBtn;
};

WSIntroPage::WSIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                       "<p><h1><b>Welcome to Web Services Tool</b></h1></p>"
                       "<p>This assistant will guide you to export "
                       "your items to popular Internet data hosting service.</p>"
                       "<p>Before to export contents, you will be able to adjust items properties "
                       "accordingly with your remote Web service capabilities.</p>"
                       "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(WSSettings::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(WSSettings::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    // --------------------

    QGroupBox* const wsBox      = new QGroupBox(vbox);
    QVBoxLayout* const wsLayout = new QVBoxLayout(wsBox);
    wsBox->setLayout(wsLayout);
    wsBox->setTitle(i18nc("@title:group", "Remote Web Service"));
    d->btnGrp     = new QButtonGroup(wsBox);
    QMap<WSSettings::WebService, QString> map = WSSettings::webServiceNames();
    d->flickrBtn  = new QRadioButton(map[WSSettings::FLICKR],  wsBox);
    d->dropboxBtn = new QRadioButton(map[WSSettings::DROPBOX], wsBox);
    d->imgurBtn   = new QRadioButton(map[WSSettings::IMGUR],   wsBox);
    d->btnGrp->setExclusive(true);
    d->btnGrp->addButton(d->flickrBtn,  WSSettings::FLICKR);
    d->btnGrp->addButton(d->dropboxBtn, WSSettings::DROPBOX);
    d->btnGrp->addButton(d->imgurBtn,   WSSettings::IMGUR);

    wsLayout->addWidget(d->flickrBtn);
    wsLayout->addWidget(d->dropboxBtn);
    wsLayout->addWidget(d->imgurBtn);

    vbox->setStretchFactor(desc,    3);
    vbox->setStretchFactor(d->hbox, 1);
    vbox->setStretchFactor(wsBox,   3);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-html")));

    connect(d->btnGrp, SIGNAL(buttonClicked(int)),
            this, SLOT(slotWSChanged(int)));
}

WSIntroPage::~WSIntroPage()
{
    delete d;
}

void WSIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(WSSettings::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->wizard->settings()->selMode);
    }

    slotWSChanged(d->btnGrp->checkedId());
}

bool WSIntroPage::validatePage()
{
    d->wizard->settings()->selMode = (WSSettings::Selection)d->imageGetOption->currentIndex();

    return true;
}

void WSIntroPage::slotWSChanged(int i)
{
    d->wizard->settings()->webService = (WSSettings::WebService)i;

    emit completeChanged();
}

} // namespace Digikam
