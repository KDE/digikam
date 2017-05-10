/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * date        : 2014-09-12
 * Description : Simple helpher widgets collection
 *
 * Copyright (C) 2014-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dfileselector.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DFileSelector::Private
{
public:

    Private()
    {
        edit      = 0;
        btn       = 0;
        fdMode    = QFileDialog::ExistingFile;
        fdOptions = -1;
    }

    QLineEdit*            edit;
    QPushButton*          btn;

    QFileDialog::FileMode fdMode;
    QString               fdFilter;
    QString               fdTitle;
    int                   fdOptions;
};

DFileSelector::DFileSelector(QWidget* const parent)
    : DHBox(parent),
      d(new Private)
{
    d->edit    = new QLineEdit(this);
    d->btn     = new QPushButton(i18n("Browse..."), this);
    setStretchFactor(d->edit, 10);

    connect(d->btn, SIGNAL(clicked()),
            this, SLOT(slotBtnClicked()));
}

DFileSelector::~DFileSelector()
{
    delete d;
}

QLineEdit* DFileSelector::lineEdit() const
{
    return d->edit;
}

void DFileSelector::setFileDlgPath(const QString& path)
{
    d->edit->setText(QDir::toNativeSeparators(path));
}

QString DFileSelector::fileDlgPath() const
{
    return QDir::fromNativeSeparators(d->edit->text());
}

void DFileSelector::setFileDlgMode(QFileDialog::FileMode mode)
{
    d->fdMode = mode;
}

void DFileSelector::setFileDlgFilter(const QString& filter)
{
    d->fdFilter = filter;
}

void DFileSelector::setFileDlgTitle(const QString& title)
{
    d->fdTitle = title;
}

void DFileSelector::setFileDlgOptions(QFileDialog::Options opts)
{
    d->fdOptions = (int)opts;
}

void DFileSelector::slotBtnClicked()
{
    if (d->fdMode == QFileDialog::ExistingFiles)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Multiple selection is not supported";
        return;
    }

    QFileDialog* const fileDlg = new QFileDialog;

    if (d->fdOptions != -1)
    {
        fileDlg->setOptions((QFileDialog::Options)d->fdOptions);
    }

    fileDlg->setDirectory(QFileInfo(fileDlgPath()).filePath());
    fileDlg->setFileMode(d->fdMode);

    if (!d->fdFilter.isNull())
    {
        fileDlg->setNameFilter(d->fdFilter);
    }

    if (!d->fdTitle.isNull())
    {
        fileDlg->setWindowTitle(d->fdTitle);
    }

    emit signalOpenFileDialog();

    if (fileDlg->exec() == QDialog::Accepted)
    {
        QStringList sel = fileDlg->selectedFiles();

        if (!sel.isEmpty())
        {
            setFileDlgPath(sel.first());
            emit signalUrlSelected(QUrl::fromLocalFile(sel.first()));
        }
    }

    delete fileDlg;
}

} // namespace Digikam
