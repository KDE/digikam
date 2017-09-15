/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server configuration dialog to share a single list of files
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dmediaserverdlg.h"

// Qt includes

#include <QGridLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dinfointerface.h"
#include "dimageslist.h"
#include "dmediaservermngr.h"
#include "dmediaserverctrl.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class DMediaServerDlg::Private
{
public:

    Private() :
        ctrl(0),
        mngr(DMediaServerMngr::instance()),
        listView(0),
        page(0),
        buttons(0)
    {
    }

    static const QString configGroupName;
    
    DMediaServerCtrl*    ctrl;
    DMediaServerMngr*    mngr;
    DImagesList*         listView;
    QWidget*             page;
    QDialogButtonBox*    buttons;
};

const QString DMediaServerDlg::Private::configGroupName(QLatin1String("DMediaServerDlg Settings"));

DMediaServerDlg::DMediaServerDlg(QObject* const /*parent*/,
                                 DInfoInterface* const iface)
    : QDialog(),
      d(new Private)
{
    setWindowTitle(QString::fromUtf8("Share Files With DLNA Media Server"));

    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Close, this);
    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
    setModal(false);

    // -------------------

    QGridLayout* const grid = new QGridLayout(d->page);
    d->listView             = new DImagesList(d->page);
    d->listView->setControlButtonsPlacement(DImagesList::ControlButtonsRight);
    d->listView->setIface(iface);
    
    // Add all items currently loaded in application.
    d->listView->loadImagesFromCurrentSelection();
    
    // Replug the previous shared items list.
    d->listView->slotAddImages(d->mngr->itemsList());
    
    d->ctrl                 = new DMediaServerCtrl(d->page);

    grid->addWidget(d->listView, 0, 0, 1, 1);
    grid->addWidget(d->ctrl,     1, 0, 1, 1);
    grid->setRowStretch(0, 10);

    // -------------------
    
    connect(d->ctrl, SIGNAL(signalStartMediaServer()),
            this, SLOT(slotStartMediaServer()));

    connect(d->buttons->button(QDialogButtonBox::Close), &QPushButton::clicked,
            this, &DMediaServerDlg::close);
    
    // -------------------
    
    d->ctrl->updateServerStatus();
    
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    winId();
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());
}

DMediaServerDlg::~DMediaServerDlg()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    delete d;
}

void DMediaServerDlg::slotStartMediaServer()
{
    QList<QUrl> urls = d->listView->imageUrls();

    if (urls.isEmpty())
    {
        QMessageBox::information(this, i18n("Starting Media Server"),
                                 i18n("There is no items to share with the current selection..."));

        return;
    }

    d->mngr->setItemsList(i18n("Shared Items"), urls);

    if (!d->mngr->startMediaServer())
    {
        QMessageBox::warning(this, i18n("Starting Media Server"),
                             i18n("An error occurs while to start Media Server..."));
    }
    else
    {
        d->mngr->mediaServerNotification(true);
    }

    d->ctrl->updateServerStatus();
}

} // namespace Digikam
