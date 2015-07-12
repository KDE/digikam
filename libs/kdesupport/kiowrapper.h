/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-10
 * Description : A wrapper to isolate KIO Jobs calls
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

#ifndef KIOWRAPPER_H
#define KIOWRAPPER_H

// Qt includes

#include <QObject>

// KDE includes

#include <kio/previewjob.h>
#include <KIOWidgets/kio/renamedialog.h>

class KJob;

namespace Digikam
{

class KIOWrapper : public QObject
{
    Q_OBJECT

public:

    enum RenameDlgResults
    {
        Cancel       = KIO::Result_Cancel,
        Skip         = KIO::Result_Skip,
        SkipAll      = KIO::Result_AutoSkip,
        Overwrite    = KIO::Result_Overwrite,
        OverwriteAll = KIO::Result_OverwriteAll
    };

public:

    KIOWrapper();

    static KIOWrapper* instance();

    QUrl mostLocalUrl(const QUrl& url);
    QUrl upUrl(const QUrl& url);

    bool fileCopy(const QUrl& src, const QUrl& dest, bool withKJobWidget = false, QWidget* const widget = 0);
    bool fileMove(const QUrl& src, const QUrl& dest);
    bool fileDelete(const QUrl &url);
    bool mkdir(const QUrl& url, bool withKJobWidget = false, QWidget* const widget = 0);
    bool rename(const QUrl& oldUrl, const QUrl& newUrl);

    void move(const QUrl& src, const QUrl& dest);
    void del(const QUrl& url);
    void trash(const QUrl& url);

    QString convertSizeFromKiB(quint64 KbSize);

    QStringList previewJobAvailablePlugins();
    void filePreview(const QList<QUrl> &urlList, const QSize &size, const QStringList* const enabledPlugins = 0);

    QPair<int, QString> renameDlg(QWidget* const widget, const QString& caption, const QUrl& src, const QUrl& dest);

Q_SIGNALS:

    void error(const QString& errMsg);

    void gotPreview(const QUrl& itemUrl, const QPixmap& pix);
    void previewJobFinished();
    void previewJobFailed(QUrl);

private Q_SLOTS:

    void kioJobResult(KJob* job);

    void gotPreview(const KFileItem& item, const QPixmap& pix);
    void previewJobFailed(const KFileItem& item);

private:

    friend class KIOWrapperCreator;
};

} // namespace Digikam

#endif // KIOWRAPPER_H
