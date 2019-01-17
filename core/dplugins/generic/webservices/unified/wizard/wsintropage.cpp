/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : intro page to export tool where user can choose web service to export,
 *               existent accounts and function mode (export/import).
 *
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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
#include <QSettings>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "wswizard.h"
#include "wssettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN WSIntroPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : imageGetOption(0),
        hbox(0),
        wizard(0),
        iface(0),
        wsOption(0),
        accountOption(0)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            iface    = wizard->iface();
            settings = wizard->settings();
        }

    }

    QComboBox*      imageGetOption;
    DHBox*          hbox;
    WSWizard*       wizard;
    WSSettings*     settings;
    DInfoInterface* iface;
    QComboBox*      wsOption;
    QComboBox*      accountOption;
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
                       "<p>This assistant will guide you step by step, to export "
                       "your items to popular Internet data hosting service.</p>"
                       "<p>Before exporting contents, you will be able to adjust items' properties "
                       "according to your remote Web service capabilities.</p>"
                       "</qt>"));

    /* --------------------
     * ComboBox for image selection method
     */

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose operation:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(WSSettings::EXPORT, i18n("Export to web services"));
    d->imageGetOption->insertItem(WSSettings::IMPORT, i18n("Import from web services"));
    getImageLabel->setBuddy(d->imageGetOption);

    connect(d->imageGetOption, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotImageGetOptionChanged(int)));

    /* --------------------
     * ComboBox for web service selection
     */

    DHBox* const wsBox          = new DHBox(vbox);
    QLabel* const wsLabel       = new QLabel(i18n("&Choose remote Web Service:"), wsBox);
    d->wsOption                 = new QComboBox(wsBox);
    QMap<WSSettings::WebService, QString> map                = WSSettings::webServiceNames();
    QMap<WSSettings::WebService, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        QString wsName = it.value().toLower();
        QIcon icon     = QIcon::fromTheme(wsName.remove(QLatin1Char(' ')));
        d->wsOption->addItem(icon, it.value(), (int)it.key());
        ++it;
    }

    wsLabel->setBuddy(d->wsOption);

    connect(d->wsOption, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(slotWebServiceOptionChanged(QString)));

    /* --------------------
     * ComboBox for user account selection
     *
     * An empty option is added to accounts, so that user can choose to login with new account
     */

    DHBox* const accountBox     = new DHBox(vbox);
    QLabel* const accountLabel  = new QLabel(i18n("&Choose account:"), accountBox);
    d->accountOption            = new QComboBox(accountBox);

    QStringList accounts        = QStringList(QString())
                                  << d->settings->allUserNames(map.constBegin().value());

    foreach(const QString& account, accounts)
    {
        d->accountOption->addItem(account);
    }

    accountLabel->setBuddy(d->accountOption);

    /* --------------------
     * Place widget in the view
     */

    vbox->setStretchFactor(desc,       3);
    vbox->setStretchFactor(d->hbox,    1);
    vbox->setStretchFactor(wsBox,      3);
    vbox->setStretchFactor(accountBox, 3);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-html")));
}

WSIntroPage::~WSIntroPage()
{
    delete d;
}

void WSIntroPage::slotImageGetOptionChanged(int index)
{
    QMap<WSSettings::WebService, QString> map   = WSSettings::webServiceNames();

    d->wsOption->clear();

    /*
     * index == 0 <=> Export
     * index == 1 <=> Import, for now we only have Google Photo and Smugmug in this option
     */
    if (index == 0)
    {
        QMap<WSSettings::WebService, QString>::const_iterator it = map.constBegin();

        while (it != map.constEnd())
        {
            QString wsName = it.value().toLower();
            QIcon icon = QIcon::fromTheme(wsName.remove(QLatin1Char(' ')));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << wsName.remove(QLatin1Char(' '));
            d->wsOption->addItem(icon, it.value(), (int)it.key());
            ++it;
        }
    }
    else
    {
        d->wsOption->addItem(QIcon::fromTheme(QLatin1String("dk-smugmug")),
                             map[WSSettings::WebService::SMUGMUG],
                             WSSettings::WebService::SMUGMUG);
        d->wsOption->addItem(QIcon::fromTheme(QLatin1String("dk-googlephoto")),
                             map[WSSettings::WebService::GPHOTO],
                             WSSettings::WebService::GPHOTO);
    }
}

void WSIntroPage::slotWebServiceOptionChanged(const QString& serviceName)
{
    d->accountOption->clear();

    // An empty option is added to accounts, so that user can choose to login with new account
    QStringList accounts = QStringList(QString())
                           << d->settings->allUserNames(serviceName);

    foreach(const QString& account, accounts)
    {
        d->accountOption->addItem(account);
    }
}

void WSIntroPage::initializePage()
{
    d->imageGetOption->setCurrentIndex(d->wizard->settings()->selMode);
}

bool WSIntroPage::validatePage()
{
    d->wizard->settings()->selMode    = (WSSettings::Selection)d->imageGetOption->currentIndex();
    d->wizard->settings()->webService = (WSSettings::WebService)d->wsOption->currentIndex();
    d->wizard->settings()->userName   = d->accountOption->currentText();

    return true;
}

} // namespace Digikam
