/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-07
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "smugnewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// local includes

#include "smugitem.h"

namespace Digikam
{

class SmugNewAlbumDlg::Private
{
public:

    explicit Private()
    {
        categCoB     = 0;
        subCategCoB  = 0;
        templateCoB  = 0;
        privBox      = 0;
        titleEdt     = 0;
        passwdEdt    = 0;
        hintEdt      = 0;
        descEdt      = 0;
        publicRBtn   = 0;
        unlistedRBtn = 0;
    }

    QComboBox*    categCoB;
    QComboBox*    subCategCoB;
    QComboBox*    templateCoB;
    QGroupBox*    privBox;
    QLineEdit*    titleEdt;
    QLineEdit*    passwdEdt;
    QLineEdit*    hintEdt;
    QTextEdit*    descEdt;
    QRadioButton* publicRBtn;
    QRadioButton* unlistedRBtn;
};

SmugNewAlbumDlg::SmugNewAlbumDlg(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    QString header(i18n("SmugMug New Album"));
    setWindowTitle(header);
    setModal(false);
    setMinimumSize(400, 400);

    // ------------------------------------------------------------------------

    const int spacing         = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGroupBox* const albumBox = new QGroupBox(i18n("Album"), this);
    albumBox->setWhatsThis(i18n("These are basic settings for the new SmugMug album."));

    d->titleEdt          = new QLineEdit;
    d->titleEdt->setWhatsThis(i18n("Title of the album that will be created (required)."));

    d->categCoB          = new QComboBox;
    d->categCoB->setEditable(false);
    d->categCoB->setWhatsThis(i18n("Category of the album that will be created (required)."));

    d->subCategCoB       = new QComboBox;
    d->subCategCoB->setEditable(false);
    d->subCategCoB->setWhatsThis(i18n("Subcategory of the album that will be created (optional)."));

    d->descEdt           = new QTextEdit;
    d->descEdt->setWhatsThis(i18n("Description of the album that will be created (optional)."));

    d->templateCoB       = new QComboBox;
    d->templateCoB->setEditable(false);
    d->templateCoB->setWhatsThis(i18n("Album template for the new album (optional)."));

    QFormLayout* const albumBoxLayout = new QFormLayout;
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Title:"),       d->titleEdt);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Category:"),    d->categCoB);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Subcategory:"), d->subCategCoB);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Description:"), d->descEdt);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Template:"),    d->templateCoB);
    albumBoxLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    albumBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    albumBoxLayout->setSpacing(spacing);
    albumBox->setLayout(albumBoxLayout);

    // ------------------------------------------------------------------------

    d->privBox      = new QGroupBox(i18n("Security && Privacy"), this);
    d->privBox->setWhatsThis(i18n("These are security and privacy settings for the new SmugMug album."));

    d->publicRBtn   = new QRadioButton(i18nc("smug album privacy", "Public"));
    d->publicRBtn->setChecked(true);
    d->publicRBtn->setWhatsThis(i18n("Public album is listed on your public SmugMug page."));
    d->unlistedRBtn = new QRadioButton(i18nc("smug album privacy", "Unlisted"));
    d->unlistedRBtn->setWhatsThis(i18n("Unlisted album is only accessible via URL."));

    QHBoxLayout* const radioLayout = new QHBoxLayout;
    radioLayout->addWidget(d->publicRBtn);
    radioLayout->addWidget(d->unlistedRBtn);

    d->passwdEdt = new QLineEdit;
    d->passwdEdt->setWhatsThis(i18n("Require password to access the album (optional)."));

    d->hintEdt   = new QLineEdit;
    d->hintEdt->setWhatsThis(i18n("Password hint to present to users in the password prompt (optional)."));

    QFormLayout* const privBoxLayout = new QFormLayout;
    privBoxLayout->addRow(i18n("Privacy:"),       radioLayout);
    privBoxLayout->addRow(i18n("Password:"),      d->passwdEdt);
    privBoxLayout->addRow(i18n("Password Hint:"), d->hintEdt);
    privBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    privBoxLayout->setSpacing(spacing);
    d->privBox->setLayout(privBoxLayout);

    // ------------------------------------------------------------------------

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

    connect(buttonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);

    connect(buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    // ------------------------------------------------------------------------

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(albumBox);
    mainLayout->addWidget(d->privBox);
    mainLayout->addWidget(buttonBox);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(spacing);
    setLayout(mainLayout);
}

SmugNewAlbumDlg::~SmugNewAlbumDlg()
{
    delete d;
}

void SmugNewAlbumDlg::getAlbumProperties(SmugAlbum &album)
{
    album.title         = d->titleEdt->text();

    album.category      = d->categCoB->currentText();
    album.categoryID    = d->categCoB->itemData(d->categCoB->currentIndex()).toLongLong();

    album.subCategory   = d->subCategCoB->currentText();
    album.subCategoryID = d->subCategCoB->itemData(d->subCategCoB->currentIndex()).toLongLong();

    album.description   = d->descEdt->toPlainText();

    album.tmpl          = d->templateCoB->currentText();
    album.tmplID        = d->templateCoB->itemData(d->templateCoB->currentIndex()).toLongLong();

    album.isPublic      = d->publicRBtn->isChecked();
    album.password      = d->passwdEdt->text();
    album.passwordHint  = d->hintEdt->text();
}

QComboBox* SmugNewAlbumDlg::categoryCombo() const
{
    return d->categCoB;
}

QComboBox* SmugNewAlbumDlg::subCategoryCombo() const
{
    return d->subCategCoB;
}

QComboBox* SmugNewAlbumDlg::templateCombo() const
{
    return d->templateCoB;
}

QGroupBox* SmugNewAlbumDlg::privateGroupBox() const
{
    return d->privBox;
}

} // namespace Digikam
