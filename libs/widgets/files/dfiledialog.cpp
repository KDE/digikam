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

#include "dfiledialog.h"

// Qt includes

#include <QApplication>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace Digikam
{

DFileDialog::DFileDialog(QWidget* const parent, Qt::WindowFlags flags)
    : QFileDialog(parent, flags)
{
    setOption(getNativeFileDialogOption());
}

DFileDialog::DFileDialog(QWidget* const parent, const QString& caption,
                                                const QString& directory,
                                                const QString& filter)
    : QFileDialog(parent, caption, directory, filter)
{
    setOption(getNativeFileDialogOption());
}

DFileDialog::~DFileDialog()
{
}

QString DFileDialog::getExistingDirectory(QWidget* const parent, const QString& caption,
                                                                 const QString& dir,
                                                                 Options options)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getExistingDirectory(parent, caption, dir, options);
}

QUrl DFileDialog::getExistingDirectoryUrl(QWidget* const parent, const QString& caption,
                                                                 const QUrl& dir,
                                                                 Options options,
                                                                 const QStringList& supportedSchemes)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getExistingDirectoryUrl(parent, caption, dir, options, supportedSchemes);
}

QString DFileDialog::getOpenFileName(QWidget* const parent, const QString& caption,
                                                            const QString& dir,
                                                            const QString& filter,
                                                            QString* selectedFilter,
                                                            Options options)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
}

QStringList DFileDialog::getOpenFileNames(QWidget* const parent, const QString& caption,
                                                                 const QString& dir,
                                                                 const QString& filter,
                                                                 QString* selectedFilter,
                                                                 Options options)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
}

QUrl DFileDialog::getOpenFileUrl(QWidget* const parent, const QString& caption,
                                                        const QUrl& dir,
                                                        const QString& filter,
                                                        QString* selectedFilter,
                                                        Options options,
                                                        const QStringList& supportedSchemes)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getOpenFileUrl(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);
}

QList<QUrl> DFileDialog::getOpenFileUrls(QWidget* const parent, const QString& caption,
                                                                const QUrl& dir,
                                                                const QString& filter,
                                                                QString* selectedFilter,
                                                                Options options,
                                                                const QStringList& supportedSchemes)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getOpenFileUrls(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);
}

QString DFileDialog::getSaveFileName(QWidget* const parent, const QString& caption,
                                                            const QString& dir,
                                                            const QString& filter,
                                                            QString* selectedFilter,
                                                            Options options)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
}

QUrl DFileDialog::getSaveFileUrl(QWidget* const parent, const QString& caption,
                                                        const QUrl& dir,
                                                        const QString& filter,
                                                        QString* selectedFilter,
                                                        Options options,
                                                        const QStringList& supportedSchemes)
{
    options |= getNativeFileDialogOption();
    return QFileDialog::getSaveFileUrl(parent, caption, dir, filter, selectedFilter, options, supportedSchemes);
}

QFileDialog::Option DFileDialog::getNativeFileDialogOption()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group;

    if (qApp->applicationName() == QLatin1String("digikam"))
    {
        group = config->group(QLatin1String("General Settings"));
    }
    else
    {
        group = config->group(QLatin1String("ImageViewer Settings"));
    }

    bool useNativeFileDialog  = group.readEntry(QLatin1String("Use Native File Dialog"), false);

    if (useNativeFileDialog)
        return (QFileDialog::Option)0;

    return QFileDialog::DontUseNativeDialog;
}

} // namespace Digikam
