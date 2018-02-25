/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-19
 * Description : a tool to export images to VKontakte web service
 *
 * Copyright (C) 2011-2015 by Alexander Potashev <aspotashev at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "vkalbumchooser.h"

// Qt includes

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// LibKvkontakte includes

#include <Vkontakte/albumlistjob.h>
#include <Vkontakte/createalbumjob.h>
#include <Vkontakte/editalbumjob.h>
#include <Vkontakte/deletealbumjob.h>
#include <Vkontakte/vkapi.h>

// Local includes

#include "vknewalbumdlg.h"

namespace Digikam
{

class VKAlbumChooser::Private
{
public:

    explicit Private()
    {
        albumsCombo        = 0;
        newAlbumButton     = 0;
        reloadAlbumsButton = 0;
        editAlbumButton    = 0;
        deleteAlbumButton  = 0;
        albumToSelect      = -1;
        vkapi              = 0;
    }

    QComboBox*                  albumsCombo;
    QPushButton*                newAlbumButton;
    QPushButton*                reloadAlbumsButton;
    QToolButton*                editAlbumButton;
    QToolButton*                deleteAlbumButton;

    QList<Vkontakte::AlbumInfo> albums;

    /** Album with this "aid" will
     *  be selected in slotAlbumsReloadDone()
     */
    int                         albumToSelect;

    Vkontakte::VkApi*           vkapi;
};

VKAlbumChooser::VKAlbumChooser(QWidget* const parent,
                               Vkontakte::VkApi* const vkapi)
    : QGroupBox(i18nc("@title:group Header above controls for managing albums", "Album"), parent),
      d(new Private)
{
    d->vkapi              = vkapi;

    setWhatsThis(i18n("This is the VKontakte album that will be used for the transfer."));
    QVBoxLayout* const albumsBoxLayout = new QVBoxLayout(this);

    d->albumsCombo        = new QComboBox(this);
    d->albumsCombo->setEditable(false);

    d->newAlbumButton     = new QPushButton(QIcon::fromTheme(QString::fromLatin1("list-add")),
                                           i18n("New Album"), this);
    d->newAlbumButton->setToolTip(i18n("Create new VKontakte album"));

    d->reloadAlbumsButton = new QPushButton(QIcon::fromTheme(QString::fromLatin1("view-refresh")),
                                           i18nc("reload albums list", "Reload"), this);
    d->reloadAlbumsButton->setToolTip(i18n("Reload albums list"));

    d->editAlbumButton    = new QToolButton(this);
    d->editAlbumButton->setToolTip(i18n("Edit selected album"));
    d->editAlbumButton->setEnabled(false);
    d->editAlbumButton->setIcon(QIcon::fromTheme(QString::fromLatin1("document-edit")));

    d->deleteAlbumButton  = new QToolButton(this);
    d->deleteAlbumButton->setToolTip(i18n("Delete selected album"));
    d->deleteAlbumButton->setEnabled(false);
    d->deleteAlbumButton->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-delete")));

    QWidget* const currentAlbumWidget           = new QWidget(this);
    QHBoxLayout* const currentAlbumWidgetLayout = new QHBoxLayout(currentAlbumWidget);
    currentAlbumWidgetLayout->setContentsMargins(0, 0, 0, 0);
    currentAlbumWidgetLayout->addWidget(d->albumsCombo);
    currentAlbumWidgetLayout->addWidget(d->editAlbumButton);
    currentAlbumWidgetLayout->addWidget(d->deleteAlbumButton);

    QWidget* const albumButtons           = new QWidget(this);
    QHBoxLayout* const albumButtonsLayout = new QHBoxLayout(albumButtons);
    albumButtonsLayout->setContentsMargins(0, 0, 0, 0);
    albumButtonsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    albumButtonsLayout->addWidget(d->newAlbumButton);
    albumButtonsLayout->addWidget(d->reloadAlbumsButton);

    albumsBoxLayout->addWidget(currentAlbumWidget);
    albumsBoxLayout->addWidget(albumButtons);

    connect(d->newAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    connect(d->editAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotEditAlbumRequest()));

    connect(d->deleteAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotDeleteAlbumRequest()));

    connect(d->reloadAlbumsButton, SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(d->vkapi, SIGNAL(authenticated()),
            this, SLOT(slotReloadAlbumsRequest()));
}

VKAlbumChooser::~VKAlbumChooser()
{
    delete d;
}

/**
 * @brief Clear the list of albums
 **/
void VKAlbumChooser::clearList()
{
    d->albumsCombo->clear();
}

bool VKAlbumChooser::getCurrentAlbumInfo(VKNewAlbumDlg::AlbumProperties& out)
{
    int index = d->albumsCombo->currentIndex();

    if (index >= 0)
    {
        Vkontakte::AlbumInfo album = d->albums.at(index);
        out.title                  = album.title();
        out.description            = album.description();
        out.privacy                = album.privacy();
        out.commentPrivacy         = album.commentPrivacy();

        return true;
    }
    else
    {
        return false;
    }
}

bool VKAlbumChooser::getCurrentAlbumId(int& out)
{
    int index = d->albumsCombo->currentIndex();

    if (index >= 0)
    {
        Vkontakte::AlbumInfo album = d->albums.at(index);
        out                        = album.aid();

        return true;
    }
    else
    {
        return false;
    }
}

void VKAlbumChooser::selectAlbum(int aid)
{
    /*
     * If the album list is not ready yet, select this album later
     */
    d->albumToSelect = aid;

    for (int i = 0 ; i < d->albums.size() ; i ++)
    {
        if (d->albums.at(i).aid() == aid)
        {
            d->albumsCombo->setCurrentIndex(i);
            break;
        }
    }
}

//------------------------------

void VKAlbumChooser::slotNewAlbumRequest()
{
    QPointer<VKNewAlbumDlg> dlg = new VKNewAlbumDlg(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        updateBusyStatus(true);
        slotStartAlbumCreation(dlg->album());
    }

    delete dlg;
}

void VKAlbumChooser::slotStartAlbumCreation(const VKNewAlbumDlg::AlbumProperties &album)
{
    Vkontakte::CreateAlbumJob* const job = new Vkontakte::CreateAlbumJob(d->vkapi->accessToken(),
                                                                         album.title, album.description,
                                                                         album.privacy, album.commentPrivacy);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumCreationDone(KJob*)));

    job->start();
}

void VKAlbumChooser::slotAlbumCreationDone(KJob* kjob)
{
    Vkontakte::CreateAlbumJob* const job = dynamic_cast<Vkontakte::CreateAlbumJob*>(kjob);
    Q_ASSERT(job);

    if (job == 0 || job->error())
    {
        handleVkError(job);
        updateBusyStatus(false);
    }
    else
    {
        // Select the newly created album in the combobox later (in "slotAlbumsReloadDone()")
        d->albumToSelect = job->album().aid();

        slotStartAlbumsReload();
        updateBusyStatus(true);
    }
}

//------------------------------

void VKAlbumChooser::slotEditAlbumRequest()
{
    VKNewAlbumDlg::AlbumProperties album;
    int aid = 0;

    if (!getCurrentAlbumInfo(album) || !getCurrentAlbumId(aid))
    {
        return;
    }

    QPointer<VKNewAlbumDlg> dlg = new VKNewAlbumDlg(this, album);

    if (dlg->exec() == QDialog::Accepted)
    {
        updateBusyStatus(true);
        slotStartAlbumEditing(aid, dlg->album());
    }

    delete dlg;
}

void VKAlbumChooser::slotStartAlbumEditing(int aid, const VKNewAlbumDlg::AlbumProperties& album)
{
    // Select the same album again in the combobox later (in "slotAlbumsReloadDone()")
    d->albumToSelect                   = aid;

    Vkontakte::EditAlbumJob* const job = new Vkontakte::EditAlbumJob(d->vkapi->accessToken(),
                                                                     aid, album.title, album.description,
                                                                     album.privacy, album.commentPrivacy);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumEditingDone(KJob*)));

    job->start();
}

void VKAlbumChooser::slotAlbumEditingDone(KJob* kjob)
{
    Vkontakte::EditAlbumJob* const job = dynamic_cast<Vkontakte::EditAlbumJob*>(kjob);
    Q_ASSERT(job);

    if (job && job->error())
    {
        handleVkError(job);
        return;
    }

    slotStartAlbumsReload();

    updateBusyStatus(true);
}

//------------------------------

void VKAlbumChooser::slotDeleteAlbumRequest()
{
    VKNewAlbumDlg::AlbumProperties album;
    int aid = 0;

    if (!getCurrentAlbumInfo(album) || !getCurrentAlbumId(aid))
    {
        return;
    }

    if (QMessageBox::question(this, i18nc("@title:window", "Confirm Album Deletion"),
                              i18n("<qt>Are you sure you want to remove the album <b>%1</b> "
                                   "including all photos in it?</qt>", album.title))
            != QMessageBox::Yes)
    {
        return;
    }

    updateBusyStatus(true);
    slotStartAlbumDeletion(aid);
}

void VKAlbumChooser::slotStartAlbumDeletion(int aid)
{
    Vkontakte::DeleteAlbumJob* const job = new Vkontakte::DeleteAlbumJob(d->vkapi->accessToken(), aid);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumDeletionDone(KJob*)));

    job->start();
}

void VKAlbumChooser::slotAlbumDeletionDone(KJob* kjob)
{
    Vkontakte::DeleteAlbumJob* const job = dynamic_cast<Vkontakte::DeleteAlbumJob*>(kjob);
    Q_ASSERT(job);

    if (job && job->error())
    {
        handleVkError(job);
        return;
    }

    slotStartAlbumsReload();

    updateBusyStatus(true);
}

//------------------------------

void VKAlbumChooser::slotReloadAlbumsRequest()
{
    updateBusyStatus(true);

    int aid = 0;

    if (getCurrentAlbumId(aid))
    {
        d->albumToSelect = aid;
    }

    slotStartAlbumsReload();
}

void VKAlbumChooser::slotStartAlbumsReload()
{
    updateBusyStatus(true);

    Vkontakte::AlbumListJob* const job = new Vkontakte::AlbumListJob(d->vkapi->accessToken());

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumsReloadDone(KJob*)));

    job->start();
}

void VKAlbumChooser::slotAlbumsReloadDone(KJob* kjob)
{
    Vkontakte::AlbumListJob* const job = dynamic_cast<Vkontakte::AlbumListJob*>(kjob);
    Q_ASSERT(job);                                      

    if (job && job->error())                            
    {                                                   
        handleVkError(job);                             
        return;                                         
    }

    if (!job)
        return;

    d->albumsCombo->clear();
    d->albums = job->list();

    foreach (const Vkontakte::AlbumInfo &album, d->albums)
        d->albumsCombo->addItem(QIcon::fromTheme(QString::fromLatin1("folder-image")), album.title());

    if (d->albumToSelect != -1)
    {
        selectAlbum(d->albumToSelect);
        d->albumToSelect = -1;
    }

    d->albumsCombo->setEnabled(true);

    if (!d->albums.empty())
    {
        d->editAlbumButton->setEnabled(true);
        d->deleteAlbumButton->setEnabled(true);
    }

    updateBusyStatus(false);
}

//------------------------------

void VKAlbumChooser::updateBusyStatus(bool busy)
{
    setEnabled(!busy);
}

// TODO: share this code with `vkwindow.cpp`
void VKAlbumChooser::handleVkError(KJob* kjob)
{
    QMessageBox::critical(this,
                          i18nc("@title:window", "Request to VKontakte failed"),
                          kjob == 0 ? i18n("Internal error: Null pointer to KJob instance.") : kjob->errorText());
}

} // namespace Digikam
