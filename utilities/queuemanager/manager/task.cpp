/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions task.
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "task.moc"

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// Qt includes

#include <QFileInfo>

// KDE includes

#include <kde_file.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <threadweaver/ThreadWeaver.h>

// Local includes

#include "config-digikam.h"
#include "dimg.h"
#include "dmetadata.h"
#include "imageinfo.h"
#include "fileactionmngr.h"
#include "batchtool.h"
#include "batchtoolsmanager.h"
#include "fileoperation.h"

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
    : Job(0),
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

void Task::emitActionData(ActionData::ActionStatus st, const QString& mess, const KUrl& dest)
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

    bool       success = false;
    int        index   = 0;
    KUrl       outUrl  = d->tools.m_itemUrl;
    KUrl       workUrl = !d->settings.useOrgAlbum ? d->settings.workingUrl : KUrl(d->tools.m_itemUrl.directory(KUrl::AppendTrailingSlash));
    KUrl       inUrl;
    KUrl::List tmp2del;
    DImg       tmpImage;
    QString    errMsg;

    foreach (const BatchToolSet& set, d->tools.m_toolsList)
    {
        d->tool = BatchToolsManager::instance()->findTool(set.name, set.group)->clone();
        inUrl   = outUrl;
        index   = set.index + 1;

        kDebug() << "Tool : index= " << index
                 << " :: name= "     << set.name
                 << " :: group= "    << set.group
                 << " :: wurl= "     << workUrl;

        d->tool->setImageData(tmpImage);
        d->tool->setInputUrl(inUrl);
        d->tool->setWorkingUrl(workUrl);
        d->tool->setSettings(set.settings);
        d->tool->setIOFileSettings(d->settings.ioFileSettings);
        d->tool->setRawLoadingRules(d->settings.rawLoadingRule);
        d->tool->setRawDecodingSettings(d->settings.rawDecodingSettings);
        d->tool->setResetExifOrientationAllowed(d->settings.exifSetOrientation);
        d->tool->setLastChainedTool(index == d->tools.m_toolsList.count());
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

    foreach (const KUrl& url, tmp2del)
    {
        unlink(QFile::encodeName(url.toLocalFile()));
    }

    // Move processed temp file to target

    KUrl dest = workUrl;
    dest.setFileName(d->tools.m_destFileName);
    QString renameMess;
    QFileInfo fi(dest.toLocalFile());

    if (fi.exists())
    {
        if (d->settings.conflictRule != QueueSettings::OVERWRITE)
        {
            int i          = 0;
            bool fileFound = false;

            do
            {
                QFileInfo nfi(dest.toLocalFile());

                if (!nfi.exists())
                {
                    fileFound = false;
                }
                else
                {
                    i++;
                    dest.setFileName(fi.completeBaseName() + QString("_%1.").arg(i) + fi.completeSuffix());
                    fileFound = true;
                }
            }
            while (fileFound);

            renameMess = i18n("(renamed to %1)", dest.fileName());
        }
        else
        {
            renameMess = i18n("(overwritten)");
        }
    }

    if (!dest.isEmpty())
    {
        if (DMetadata::hasSidecar(outUrl.toLocalFile()))
        {
            if (!FileOperation::localFileRename(d->tools.m_itemUrl.toLocalFile(),
                                               DMetadata::sidecarPath(outUrl.toLocalFile()),
                                               DMetadata::sidecarPath(dest.toLocalFile())))
            {
                emitActionData(ActionData::BatchFailed, i18n("Failed to create sidecar file..."), dest);
            }
        }

        if (!FileOperation::localFileRename(d->tools.m_itemUrl.toLocalFile(), 
                                           outUrl.toLocalFile(),
                                           dest.toLocalFile()))
        {
            emitActionData(ActionData::BatchFailed, i18n("Failed to create file..."), dest);
        }
        else
        {
            // -- Now copy the digiKam attributes from original file to the new file ------------

            // ImageInfo must be tread-safe.
            ImageInfo source = ImageInfo::fromUrl(d->tools.m_itemUrl);
            FileActionMngr::instance()->copyAttributes(source, dest.toLocalFile());

            emitActionData(ActionData::BatchDone, i18n("Item processed successfully %1", renameMess), dest);
        }
    }
    else
    {
        emitActionData(ActionData::BatchFailed, i18n("Failed to create file..."), dest);
    }
}

}  // namespace Digikam
