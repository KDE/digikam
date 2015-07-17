/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs for file systems jobs
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

// KDCraw includes

#include <KDCRAW/RActionJob>

using namespace KDcrawIface;

namespace Digikam
{

class ImageInfo;

class IOJob : public RActionJob
{
    Q_OBJECT

protected:

    IOJob();

Q_SIGNALS:

    void error(const QString& errMsg);
};

// ---------------------------------------

class CopyJob : public IOJob
{
    Q_OBJECT

public:

    CopyJob(const QUrl& src, const QUrl& dest, bool isMove);

protected:

    void run();
    bool copyFolderRecursively(const QString& srcPath, const QString& dstPath);

private:

    QUrl m_src;
    QUrl m_dest;
    bool m_isMove;
};

// ---------------------------------------

class DeleteJob : public IOJob
{
    Q_OBJECT

public:

    DeleteJob(const QUrl& srcToDelete, bool useTrash);

protected:

    void run();

private:

    QUrl m_srcToDelete;
    bool m_useTrash;
};

// ---------------------------------------

class RenameFileJob : public IOJob
{
    Q_OBJECT

public:

    RenameFileJob(const QUrl& srcToRename, const QUrl& newName);

Q_SIGNALS:

    void signalRenamed(const QUrl& oldUrl, const QUrl& newUrl);

protected:

    void run();

private:

    QUrl m_srcToRename;
    QUrl m_newUrl;
};

} // namespace Digikam

#endif // IOJOB_H
