/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-19
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2011      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "yfnewalbumdlg.h"

// Qt includes

#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QApplication>
#include <QStyle>
#include <QPushButton>
#include <QDebug>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "yfalbum.h"
#include "digikam_debug.h"

namespace Digikam
{

class YFNewAlbumDlg::Private
{
public:

    explicit Private()
    {
        passwordEdit = 0;
    }

    QLineEdit*       passwordEdit;

    YandexFotkiAlbum album;
};

YFNewAlbumDlg::YFNewAlbumDlg(QWidget* const parent, YandexFotkiAlbum& album)
    : WSNewAlbumDialog(parent, QString::fromLatin1("Yandex.Fotki")),
      d(new Private)
{
    d->album = album;

    hideLocation();
    hideDateTime();

    QGroupBox* const albumBox         = new QGroupBox(QString(), this);

    d->passwordEdit                   = new QLineEdit();
    d->passwordEdit->setWhatsThis(i18n("Password for the album (optional)."));

    QFormLayout* const albumBoxLayout = new QFormLayout;
    albumBoxLayout->addRow(i18n("Password:"), d->passwordEdit);

    albumBox->setLayout(albumBoxLayout);
    addToMainLayout(albumBox);

    connect(getButtonBox(), SIGNAL(accepted()),
            this, SLOT(slotOkClicked()));
}

YFNewAlbumDlg::~YFNewAlbumDlg()
{
    delete d;
}

YandexFotkiAlbum& YFNewAlbumDlg::album() const
{
    return d->album;
}

void YFNewAlbumDlg::slotOkClicked()
{
    if (getTitleEdit()->text().isEmpty())
    {
        QMessageBox::critical(this, i18n("Error"), i18n("Title cannot be empty."));
        return;
    }

    d->album.setTitle(getTitleEdit()->text());
    d->album.setSummary(getDescEdit()->toPlainText());

    if (d->passwordEdit->text().isEmpty())
    {
        d->album.setPassword(QString()); // force null string
    }
    else
    {
        d->album.setPassword(d->passwordEdit->text());
    }

    accept();
}

} // namespace Digikam
