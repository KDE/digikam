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

FileActionProgress::FileActionProgress(const char* name)
    : ProgressItem(0, name, QString(), QString(), true, true)
{
    ProgressManager::addProgressItem(this);
    setLabel(i18n("Process Items"));
    setThumbnail(KIcon("digikam").pixmap(22));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));
}

FileActionProgress::~FileActionProgress()
{
}

void FileActionProgress::slotProgressValue(float v)
{
    setProgress((int)(v*100.0));
}

void FileActionProgress::slotProgressStatus(const QString& st)
{
    setStatus(st);
}

void FileActionProgress::slotCompleted()
{
    emit signalComplete();

    setComplete();
}

void FileActionProgress::slotCancel()
{
    setComplete();
}

}  // namespace Digikam
