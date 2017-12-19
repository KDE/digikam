/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media Server configuration dialog to share a single list of files
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QIcon>
#include <QCheckBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dinfointerface.h"
#include "dimageslist.h"
#include "dmediaservermngr.h"
#include "dxmlguiwindow.h"
#include "workingwidget.h"

namespace Digikam
{

class DMediaServerDlg::Private
{
public:

    Private() :
        dirty(false),
        mngr(DMediaServerMngr::instance()),
        srvButton(0),
        srvStatus(0),
        progress(0),
        aStats(0),
        separator(0),
        iStats(0),
        startOnStartup(0),
        albumSupport(false),
        albumSelector(0),
        listView(0),
        iface(0),
        page(0),
        buttons(0)
    {
    }

    bool                dirty;
    DMediaServerMngr*   mngr;
    QPushButton*        srvButton;
    QLabel*             srvStatus;
    WorkingWidget*      progress;
    QLabel*             aStats;
    QLabel*             separator;
    QLabel*             iStats;
    QCheckBox*          startOnStartup;
    bool                albumSupport;
    QWidget*            albumSelector;
    DImagesList*        listView;
    DInfoInterface*     iface;
    QWidget*            page;
    QDialogButtonBox*   buttons;
};

DMediaServerDlg::DMediaServerDlg(QObject* const /*parent*/,
                                 DInfoInterface* const iface)
    : QDialog(),
      d(new Private)
{
    setWindowTitle(i18nc("@title:window", "Share Files with DLNA Media Server"));

    d->iface                 = iface;
    d->buttons               = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->page                  = new QWidget(this);
    QVBoxLayout* const vbx   = new QVBoxLayout(this);
    vbx->addWidget(d->page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
    setModal(false);

    // -------------------

    QGridLayout* const grid = new QGridLayout(d->page);
    d->albumSupport         = (d->iface && d->iface->supportAlbums());

    if (d->albumSupport)
    {
        d->albumSelector = d->iface->albumChooser(this);
        grid->addWidget(d->albumSelector, 0, 0, 1, 6);

        connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SLOT(slotSelectionChanged()));
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
        grid->addWidget(d->listView, 0, 0, 1, 6);

        connect(d->listView, SIGNAL(signalImageListChanged()),
                this, SLOT(slotSelectionChanged()));
    }

    // -------------------

    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->startOnStartup       = new QCheckBox(i18n("Start Server at Startup"));
    d->startOnStartup->setWhatsThis(i18n("Set this option to turn-on the DLNA server at application start-up automatically"));
    d->startOnStartup->setChecked(true);

    d->srvButton            = new QPushButton(this);
    d->srvStatus            = new QLabel(this);
    d->progress             = new WorkingWidget(this);
    d->aStats               = new QLabel(this);
    d->separator            = new QLabel(QLatin1String(" / "), this);
    d->iStats               = new QLabel(this);

    QLabel* const explanation = new QLabel(this);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    explanation->setFrameStyle(QFrame::Box | QFrame::Plain);
    QString txt;

    explanation->setText(i18n("The media server permit to share items through the local network "
                              "using <a href='https://en.wikipedia.org/wiki/Digital_Living_Network_Alliance'>DLNA</a> "
                              "standard and <a href='https://en.wikipedia.org/wiki/Universal_Plug_and_Play'>UPNP</a> "
                              "protocol. Many kind of electronic devices can support DLNA, as tablets, cellulars, TV, etc."
                              "<br>Note: depending of the network features and the configuration, "
                              "the delay to discover the server on client devices can take a while."));

    grid->addWidget(d->startOnStartup, 1, 0, 1, 6);
    grid->addWidget(d->srvButton,      2, 0, 1, 1);
    grid->addWidget(d->srvStatus,      2, 1, 1, 1);
    grid->addWidget(d->aStats,         2, 2, 1, 1);
    grid->addWidget(d->separator,      2, 3, 1, 1);
    grid->addWidget(d->iStats,         2, 4, 1, 1);
    grid->addWidget(d->progress,       2, 5, 1, 1);
    grid->addWidget(explanation,       3, 0, 1, 6);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(0, 10);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->srvButton, SIGNAL(clicked()),
            this, SLOT(slotToggleMediaServer()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &DMediaServerDlg::reject);

    connect(d->buttons->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &DMediaServerDlg::accept);

    // -------------------

    readSettings();
}

DMediaServerDlg::~DMediaServerDlg()
{
    delete d;
}

void DMediaServerDlg::accept()
{
    if (d->dirty)
    {
        bool empty = false;

        if (d->albumSupport)
        {
            empty = d->iface->albumChooserItems().isEmpty();
        }
        else
        {
            empty = d->listView->imageUrls().isEmpty();
        }

        if (!empty)
        {
            int rc = QMessageBox::question(this, i18n("Media Server Contents"),
                                           i18n("The items list to share has changed. "
                                                "Do you want to start now the media server with this contents?"));
            if (rc == QMessageBox::Yes)
            {
                startMediaServer();
            }
        }
    }

    saveSettings();
    QDialog::accept();
}

void DMediaServerDlg::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());

    d->startOnStartup->setChecked(group.readEntry(d->mngr->configStartServerOnStartupEntry(), false));

    winId();
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    updateServerStatus();
}

void DMediaServerDlg::saveSettings()
{
    setMediaServerContents();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->mngr->configGroupName());
    group.writeEntry(d->mngr->configStartServerOnStartupEntry(), d->startOnStartup->isChecked());
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
    config->sync();
}

void DMediaServerDlg::updateServerStatus()
{
    if (d->mngr->isRunning())
    {
        d->srvStatus->setText(i18n("Server is running"));
        d->aStats->setText(i18np("1 album shared", "%1 albums shared", d->mngr->albumsShared()));
        d->separator->setVisible(true);
        d->iStats->setText(i18np("1 item shared",  "%1 items shared",  d->mngr->itemsShared()));
        d->srvButton->setText(i18n("Stop"));
        d->srvButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-stop")));
        d->progress->toggleTimer(true);
        d->progress->setVisible(true);
    }
    else
    {
        d->srvStatus->setText(i18n("Server is not running"));
        d->aStats->clear();
        d->separator->setVisible(false);
        d->iStats->clear();
        d->srvButton->setText(i18n("Start"));
        d->srvButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
        d->progress->toggleTimer(false);
        d->progress->setVisible(false);
    }
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

void DMediaServerDlg::startMediaServer()
{
    if (d->dirty)
        d->dirty = false;

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

    updateServerStatus();
}

void DMediaServerDlg::slotSelectionChanged()
{
    d->dirty = true;
}

void DMediaServerDlg::slotToggleMediaServer()
{
    if (!d->mngr->isRunning())
    {
        startMediaServer();
    }
    else
    {
        d->mngr->cleanUp();
        updateServerStatus();
    }
}

} // namespace Digikam
