/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : file action manager
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2011-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "fileactionmngr.moc"
#include "fileactionmngr_p.h"

// Qt includes

#include <QMutexLocker>
#include <QPointer>

// KDE includes

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kprogressdialog.h>

// Local includes

#include "albumsettings.h"
#include "imageinfotasksplitter.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metadatasettings.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class FileActionMngrCreator
{
public:

    FileActionMngr object;
};

K_GLOBAL_STATIC(FileActionMngrCreator, metadataManagercreator)

FileActionMngr* FileActionMngr::instance()
{
    return &metadataManagercreator->object;
}

FileActionMngr::FileActionMngr()
    : d(new FileActionMngrPriv(this))
{
    connect(d, SIGNAL(signalProgressMessageChanged(QString)),
            this, SIGNAL(signalProgressMessageChanged(QString)));

    connect(d, SIGNAL(signalProgressValueChanged(float)),
            this, SIGNAL(signalProgressValueChanged(float)));

    connect(d, SIGNAL(signalProgressFinished()),
            this, SIGNAL(signalProgressFinished()));

    connect(d->fileWorker, SIGNAL(imageChangeFailed(QString, QStringList)),
            this, SIGNAL(signalImageChangeFailed(QString, QStringList)));
}

FileActionMngr::~FileActionMngr()
{
    shutDown();
    delete d;
}

bool FileActionMngr::requestShutDown()
{
    if (!isActive())
    {
        return true;
    }

    QPointer<KProgressDialog> dialog = new KProgressDialog;
    dialog->setAllowCancel(true);
    dialog->setMinimumDuration(100);
    dialog->setLabelText(i18nc("@label", "Finishing tasks"));

    connect(this, SIGNAL(signalProgressValueChanged(int)),
            dialog->progressBar(), SLOT(setValue(int)));

    connect(this, SIGNAL(signalProgressFinished()),
            dialog, SLOT(accept()));

    d->updateProgress();

    dialog->exec();
    // Either, we finished and all is fine, or the user cancelled and we kill
    shutDown();
    return true;
}

void FileActionMngr::shutDown()
{
    d->dbWorker->deactivate();
    d->fileWorker->deactivate();
    d->dbWorker->wait();
    d->fileWorker->wait();
}

bool FileActionMngr::isActive()
{
    return d->dbTodo || d->writerTodo;
}

void FileActionMngr::assignTags(const QList<qlonglong>& ids, const QList<int>& tagIDs)
{
    assignTags(ImageInfoList(ids), tagIDs);
}

void FileActionMngr::assignTag(const ImageInfo& info, int tagID)
{
    assignTags(QList<ImageInfo>() << info, QList<int>() << tagID);
}

void FileActionMngr::assignTag(const QList<ImageInfo>& infos, int tagID)
{
    assignTags(infos, QList<int>() << tagID);
}

void FileActionMngr::assignTags(const ImageInfo& info, const QList<int>& tagIDs)
{
    assignTags(QList<ImageInfo>() << info, tagIDs);
}

void FileActionMngr::assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->assignTags(infos, tagIDs);
}

void FileActionMngr::removeTag(const ImageInfo& info, int tagID)
{
    removeTags(QList<ImageInfo>() << info, QList<int>() << tagID);
}

void FileActionMngr::removeTag(const QList<ImageInfo>& infos, int tagID)
{
    removeTags(infos, QList<int>() << tagID);
}

void FileActionMngr::removeTags(const ImageInfo& info, const QList<int>& tagIDs)
{
    removeTags(QList<ImageInfo>() << info, tagIDs);
}

void FileActionMngr::removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->removeTags(infos, tagIDs);
}

void FileActionMngr::assignPickLabel(const ImageInfo& info, int pickId)
{
    assignPickLabel(QList<ImageInfo>() << info, pickId);
}

void FileActionMngr::assignColorLabel(const ImageInfo& info, int colorId)
{
    assignColorLabel(QList<ImageInfo>() << info, colorId);
}

void FileActionMngr::assignPickLabel(const QList<ImageInfo>& infos, int pickId)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->assignPickLabel(infos, pickId);
}

void FileActionMngr::assignColorLabel(const QList<ImageInfo>& infos, int colorId)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->assignColorLabel(infos, colorId);
}

void FileActionMngr::assignRating(const ImageInfo& info, int rating)
{
    assignRating(QList<ImageInfo>() << info, rating);
}

void FileActionMngr::assignRating(const QList<ImageInfo>& infos, int rating)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->assignRating(infos, rating);
}

void FileActionMngr::addToGroup(const ImageInfo& pick, const QList<ImageInfo>& infos)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->editGroup(AddToGroup, pick, infos);
}

void FileActionMngr::removeFromGroup(const ImageInfo& info)
{
    removeFromGroup(QList<ImageInfo>() << info);
}

void FileActionMngr::removeFromGroup(const QList<ImageInfo>& infos)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->editGroup(RemoveFromGroup, ImageInfo(), infos);
}

void FileActionMngr::ungroup(const ImageInfo& info)
{
    ungroup(QList<ImageInfo>() << info);
}

void FileActionMngr::ungroup(const QList<ImageInfo>& infos)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->editGroup(Ungroup, ImageInfo(), infos);
}

void FileActionMngr::setExifOrientation(const QList<ImageInfo>& infos, int orientation)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->setExifOrientation(infos, orientation);
}

void FileActionMngr::applyMetadata(const QList<ImageInfo>& infos, const MetadataHub& hub)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->applyMetadata(infos, new MetadataHubOnTheRoad(hub, this));
}

void FileActionMngr::applyMetadata(const QList<ImageInfo>& infos, const MetadataHubOnTheRoad& hub)
{
    emit signalProgressScheduled();
    d->schedulingForDB(infos.size());
    d->applyMetadata(infos, new MetadataHubOnTheRoad(hub, this));
}

void FileActionMngr::transform(const QList<ImageInfo>& infos, KExiv2Iface::RotationMatrix::TransformationAction action)
{
    emit signalProgressScheduled();
    d->schedulingForWrite(infos.size());
    for (ImageInfoTaskSplitter splitter(infos); splitter.hasNext();)
        d->transform(splitter.next(), action);
}

} // namespace Digikam
