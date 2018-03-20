/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-19
 * Description : a tool to export images to VKontakte web service
 *
 * Copyright (C) 2011      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2011-2015 by Alexander Potashev <aspotashev at gmail dot com>
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

#include "vknewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QMessageBox>
#include <QDialogButtonBox>

// KDE includes

#include <klocalizedstring.h>

// libvkontakte includes

#include <Vkontakte/albuminfo.h>

namespace Digikam
{

class VKNewAlbumDlg::Private
{
public:

    explicit Private()
    {
        titleEdit            = 0;
        summaryEdit          = 0;
        albumPrivacyCombo    = 0;
        commentsPrivacyCombo = 0; 
    }

    QLineEdit*      titleEdit;
    QTextEdit*      summaryEdit;
    QComboBox*      albumPrivacyCombo;
    QComboBox*      commentsPrivacyCombo;

    AlbumProperties album;
};

VKNewAlbumDlg::VKNewAlbumDlg(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    initDialog(false);
}

VKNewAlbumDlg::VKNewAlbumDlg(QWidget* const parent,
                             const AlbumProperties& album)
    : QDialog(parent),
      d(new Private)
{
    d->album = album;
    initDialog(true);
}

VKNewAlbumDlg::~VKNewAlbumDlg()
{
    delete d;
}

void VKNewAlbumDlg::initDialog(bool editing)
{
    setWindowTitle(editing ? i18nc("@title:window", "Edit album")
                           : i18nc("@title:window", "New album"));
    setMinimumSize(400, 300);

    QVBoxLayout* const mainLayout     = new QVBoxLayout(this);
    setLayout(mainLayout);

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    QPushButton* const okButton       = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &VKNewAlbumDlg::accept);

    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &VKNewAlbumDlg::reject);

    QGroupBox* const albumBox = new QGroupBox(i18nc("@title:group Header above Title and Summary fields", "Album"), this);
    albumBox->setWhatsThis(i18n("These are basic settings for the new VKontakte album."));

    d->titleEdit              = new QLineEdit(d->album.title);
    d->titleEdit->setWhatsThis(i18n("Title of the album that will be created (required)."));

    d->summaryEdit            = new QTextEdit(d->album.description);
    d->summaryEdit->setWhatsThis(i18n("Description of the album that will be created (optional)."));


    QFormLayout* const albumBoxLayout   = new QFormLayout;
    albumBoxLayout->addRow(i18n("Title:"),   d->titleEdit);
    albumBoxLayout->addRow(i18n("Summary:"), d->summaryEdit);
    albumBox->setLayout(albumBoxLayout);

    QGroupBox* const privacyBox         = new QGroupBox(i18n("Privacy Settings"), this);
    QGridLayout* const privacyBoxLayout = new QGridLayout;

    d->albumPrivacyCombo = new QComboBox(privacyBox);
    d->albumPrivacyCombo->addItem(i18n("Only me"),               QVariant(Vkontakte::AlbumInfo::PRIVACY_PRIVATE));
    d->albumPrivacyCombo->addItem(i18n("My friends"),            QVariant(Vkontakte::AlbumInfo::PRIVACY_FRIENDS));
    d->albumPrivacyCombo->addItem(i18n("Friends of my friends"), QVariant(Vkontakte::AlbumInfo::PRIVACY_FRIENDS_OF_FRIENDS));
    d->albumPrivacyCombo->addItem(i18n("Everyone"),              QVariant(Vkontakte::AlbumInfo::PRIVACY_PUBLIC));
    privacyBoxLayout->addWidget(new QLabel(i18n("Album available to:")), 0, 0);
    privacyBoxLayout->addWidget(d->albumPrivacyCombo, 0, 1);

    d->commentsPrivacyCombo = new QComboBox(privacyBox);
    d->commentsPrivacyCombo->addItem(i18n("Only me"),               QVariant(Vkontakte::AlbumInfo::PRIVACY_PRIVATE));
    d->commentsPrivacyCombo->addItem(i18n("My friends"),            QVariant(Vkontakte::AlbumInfo::PRIVACY_FRIENDS));
    d->commentsPrivacyCombo->addItem(i18n("Friends of my friends"), QVariant(Vkontakte::AlbumInfo::PRIVACY_FRIENDS_OF_FRIENDS));
    d->commentsPrivacyCombo->addItem(i18n("Everyone"),              QVariant(Vkontakte::AlbumInfo::PRIVACY_PUBLIC));
    privacyBoxLayout->addWidget(new QLabel(i18n("Comments available to:")), 1, 0);
    privacyBoxLayout->addWidget(d->commentsPrivacyCombo, 1, 1);

    privacyBox->setLayout(privacyBoxLayout);

    mainLayout->addWidget(albumBox);
    mainLayout->addWidget(privacyBox);
    mainLayout->addWidget(buttonBox);

    if (editing)
    {
        d->titleEdit->setText(d->album.title);
        d->summaryEdit->setText(d->album.description);
        d->albumPrivacyCombo->setCurrentIndex(d->albumPrivacyCombo->findData(d->album.privacy));
        d->commentsPrivacyCombo->setCurrentIndex(d->commentsPrivacyCombo->findData(d->album.commentPrivacy));
    }

    d->titleEdit->setFocus();
}

void VKNewAlbumDlg::accept()
{
    if (d->titleEdit->text().isEmpty())
    {
        QMessageBox::critical(this, i18n("Error"), i18n("Title cannot be empty."));
        return;
    }

    d->album.title       = d->titleEdit->text();
    d->album.description = d->summaryEdit->toPlainText();

    if (d->albumPrivacyCombo->currentIndex() != -1)
    {
        d->album.privacy = d->albumPrivacyCombo->itemData(d->albumPrivacyCombo->currentIndex()).toInt();
    }
    else
    {
        // for safety, see info about VK API bug below
        d->album.privacy = Vkontakte::AlbumInfo::PRIVACY_PRIVATE;
    }

    if (d->commentsPrivacyCombo->currentIndex() != -1)
    {
        d->album.commentPrivacy = d->commentsPrivacyCombo->itemData(d->commentsPrivacyCombo->currentIndex()).toInt();
    }
    else
    {
        // VK API has a bug: if "comment_privacy" is not set, it will be set to PRIVACY_PUBLIC
        d->album.commentPrivacy = Vkontakte::AlbumInfo::PRIVACY_PRIVATE;
    }

    QDialog::accept();
}

const VKNewAlbumDlg::AlbumProperties& VKNewAlbumDlg::album() const
{
    return d->album;
}

} // namespace Digikam
