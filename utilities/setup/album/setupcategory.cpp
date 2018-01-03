/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : album category setup tab.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupcategory.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QUrl>
#include <QListWidget>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "applicationsettings.h"
#include "thumbnailsize.h"

namespace Digikam
{

class SetupCategory::Private
{
public:

    Private() :
        addCategoryButton(0),
        delCategoryButton(0),
        repCategoryButton(0),
        albumCategoryBox(0),
        categoryEdit(0)
    {
    }

    QPushButton* addCategoryButton;
    QPushButton* delCategoryButton;
    QPushButton* repCategoryButton;

    QListWidget* albumCategoryBox;

    QLineEdit*   categoryEdit;
};

SetupCategory::SetupCategory(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel    = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QGridLayout* const grid = new QGridLayout(panel);

    QLabel* const explanationLabel = new QLabel(panel);
    explanationLabel->setText(i18n("Manage categories to sort and re-arrange album tree-view."));
    explanationLabel->setWordWrap(true);

    // --------------------------------------------------------

    d->categoryEdit     = new QLineEdit(panel);
    d->categoryEdit->setClearButtonEnabled(true);

    d->albumCategoryBox = new QListWidget(panel);
    d->albumCategoryBox->setWhatsThis(i18n("You can add or remove Album "
                                           "category types here to improve how "
                                           "your Albums are sorted in digiKam."));

    d->albumCategoryBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addCategoryButton = new QPushButton(i18n("&Add..."), panel);
    d->delCategoryButton = new QPushButton(i18n("&Remove"), panel);
    d->repCategoryButton = new QPushButton(i18n("&Replace"), panel);

    d->addCategoryButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delCategoryButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    d->repCategoryButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->delCategoryButton->setEnabled(false);
    d->repCategoryButton->setEnabled(false);

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(explanationLabel,     0, 0, 1, 1);
    grid->addWidget(d->categoryEdit,      1, 0, 1, 1);
    grid->addWidget(d->albumCategoryBox,  2, 0, 5, 1);
    grid->addWidget(d->addCategoryButton, 2, 1, 1, 1);
    grid->addWidget(d->delCategoryButton, 3, 1, 1, 1);
    grid->addWidget(d->repCategoryButton, 4, 1, 1, 1);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(0, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->albumCategoryBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotCategorySelectionChanged()));

    connect(d->addCategoryButton, SIGNAL(clicked()),
            this, SLOT(slotAddCategory()));

    connect(d->delCategoryButton, SIGNAL(clicked()),
            this, SLOT(slotDelCategory()));

    connect(d->repCategoryButton, SIGNAL(clicked()),
            this, SLOT(slotRepCategory()));

    // --------------------------------------------------------

    adjustSize();
}

SetupCategory::~SetupCategory()
{
    delete d;
}

void SetupCategory::slotDelCategory()
{
    QListWidgetItem* const item = d->albumCategoryBox->currentItem();

    if (!item)
    {
        return;
    }

    d->albumCategoryBox->takeItem(d->albumCategoryBox->row(item));
    delete item;
}

void SetupCategory::slotRepCategory()
{
    QString newCategory = d->categoryEdit->text();

    if (newCategory.isEmpty())
    {
        return;
    }

    if (!d->albumCategoryBox->selectedItems().isEmpty())
    {
        d->albumCategoryBox->selectedItems().at(0)->setText(newCategory);
        d->categoryEdit->clear();
    }
}

void SetupCategory::slotCategorySelectionChanged()
{
    if (!d->albumCategoryBox->selectedItems().isEmpty())
    {
        d->categoryEdit->setText(d->albumCategoryBox->selectedItems().at(0)->text());
        d->delCategoryButton->setEnabled(true);
        d->repCategoryButton->setEnabled(true);
    }
    else
    {
        d->delCategoryButton->setEnabled(false);
        d->repCategoryButton->setEnabled(false);
    }
}

void SetupCategory::slotAddCategory()
{
    QString newCategory = d->categoryEdit->text();

    if (newCategory.isEmpty())
    {
        return;
    }

    bool found = false;

    for (int i = 0 ; i < d->albumCategoryBox->count(); ++i)
    {
        QListWidgetItem* const item = d->albumCategoryBox->item(i);

        if (newCategory == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->albumCategoryBox->insertItem(d->albumCategoryBox->count(), newCategory);
        d->categoryEdit->clear();
    }
}

void SetupCategory::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    QStringList categoryList;

    for (int i = 0 ; i < d->albumCategoryBox->count(); ++i)
    {
        QListWidgetItem* const item = d->albumCategoryBox->item(i);
        categoryList.append(item->text());
    }

    settings->setAlbumCategoryNames(categoryList);
    settings->saveSettings();
}

void SetupCategory::readSettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    d->albumCategoryBox->insertItems(0, settings->getAlbumCategoryNames());
}

} // namespace Digikam
