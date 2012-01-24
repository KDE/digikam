/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-23
 * Description : file action progress indicator
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "fileactionprogress.moc"

// KDE includes

#include <kapplication.h>
#include <klocale.h>
#include <kicon.h>
#include <kdebug.h>

// Local includes

namespace Digikam
{

FileActionProgress::FileActionProgress()
    : ProgressItem(0, "FileActionProgress", QString(), QString(), true, true)
{
    ProgressManager::addProgressItem(this);
    /*m_dbProgress     = ProgressManager::createProgressItem(this, "FileActionMngrDBProgressItem",     QString());
    m_writerProgress = ProgressManager::createProgressItem(this, "FileActionMngrWriterProgressItem", QString());
    m_dbProgress->setUsesBusyIndicator(true);
    m_writerProgress->setUsesBusyIndicator(true);
*/    setLabel(i18n("Process Items"));
    setThumbnail(KIcon("digikam").pixmap(22));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));
}

FileActionProgress::~FileActionProgress()
{
}

void FileActionProgress::slotStarted()
{
}

void FileActionProgress::slotProgressValue(float v)
{
    setProgress((int)(v*100.0));
}

void FileActionProgress::slotProgressMessage(const QString& mess)
{
    setStatus(mess);
}

void FileActionProgress::slotCompleted()
{
    emit signalComplete();

    setComplete();
}

void FileActionProgress::slotCancel()
{
    emit signalComplete();

    setComplete();
}

}  // namespace Digikam
