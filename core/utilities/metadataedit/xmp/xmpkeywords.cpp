/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-16
 * Description : XMP keywords settings page.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "xmpkeywords.h"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
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

class XMPKeywords::Private
{
public:

    Private()
    {
        addKeywordButton = 0;
        delKeywordButton = 0;
        repKeywordButton = 0;
        keywordsBox      = 0;
        keywordsCheck    = 0;
        keywordEdit      = 0;
    }

    QStringList  oldKeywords;

    QPushButton* addKeywordButton;
    QPushButton* delKeywordButton;
    QPushButton* repKeywordButton;

    QCheckBox*   keywordsCheck;

    QLineEdit*   keywordEdit;

    QListWidget* keywordsBox;
};

XMPKeywords::XMPKeywords(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->keywordsCheck = new QCheckBox(i18n("Use information retrieval words:"), this);

    d->keywordEdit   = new QLineEdit(this);
    d->keywordEdit->setClearButtonEnabled(true);
    d->keywordEdit->setWhatsThis(i18n("Enter here a new keyword."));

    d->keywordsBox   = new QListWidget(this);
    d->keywordsBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addKeywordButton = new QPushButton( i18n("&Add"), this);
    d->delKeywordButton = new QPushButton( i18n("&Delete"), this);
    d->repKeywordButton = new QPushButton( i18n("&Replace"), this);
    d->addKeywordButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delKeywordButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->repKeywordButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->delKeywordButton->setEnabled(false);
    d->repKeywordButton->setEnabled(false);

    // --------------------------------------------------------

    grid->setAlignment( Qt::AlignTop );
    grid->addWidget(d->keywordsCheck,       0, 0, 1, 2);
    grid->addWidget(d->keywordEdit,         1, 0, 1, 1);
    grid->addWidget(d->keywordsBox,         2, 0, 5, 1);
    grid->addWidget(d->addKeywordButton,    2, 1, 1, 1);
    grid->addWidget(d->delKeywordButton,    3, 1, 1, 1);
    grid->addWidget(d->repKeywordButton,    4, 1, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setRowStretch(5, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->keywordsBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotKeywordSelectionChanged()));

    connect(d->addKeywordButton, SIGNAL(clicked()),
            this, SLOT(slotAddKeyword()));

    connect(d->delKeywordButton, SIGNAL(clicked()),
            this, SLOT(slotDelKeyword()));

    connect(d->repKeywordButton, SIGNAL(clicked()),
            this, SLOT(slotRepKeyword()));

    // --------------------------------------------------------

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->keywordEdit, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->addKeywordButton, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->delKeywordButton, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->repKeywordButton, SLOT(setEnabled(bool)));

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            d->keywordsBox, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->keywordsCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->addKeywordButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->delKeywordButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->repKeywordButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));
}

XMPKeywords::~XMPKeywords()
{
    delete d;
}

void XMPKeywords::slotDelKeyword()
{
    QListWidgetItem* const item = d->keywordsBox->currentItem();
    if (!item) return;

    d->keywordsBox->takeItem(d->keywordsBox->row(item));
    delete item;
}

void XMPKeywords::slotRepKeyword()
{
    QString newKeyword = d->keywordEdit->text();
    if (newKeyword.isEmpty()) return;

    if (!d->keywordsBox->selectedItems().isEmpty())
    {
        d->keywordsBox->selectedItems()[0]->setText(newKeyword);
        d->keywordEdit->clear();
    }
}

void XMPKeywords::slotKeywordSelectionChanged()
{
    if (!d->keywordsBox->selectedItems().isEmpty())
    {
        d->keywordEdit->setText(d->keywordsBox->selectedItems()[0]->text());
        d->delKeywordButton->setEnabled(true);
        d->repKeywordButton->setEnabled(true);
    }
    else
    {
        d->delKeywordButton->setEnabled(false);
        d->repKeywordButton->setEnabled(false);
    }
}

void XMPKeywords::slotAddKeyword()
{
    QString newKeyword = d->keywordEdit->text();
    if (newKeyword.isEmpty()) return;

    bool found = false;

    for (int i = 0 ; i < d->keywordsBox->count(); ++i)
    {
        QListWidgetItem* const item = d->keywordsBox->item(i);

        if (newKeyword == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->keywordsBox->insertItem(d->keywordsBox->count(), newKeyword);
        d->keywordEdit->clear();
    }
}

void XMPKeywords::readMetadata(QByteArray& xmpData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setXmp(xmpData);
    d->oldKeywords = meta.getXmpKeywords();

    d->keywordsBox->clear();
    d->keywordsCheck->setChecked(false);

    if (!d->oldKeywords.isEmpty())
    {
        d->keywordsBox->insertItems(0, d->oldKeywords);
        d->keywordsCheck->setChecked(true);
    }

    d->keywordEdit->setEnabled(d->keywordsCheck->isChecked());
    d->keywordsBox->setEnabled(d->keywordsCheck->isChecked());
    d->addKeywordButton->setEnabled(d->keywordsCheck->isChecked());
    d->delKeywordButton->setEnabled(d->keywordsCheck->isChecked());

    blockSignals(false);
}

void XMPKeywords::applyMetadata(QByteArray& xmpData)
{
    DMetadata meta;
    meta.setXmp(xmpData);
    QStringList newKeywords;

    for (int i = 0 ; i < d->keywordsBox->count(); ++i)
    {
        QListWidgetItem* const item = d->keywordsBox->item(i);
        newKeywords.append(item->text());
    }

    // We remove in first all existing keywords.
    meta.removeXmpTag("Xmp.dc.subject");

    // And add new list if necessary.
    if (d->keywordsCheck->isChecked())
        meta.setXmpKeywords(newKeywords);

    xmpData = meta.getXmp();
}

}  // namespace Digikam
