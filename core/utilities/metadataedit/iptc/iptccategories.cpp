/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC categories settings page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Victor Dodon <dodonvictor at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "iptccategories.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QValidator>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QListWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"

namespace Digikam
{

class IPTCCategories::Private
{
public:

    Private()
    {
        addSubCategoryButton = 0;
        delSubCategoryButton = 0;
        repSubCategoryButton = 0;
        subCategoriesBox     = 0;
        subCategoriesCheck   = 0;
        categoryCheck        = 0;
        categoryEdit         = 0;
        subCategoryEdit      = 0;
    }

    QStringList  oldSubCategories;

    QPushButton* addSubCategoryButton;
    QPushButton* delSubCategoryButton;
    QPushButton* repSubCategoryButton;

    QCheckBox*   subCategoriesCheck;
    QCheckBox*   categoryCheck;

    QLineEdit*   categoryEdit;
    QLineEdit*   subCategoryEdit;

    QListWidget* subCategoriesBox;
};

IPTCCategories::IPTCCategories(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // IPTC only accept printable Ascii char.
    QRegExp asciiRx(QLatin1String("[\x20-\x7F]+$"));
    QValidator* const asciiValidator = new QRegExpValidator(asciiRx, this);

    // --------------------------------------------------------

    d->categoryCheck = new QCheckBox(i18n("Identify subject of content (3 chars max):"), this);
    d->categoryEdit  = new QLineEdit(this);
    d->categoryEdit->setClearButtonEnabled(true);
    d->categoryEdit->setValidator(asciiValidator);
    d->categoryEdit->setMaxLength(3);
    d->categoryEdit->setWhatsThis(i18n("Set here the category of content. This field is limited "
                                       "to 3 ASCII characters."));

    d->subCategoriesCheck = new QCheckBox(i18n("Supplemental categories:"), this);

    d->subCategoryEdit = new QLineEdit(this);
    d->subCategoryEdit->setClearButtonEnabled(true);
    d->subCategoryEdit->setValidator(asciiValidator);
    d->subCategoryEdit->setMaxLength(32);
    d->subCategoryEdit->setWhatsThis(i18n("Enter here a new supplemental category of content. "
                                          "This field is limited to 32 ASCII characters."));

    d->subCategoriesBox = new QListWidget(this);
    d->subCategoriesBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addSubCategoryButton = new QPushButton( i18n("&Add"), this);
    d->delSubCategoryButton = new QPushButton( i18n("&Delete"), this);
    d->repSubCategoryButton = new QPushButton( i18n("&Replace"), this);
    d->addSubCategoryButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delSubCategoryButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->repSubCategoryButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->delSubCategoryButton->setEnabled(false);
    d->repSubCategoryButton->setEnabled(false);

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note: "
                 "<b><a href='http://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a></b> "
                 "text tags only support the printable "
                 "<b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                 "characters and limit string sizes. "
                 "Use contextual help for details.</b>"), this);
    note->setMaximumWidth(150);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    grid->setAlignment( Qt::AlignTop );
    grid->addWidget(d->categoryCheck,           0, 0, 1, 2);
    grid->addWidget(d->categoryEdit,            0, 2, 1, 1);
    grid->addWidget(d->subCategoriesCheck,      1, 0, 1, 3);
    grid->addWidget(d->subCategoryEdit,         2, 0, 1, 3);
    grid->addWidget(d->subCategoriesBox,        3, 0, 5, 3);
    grid->addWidget(d->addSubCategoryButton,    3, 3, 1, 1);
    grid->addWidget(d->delSubCategoryButton,    4, 3, 1, 1);
    grid->addWidget(d->repSubCategoryButton,    5, 3, 1, 1);
    grid->addWidget(note,                       6, 3, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(7, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->categoryCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckCategoryToggled(bool)));

    connect(d->subCategoriesCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckSubCategoryToggled(bool)));

    enableWidgets(d->categoryCheck->isChecked(), d->subCategoriesCheck->isChecked());

    // --------------------------------------------------------

    connect(d->subCategoriesBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotCategorySelectionChanged()));

    connect(d->addSubCategoryButton, SIGNAL(clicked()),
            this, SLOT(slotAddCategory()));

    connect(d->delSubCategoryButton, SIGNAL(clicked()),
            this, SLOT(slotDelCategory()));

    connect(d->repSubCategoryButton, SIGNAL(clicked()),
            this, SLOT(slotRepCategory()));

    // --------------------------------------------------------

    connect(d->categoryCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->subCategoriesCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->addSubCategoryButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->delSubCategoryButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->repSubCategoryButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->categoryEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

IPTCCategories::~IPTCCategories()
{
    delete d;
}

void IPTCCategories::slotDelCategory()
{
    QListWidgetItem* const item = d->subCategoriesBox->currentItem();
    if (!item) return;

    d->subCategoriesBox->takeItem(d->subCategoriesBox->row(item));
    delete item;
}

void IPTCCategories::slotRepCategory()
{
    QString newCategory = d->subCategoryEdit->text();
    if (newCategory.isEmpty()) return;

    if (!d->subCategoriesBox->selectedItems().isEmpty())
    {
        d->subCategoriesBox->selectedItems()[0]->setText(newCategory);
        d->subCategoryEdit->clear();
    }
}

void IPTCCategories::slotCategorySelectionChanged()
{
    if (!d->subCategoriesBox->selectedItems().isEmpty())
    {
        d->subCategoryEdit->setText(d->subCategoriesBox->selectedItems()[0]->text());
        d->delSubCategoryButton->setEnabled(true);
        d->repSubCategoryButton->setEnabled(true);
    }
    else
    {
        d->delSubCategoryButton->setEnabled(false);
        d->repSubCategoryButton->setEnabled(false);
    }
}

void IPTCCategories::slotAddCategory()
{
    QString newCategory = d->subCategoryEdit->text();
    if (newCategory.isEmpty()) return;

    bool found = false;

    for (int i = 0 ; i < d->subCategoriesBox->count(); ++i)
    {
        QListWidgetItem* const item = d->subCategoriesBox->item(i);

        if (newCategory == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->subCategoriesBox->insertItem(d->subCategoriesBox->count(), newCategory);
        d->subCategoryEdit->clear();
    }
}

void IPTCCategories::readMetadata(QByteArray& iptcData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setIptc(iptcData);
    QString data;

    // In first we handle all sub-categories.

    d->subCategoriesBox->clear();
    d->subCategoriesCheck->setChecked(false);
    d->oldSubCategories = meta.getIptcSubCategories();

    if (!d->oldSubCategories.isEmpty())
    {
        d->subCategoriesBox->insertItems(0, d->oldSubCategories);
        d->subCategoriesCheck->setChecked(true);
    }

    // And in second, the main category because all sub-categories status depend of this one.

    d->categoryEdit->clear();
    d->categoryCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Category", false);

    if (!data.isNull())
    {
        d->categoryEdit->setText(data);
        d->categoryCheck->setChecked(true);
    }

    d->categoryEdit->setEnabled(d->categoryCheck->isChecked());
    d->subCategoriesCheck->setEnabled(d->categoryCheck->isChecked());
    d->subCategoryEdit->setEnabled(d->categoryCheck->isChecked() && d->subCategoriesCheck->isChecked());
    d->subCategoriesBox->setEnabled(d->categoryCheck->isChecked() && d->subCategoriesCheck->isChecked());
    d->addSubCategoryButton->setEnabled(d->categoryCheck->isChecked() && d->subCategoriesCheck->isChecked());
    d->delSubCategoryButton->setEnabled(d->categoryCheck->isChecked() && d->subCategoriesCheck->isChecked());

    blockSignals(false);
}

void IPTCCategories::applyMetadata(QByteArray& iptcData)
{
    QStringList newCategories;
    DMetadata meta;
    meta.setIptc(iptcData);

    if (d->categoryCheck->isChecked())
        meta.setIptcTagString("Iptc.Application2.Category", d->categoryEdit->text());
    else
        meta.removeIptcTag("Iptc.Application2.Category");

    for (int i = 0 ; i < d->subCategoriesBox->count(); ++i)
    {
        QListWidgetItem* const item = d->subCategoriesBox->item(i);
        newCategories.append(item->text());
    }

    if (d->categoryCheck->isChecked() && d->subCategoriesCheck->isChecked())
        meta.setIptcSubCategories(d->oldSubCategories, newCategories);
    else
        meta.setIptcSubCategories(d->oldSubCategories, QStringList());

    iptcData = meta.getIptc();
}

void IPTCCategories::slotCheckCategoryToggled(bool checked)
{
    enableWidgets(checked, d->subCategoriesCheck->isChecked());
}

void IPTCCategories::slotCheckSubCategoryToggled(bool checked)
{
    enableWidgets(d->categoryCheck->isChecked(), checked);
}

void IPTCCategories::enableWidgets(bool checked1, bool checked2)
{
    d->categoryEdit->setEnabled(checked1);
    d->subCategoriesCheck->setEnabled(checked1);

    // --------------------------------------------------------

    d->subCategoryEdit->setEnabled(checked1 && checked2);
    d->subCategoriesBox->setEnabled(checked1 && checked2);
    d->addSubCategoryButton->setEnabled(checked1 && checked2);
    d->delSubCategoryButton->setEnabled(checked1 && checked2);
    d->repSubCategoryButton->setEnabled(checked1 && checked2);
}

}  // namespace Digikam
