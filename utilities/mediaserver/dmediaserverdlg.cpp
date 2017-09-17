/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server configuration dialog to share a single list of files
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Ahmed Fathy <ahmed dot fathi dot abdelmageed at gmail dot com>
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
        albumSupport(false),
        albumSelector(0),
        ctrl(0),
        mngr(DMediaServerMngr::instance()),
        listView(0),
        iface(0),
        page(0),
        buttons(0)
    {
    }

    static const QString configGroupName;
    
    bool                 albumSupport;
    QWidget*             albumSelector;
    DMediaServerCtrl*    ctrl;
    DMediaServerMngr*    mngr;
    DImagesList*         listView;
    DInfoInterface*      iface;
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

    d->iface                 = iface;
    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Close, this);
    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
    setModal(false);

    // -------------------
    
    QGridLayout* const grid = new QGridLayout(d->page);
    d->albumSupport         = (d->iface && d->iface->supportAlbums());
    d->ctrl                 = new DMediaServerCtrl(d->page);

    if (d->albumSupport)
    {
        d->albumSelector = d->iface->albumChooser(this);
        grid->addWidget(d->albumSelector, 0, 0, 1, 1);

        connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                d->ctrl, SLOT(slotSelectionChanged()));
    }
    else
    {
        d->listView = new DImagesList(d->page);
        d->listView->setControlButtonsPlacement(DImagesList::ControlButtonsRight);
        d->listView->setIface(d->iface);

        // Add all items currently loaded in application.
        d->listView->loadImagesFromCurrentSelection();
    
        // Replug the previous shared items list.
        d->listView->slotAddImages(d->mngr->itemsList());
        grid->addWidget(d->listView, 0, 0, 1, 1);

        connect(d->listView, SIGNAL(signalImageListChanged()),
                d->ctrl, SLOT(slotSelectionChanged()));
    }

    grid->addWidget(d->ctrl,  1, 0, 1, 1);
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
    setMediaServerContents();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    delete d;
}

bool DMediaServerDlg::setMediaServerContents()
{
    if (d->albumSupport)
    {
        DInfoInterface::DAlbumIDs albums = d->iface->albumChooserItems();
        MediaServerMap map;

        foreach(int id, albums)
        {
            DAlbumInfo anf(d->iface->albumInfo(id));
            map.insert(anf.title(), d->iface->albumItems(id));
        }

        if (map.isEmpty())
        {
            QMessageBox::information(this, i18n("Media Server Contents"),
                                     i18n("There is no collection to share with the current selection..."));
            return false;
        }

        d->mngr->setCollectionMap(map);
    }
    else
    {
        QList<QUrl> urls = d->listView->imageUrls();

        if (urls.isEmpty())
        {
            QMessageBox::information(this, i18n("Media Server Contents"),
                                     i18n("There is no item to share with the current selection..."));

            return false;
        }

        d->mngr->setItemsList(i18n("Shared Items"), urls);
    }
    
    return true;
}

void DMediaServerDlg::slotStartMediaServer()
{    
    if (!setMediaServerContents())
        return;
    
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
