/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * date        : 2017-07-04
 * Description : wrapper for the QFileDialog
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DFILE_DIALOG_H
#define DFILE_DIALOG_H

// Qt includes

#include <QFileDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DFileDialog : public QFileDialog
{
    Q_OBJECT

public:

    explicit DFileDialog(QWidget* const parent, Qt::WindowFlags flags);
    explicit DFileDialog(QWidget* const parent = 0, const QString& caption = QString(),
                                                    const QString& directory = QString(),
                                                    const QString& filter = QString());
    virtual ~DFileDialog();

    static QString getExistingDirectory(QWidget* const parent = 0, const QString& caption = QString(),
                                                                   const QString& dir = QString(),
                                                                   Options options = ShowDirsOnly);

    static QUrl getExistingDirectoryUrl(QWidget* const parent = 0, const QString& caption = QString(),
                                                                   const QUrl& dir = QUrl(),
                                                                   Options options = ShowDirsOnly,
                                                                   const QStringList& supportedSchemes = QStringList());

    static QString getOpenFileName(QWidget* const parent = 0, const QString& caption = QString(),
                                                              const QString& dir = QString(),
                                                              const QString& filter = QString(),
                                                              QString* selectedFilter = 0,
                                                              Options options = Options());

    static QStringList getOpenFileNames(QWidget* const parent = 0, const QString& caption = QString(),
                                                                   const QString& dir = QString(),
                                                                   const QString& filter = QString(),
                                                                   QString* selectedFilter = 0,
                                                                   Options options = Options());

    static QUrl getOpenFileUrl(QWidget* const parent = 0, const QString& caption = QString(),
                                                          const QUrl& dir = QUrl(),
                                                          const QString& filter = QString(),
                                                          QString* selectedFilter = 0,
                                                          Options options = Options(),
                                                          const QStringList& supportedSchemes = QStringList());

    static QList<QUrl> getOpenFileUrls(QWidget* const parent = 0, const QString& caption = QString(),
                                                                  const QUrl& dir = QUrl(),
                                                                  const QString& filter = QString(),
                                                                  QString* selectedFilter = 0,
                                                                  Options options = Options(),
                                                                  const QStringList& supportedSchemes = QStringList());

    static QString getSaveFileName(QWidget* const parent = 0, const QString& caption = QString(),
                                                              const QString& dir = QString(),
                                                              const QString& filter = QString(),
                                                              QString* selectedFilter = 0,
                                                              Options options = Options());

    static QUrl getSaveFileUrl(QWidget* const parent = 0, const QString& caption = QString(),
                                                          const QUrl& dir = QUrl(),
                                                          const QString& filter = QString(),
                                                          QString* selectedFilter = 0,
                                                          Options options = Options(),
                                                          const QStringList& supportedSchemes = QStringList());

private:

    static QFileDialog::Option getNativeFileDialogOption();
};

} // namespace Digikam

#endif // DFILE_DIALOG_H
