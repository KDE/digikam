/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions task.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Pankaj Kumar <me at panks dot me>
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

#include "task.h"

// Qt includes

#include <QFileInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dimg.h"
#include "dmetadata.h"
#include "imageinfo.h"
#include "batchtool.h"
#include "batchtoolsmanager.h"
#include "dfileoperations.h"

namespace Digikam
{

class Task::Private
{
public:

    Private()
    {
        cancel = false;
        tool   = 0;
    }

    bool               cancel;

    BatchTool*         tool;

    QueueSettings      settings;
    AssignedBatchTools tools;
};

// -------------------------------------------------------

Task::Task()
    : ActionJob(),
      d(new Private)
{
}

Task::~Task()
{
    slotCancel();
    delete d;
}

void Task::setSettings(const QueueSettings& settings)
{
    d->settings = settings;
}

void Task::setItem(const AssignedBatchTools& tools)
{
    d->tools = tools;
}

void Task::slotCancel()
{
    if (d->tool)
        d->tool->cancel();

    d->cancel = true;
}

void Task::emitActionData(ActionData::ActionStatus st, const QString& mess, const QUrl& dest)
{
    ActionData ad;
    ad.fileUrl = d->tools.m_itemUrl;
    ad.status  = st;
    ad.message = mess;
    ad.destUrl = dest;
    emit signalFinished(ad);
}

void Task::run()
{
    if (d->cancel)
    {
        return;
    }

    emitActionData(ActionData::BatchStarted);

    // Loop with all batch tools operations to apply on item.

    bool        success = false;
    int         index   = 0;
    QUrl        outUrl  = d->tools.m_itemUrl;
    QUrl        workUrl = !d->settings.useOrgAlbum ? d->settings.workingUrl
                                                   : d->tools.m_itemUrl.adjusted(QUrl::RemoveFilename);
    QUrl        inUrl;
    QList<QUrl> tmp2del;
    DImg        tmpImage;
    QString     errMsg;

    // ImageInfo must be tread-safe.
    ImageInfo source = ImageInfo::fromUrl(d->tools.m_itemUrl);
    bool timeAdjust  = false;

    foreach (const BatchToolSet& set, d->tools.m_toolsList)
    {
        d->tool     = BatchToolsManager::instance()->findTool(set.name, set.group)->clone();
        timeAdjust |= (set.name == QLatin1String("TimeAdjust"));
        inUrl       = outUrl;
        index       = set.index + 1;

        qCDebug(DIGIKAM_GENERAL_LOG) << "Tool : index= " << index
                 << " :: name= "     << set.name
                 << " :: group= "    << set.group
                 << " :: wurl= "     << workUrl;

        d->tool->setImageData(tmpImage);
        d->tool->setImageInfo(source);
        d->tool->setInputUrl(inUrl);
        d->tool->setWorkingUrl(workUrl);
        d->tool->setSettings(set.settings);
        d->tool->setIOFileSettings(d->settings.ioFileSettings);
        d->tool->setRawLoadingRules(d->settings.rawLoadingRule);
        d->tool->setDRawDecoderSettings(d->settings.rawDecodingSettings);
        d->tool->setResetExifOrientationAllowed(d->settings.exifSetOrientation);

        if (index == d->tools.m_toolsList.count())
        {
            d->tool->setLastChainedTool(true);
        }
        // If the next tool is under the custom group (user script)
        // treat as the last chained tool, i.e. save image to file
        else if (d->tools.m_toolsList[index].group == BatchTool::CustomTool)
        {
            d->tool->setLastChainedTool(true);
        }
        else
        {
            d->tool->setLastChainedTool(false);
        }

        d->tool->setOutputUrlFromInputUrl();
        d->tool->setBranchHistory(true);

        outUrl   = d->tool->outputUrl();
        success  = d->tool->apply();
        tmpImage = d->tool->imageData();
        errMsg   = d->tool->errorDescription();
        tmp2del.append(outUrl);

        if (d->cancel)
        {
            emitActionData(ActionData::BatchCanceled);
            emit signalDone();
            delete d->tool;
            d->tool = 0;
            return;
        }
        else if (!success)
        {
            emitActionData(ActionData::BatchFailed, errMsg);
            break;
        }

        delete d->tool;
        d->tool = 0;
    }

    // Clean up all tmp url.

    // We don't remove last output tmp url.
    tmp2del.removeAll(outUrl);

    foreach (const QUrl& url, tmp2del)
    {
        QString tmpPath(url.toLocalFile());
        QFile::remove(tmpPath);

        tmpPath = DMetadata::sidecarPath(tmpPath);

        if (QFile::exists(tmpPath))
            QFile::remove(tmpPath);
    }

    // Move processed temp file to target

    QUrl dest = workUrl.adjusted(QUrl::RemoveFilename);
    dest.setPath(dest.path() + d->tools.m_destFileName);
    QString renameMess;

    if (QFileInfo::exists(dest.toLocalFile()))
    {
        if (d->settings.conflictRule != FileSaveConflictBox::OVERWRITE)
        {
            dest       = DFileOperations::getUniqueFileUrl(dest);
            renameMess = i18n("(renamed to %1)", dest.fileName());
        }
        else
        {
            renameMess = i18n("(overwritten)");
        }
    }

    if (QFileInfo(outUrl.toLocalFile()).size() == 0)
    {
        QFile::remove(outUrl.toLocalFile());
        dest.clear();
    }

    if (!dest.isEmpty())
    {
        if (DMetadata::hasSidecar(outUrl.toLocalFile()))
        {
            if (!DFileOperations::localFileRename(d->tools.m_itemUrl.toLocalFile(),
                                                  DMetadata::sidecarPath(outUrl.toLocalFile()),
                                                  DMetadata::sidecarPath(dest.toLocalFile()),
                                                  timeAdjust))
            {
                emitActionData(ActionData::BatchFailed, i18n("Failed to create sidecar file..."), dest);
            }
        }

        if (DFileOperations::localFileRename(d->tools.m_itemUrl.toLocalFile(),
                                             outUrl.toLocalFile(),
                                             dest.toLocalFile(),
                                             timeAdjust))
        {
            emitActionData(ActionData::BatchDone, i18n("Item processed successfully %1", renameMess), dest);
        }
        else
        {
            emitActionData(ActionData::BatchFailed, i18n("Failed to create file..."), dest);
        }
    }
    else
    {
        emitActionData(ActionData::BatchFailed, i18n("Failed to create file..."), dest);
    }

    emit signalDone();
}

}  // namespace Digikam
