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

#ifndef VK_ALBUM_CHOOSER_H
#define VK_ALBUM_CHOOSER_H

// Qt includes

#include <QString>
#include <QGroupBox>

// LibKvkontakte includes

#include <Vkontakte/albuminfo.h>

// Local includes

#include "vknewalbumdlg.h"

class KJob;

namespace Vkontakte
{
    class VkApi;
}

namespace Digikam
{

class VKAlbumChooser : public QGroupBox
{
    Q_OBJECT

public:

    explicit VKAlbumChooser(QWidget* const parent, Vkontakte::VkApi* const vkapi);
    ~VKAlbumChooser();

public:

    void clearList();

    bool getCurrentAlbumInfo(VKNewAlbumDlg::AlbumProperties& out);
    bool getCurrentAlbumId(int &out);
    void selectAlbum(int aid);

private Q_SLOTS:

    void slotNewAlbumRequest();
    void slotEditAlbumRequest();
    void slotDeleteAlbumRequest();
    void slotReloadAlbumsRequest();

    void slotStartAlbumCreation(const VKNewAlbumDlg::AlbumProperties& album);
    void slotStartAlbumEditing(int aid, const VKNewAlbumDlg::AlbumProperties& album);
    void slotStartAlbumDeletion(int aid);
    void slotStartAlbumsReload();

    void slotAlbumCreationDone(KJob* kjob);
    void slotAlbumEditingDone(KJob* kjob);
    void slotAlbumDeletionDone(KJob* kjob);
    void slotAlbumsReloadDone(KJob* kjob);

private:

    void updateBusyStatus(bool busy);
    void handleVkError(KJob* kjob);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // VK_ALBUM_CHOOSER_H
