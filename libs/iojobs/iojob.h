/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs for file systems jobs
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#ifndef IOJOB_H
#define IOJOB_H

// Qt includes

#include <QUrl>

// Local includes

#include "actionthreadbase.h"
#include "dtrashiteminfo.h"
#include "digikam_export.h"
#include "iojobdata.h"

namespace Digikam
{

class ImageInfo;

class DIGIKAM_EXPORT IOJob : public ActionJob
{
    Q_OBJECT

protected:

    IOJob();

Q_SIGNALS:

    void signalOneProccessed(int operation);
    void error(const QString& errMsg);
};

// ---------------------------------------

class DIGIKAM_EXPORT CopyJob : public IOJob
{
    Q_OBJECT

public:

    CopyJob(IOJobData* const data);

protected:

    void run();

private:

    IOJobData* m_data;
};

// ---------------------------------------

class DIGIKAM_EXPORT DeleteJob : public IOJob
{
    Q_OBJECT

public:

    DeleteJob(IOJobData* const data);

protected:

    void run();

private:

    qlonglong getItemFromUrl(const QUrl& url);

private:

    IOJobData* m_data;
};

// ---------------------------------------

class DIGIKAM_EXPORT RenameFileJob : public IOJob
{
    Q_OBJECT

public:

    RenameFileJob(IOJobData* const data);

Q_SIGNALS:

    void signalRenamed(const QUrl& oldUrl);
    void signalRenameFailed(const QUrl& oldUrl);

protected:

    void run();

private:

    IOJobData* m_data;
};

// ----------------------------------------------

class DIGIKAM_EXPORT DTrashItemsListingJob : public IOJob
{
    Q_OBJECT

public:

    DTrashItemsListingJob(const QString& collectionPath);

Q_SIGNALS:

    void trashItemInfo(const DTrashItemInfo& info);

protected:

    void run();

private:

    QString m_collectionPath;
};

// ----------------------------------------------

class DIGIKAM_EXPORT RestoreDTrashItemsJob : public IOJob
{
    Q_OBJECT

public:

    RestoreDTrashItemsJob(const DTrashItemInfoList& infos);

protected:

    void run();

private:

    DTrashItemInfoList m_dtrashItemInfoList;
};

// ----------------------------------------------

class DIGIKAM_EXPORT DeleteDTrashItemsJob : public IOJob
{
    Q_OBJECT

public:

    DeleteDTrashItemsJob(const DTrashItemInfoList& infos);

protected:

    void run();

private:

    DTrashItemInfoList m_dtrashItemInfoList;
};

} // namespace Digikam

#endif // IOJOB_H
