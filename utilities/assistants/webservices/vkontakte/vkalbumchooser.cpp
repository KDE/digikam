/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-19
 * Description : a tool to export images to VKontakte web service
 *
 * Copyright (C) 2011-2015 by Alexander Potashev <aspotashev at gmail dot com>
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

#include <Vkontakte/AlbumListJob>
#include <Vkontakte/CreateAlbumJob>
#include <Vkontakte/EditAlbumJob>
#include <Vkontakte/DeleteAlbumJob>
#include <Vkontakte/VkApi>

// Local includes

#include "vknewalbumdlg.h"

namespace Digikam
{

VKAlbumChooser::VKAlbumChooser(QWidget* const parent,
                                       Vkontakte::VkApi* const vkapi)
    : QGroupBox(i18nc("@title:group Header above controls for managing albums", "Album"), parent)
{
    m_vkapi         = vkapi;
    m_albumToSelect = -1;

    setWhatsThis(i18n("This is the VKontakte album that will be used for the transfer."));
    QVBoxLayout* const albumsBoxLayout = new QVBoxLayout(this);

    m_albumsCombo        = new QComboBox(this);
    m_albumsCombo->setEditable(false);

    m_newAlbumButton     = new QPushButton(QIcon::fromTheme(QString::fromLatin1("list-add")),
                                           i18n("New Album"), this);
    m_newAlbumButton->setToolTip(i18n("Create new VKontakte album"));

    m_reloadAlbumsButton = new QPushButton(QIcon::fromTheme(QString::fromLatin1("view-refresh")),
                                           i18nc("reload albums list", "Reload"), this);
    m_reloadAlbumsButton->setToolTip(i18n("Reload albums list"));

    m_editAlbumButton    = new QToolButton(this);
    m_editAlbumButton->setToolTip(i18n("Edit selected album"));
    m_editAlbumButton->setEnabled(false);
    m_editAlbumButton->setIcon(QIcon::fromTheme(QString::fromLatin1("document-edit")));

    m_deleteAlbumButton  = new QToolButton(this);
    m_deleteAlbumButton->setToolTip(i18n("Delete selected album"));
    m_deleteAlbumButton->setEnabled(false);
    m_deleteAlbumButton->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-delete")));

    QWidget* const currentAlbumWidget           = new QWidget(this);
    QHBoxLayout* const currentAlbumWidgetLayout = new QHBoxLayout(currentAlbumWidget);
    currentAlbumWidgetLayout->setContentsMargins(0, 0, 0, 0);
    currentAlbumWidgetLayout->addWidget(m_albumsCombo);
    currentAlbumWidgetLayout->addWidget(m_editAlbumButton);
    currentAlbumWidgetLayout->addWidget(m_deleteAlbumButton);

    QWidget* const albumButtons           = new QWidget(this);
    QHBoxLayout* const albumButtonsLayout = new QHBoxLayout(albumButtons);
    albumButtonsLayout->setContentsMargins(0, 0, 0, 0);
    albumButtonsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    albumButtonsLayout->addWidget(m_newAlbumButton);
    albumButtonsLayout->addWidget(m_reloadAlbumsButton);

    albumsBoxLayout->addWidget(currentAlbumWidget);
    albumsBoxLayout->addWidget(albumButtons);

    connect(m_newAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotNewAlbumRequest()));

    connect(m_editAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotEditAlbumRequest()));

    connect(m_deleteAlbumButton, SIGNAL(clicked()),
            this, SLOT(slotDeleteAlbumRequest()));

    connect(m_reloadAlbumsButton, SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(m_vkapi, SIGNAL(authenticated()),
            this, SLOT(slotReloadAlbumsRequest()));
}

VKAlbumChooser::~VKAlbumChooser()
{
}

/**
 * @brief Clear the list of albums
 **/
void VKAlbumChooser::clearList()
{
    m_albumsCombo->clear();
}

bool VKAlbumChooser::getCurrentAlbumInfo(VKNewAlbumDlg::AlbumProperties& out)
{
    int index = m_albumsCombo->currentIndex();

    if (index >= 0)
    {
        Vkontakte::AlbumInfo album = m_albums.at(index);
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
    int index = m_albumsCombo->currentIndex();

    if (index >= 0)
    {
        Vkontakte::AlbumInfo album = m_albums.at(index);
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
    m_albumToSelect = aid;

    for (int i = 0; i < m_albums.size(); i ++)
    {
        if (m_albums.at(i).aid() == aid)
        {
            m_albumsCombo->setCurrentIndex(i);
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
        startAlbumCreation(dlg->album());
    }

    delete dlg;
}

void VKAlbumChooser::startAlbumCreation(const VKNewAlbumDlg::AlbumProperties &album)
{
    Vkontakte::CreateAlbumJob* const job = new Vkontakte::CreateAlbumJob(m_vkapi->accessToken(),
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
        m_albumToSelect = job->album().aid();

        startAlbumsReload();
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
        startAlbumEditing(aid, dlg->album());
    }

    delete dlg;
}

void VKAlbumChooser::startAlbumEditing(int aid, const VKNewAlbumDlg::AlbumProperties& album)
{
    // Select the same album again in the combobox later (in "slotAlbumsReloadDone()")
    m_albumToSelect                    = aid;

    Vkontakte::EditAlbumJob* const job = new Vkontakte::EditAlbumJob(m_vkapi->accessToken(),
                                                                     aid, album.title, album.description,
                                                                     album.privacy, album.commentPrivacy);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumEditingDone(KJob*)));

    job->start();
}

void VKAlbumChooser::slotAlbumEditingDone(KJob* kjob)
{
    SLOT_JOB_DONE_INIT(Vkontakte::EditAlbumJob)

    startAlbumsReload();

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
    startAlbumDeletion(aid);
}

void VKAlbumChooser::startAlbumDeletion(int aid)
{
    Vkontakte::DeleteAlbumJob* const job = new Vkontakte::DeleteAlbumJob(m_vkapi->accessToken(), aid);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumDeletionDone(KJob*)));

    job->start();
}

void VKAlbumChooser::slotAlbumDeletionDone(KJob* kjob)
{
    SLOT_JOB_DONE_INIT(Vkontakte::DeleteAlbumJob)

    startAlbumsReload();

    updateBusyStatus(true);
}

//------------------------------

void VKAlbumChooser::slotReloadAlbumsRequest()
{
    updateBusyStatus(true);

    int aid = 0;

    if (getCurrentAlbumId(aid))
    {
        m_albumToSelect = aid;
    }

    startAlbumsReload();
}

void VKAlbumChooser::startAlbumsReload()
{
    updateBusyStatus(true);

    Vkontakte::AlbumListJob* const job = new Vkontakte::AlbumListJob(m_vkapi->accessToken());

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotAlbumsReloadDone(KJob*)));

    job->start();
}

void VKAlbumChooser::slotAlbumsReloadDone(KJob* kjob)
{
    SLOT_JOB_DONE_INIT(Vkontakte::AlbumListJob)

    if (!job) return;

    m_albumsCombo->clear();
    m_albums = job->list();

    foreach (const Vkontakte::AlbumInfo &album, m_albums)
        m_albumsCombo->addItem(QIcon::fromTheme(QString::fromLatin1("folder-image")), album.title());

    if (m_albumToSelect != -1)
    {
        selectAlbum(m_albumToSelect);
        m_albumToSelect = -1;
    }

    m_albumsCombo->setEnabled(true);

    if (!m_albums.empty())
    {
        m_editAlbumButton->setEnabled(true);
        m_deleteAlbumButton->setEnabled(true);
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
