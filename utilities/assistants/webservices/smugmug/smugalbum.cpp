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

#include "smugalbum.h"

// Qt includes

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
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

SmugNewAlbum::SmugNewAlbum(QWidget* const parent)
    : QDialog(parent)
{
    QString header(i18n("SmugMug New Album"));
    setWindowTitle(header);
    setModal(false);

    setMinimumSize(400, 400);

    // ------------------------------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGroupBox* const albumBox = new QGroupBox(i18n("Album"), this);
    albumBox->setWhatsThis(i18n("These are basic settings for the new SmugMug album."));

    m_titleEdt          = new QLineEdit;
    m_titleEdt->setWhatsThis(i18n("Title of the album that will be created (required)."));

    m_categCoB          = new QComboBox;
    m_categCoB->setEditable(false);
    m_categCoB->setWhatsThis(i18n("Category of the album that will be created (required)."));

    m_subCategCoB       = new QComboBox;
    m_subCategCoB->setEditable(false);
    m_subCategCoB->setWhatsThis(i18n("Subcategory of the album that will be created (optional)."));

    m_descEdt           = new QTextEdit;
    m_descEdt->setWhatsThis(i18n("Description of the album that will be created (optional)."));

    m_templateCoB      = new QComboBox;
    m_templateCoB->setEditable(false);
    m_templateCoB->setWhatsThis(i18n("Album template for the new album (optional)."));

    QFormLayout* const albumBoxLayout = new QFormLayout;
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Title:"), m_titleEdt);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Category:"), m_categCoB);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Subcategory:"), m_subCategCoB);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Description:"), m_descEdt);
    albumBoxLayout->addRow(i18nc("new smug album dialog", "Template:"), m_templateCoB);
    albumBoxLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    albumBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    albumBoxLayout->setSpacing(spacing);
    albumBox->setLayout(albumBoxLayout);

    // ------------------------------------------------------------------------

    m_privBox      = new QGroupBox(i18n("Security && Privacy"), this);
    m_privBox->setWhatsThis(i18n("These are security and privacy settings for the new SmugMug album."));

    m_publicRBtn   = new QRadioButton(i18nc("smug album privacy", "Public"));
    m_publicRBtn->setChecked(true);
    m_publicRBtn->setWhatsThis(i18n("Public album is listed on your public SmugMug page."));
    m_unlistedRBtn = new QRadioButton(i18nc("smug album privacy", "Unlisted"));
    m_unlistedRBtn->setWhatsThis(i18n("Unlisted album is only accessible via URL."));

    QHBoxLayout* const radioLayout = new QHBoxLayout;
    radioLayout->addWidget(m_publicRBtn);
    radioLayout->addWidget(m_unlistedRBtn);

    m_passwdEdt = new QLineEdit;
    m_passwdEdt->setWhatsThis(i18n("Require password to access the album (optional)."));

    m_hintEdt   = new QLineEdit;
    m_hintEdt->setWhatsThis(i18n("Password hint to present to users in the password prompt (optional)."));

    QFormLayout* const privBoxLayout = new QFormLayout;
    privBoxLayout->addRow(i18n("Privacy:"), radioLayout);
    privBoxLayout->addRow(i18n("Password:"), m_passwdEdt);
    privBoxLayout->addRow(i18n("Password Hint:"), m_hintEdt);
    privBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    privBoxLayout->setSpacing(spacing);
    m_privBox->setLayout(privBoxLayout);

    // ------------------------------------------------------------------------

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // ------------------------------------------------------------------------

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(albumBox);
    mainLayout->addWidget(m_privBox);
    mainLayout->addWidget(buttonBox);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(spacing);
    setLayout(mainLayout);
}

SmugNewAlbum::~SmugNewAlbum()
{
}

void SmugNewAlbum::getAlbumProperties(SmugAlbum &album)
{
    album.title         = m_titleEdt->text();

    album.category      = m_categCoB->currentText();
    album.categoryID    = m_categCoB->itemData(m_categCoB->currentIndex()).toLongLong();

    album.subCategory   = m_subCategCoB->currentText();
    album.subCategoryID = m_subCategCoB->itemData(m_subCategCoB->currentIndex()).toLongLong();

    album.description   = m_descEdt->toPlainText();

    album.tmpl          = m_templateCoB->currentText();
    album.tmplID        = m_templateCoB->itemData(m_templateCoB->currentIndex()).toLongLong();

    album.isPublic      = m_publicRBtn->isChecked();
    album.password      = m_passwdEdt->text();
    album.passwordHint  = m_hintEdt->text();
}

} // namespace Digikam
