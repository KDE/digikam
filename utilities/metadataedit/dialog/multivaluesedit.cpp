/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-08
 * Description : a widget to edit a tag with multiple fixed values.
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

#include "multivaluesedit.h"

// Qt includes

#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "squeezedcombobox.h"
#include "metadatacheckbox.h"

namespace Digikam
{

class MultiValuesEdit::Private
{
public:

    Private()
    {
        addValueButton = 0;
        delValueButton = 0;
        repValueButton = 0;
        valueBox       = 0;
        valueCheck     = 0;
        dataList       = 0;
    }

    QStringList       oldValues;

    QPushButton*      addValueButton;
    QPushButton*      delValueButton;
    QPushButton*      repValueButton;

    QListWidget*      valueBox;

    MetadataCheckBox* valueCheck;

    SqueezedComboBox* dataList;
};

MultiValuesEdit::MultiValuesEdit(QWidget* const parent, const QString& title, const QString& desc)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->valueCheck     = new MetadataCheckBox(title, this);

    d->addValueButton = new QPushButton(this);
    d->delValueButton = new QPushButton(this);
    d->repValueButton = new QPushButton(this);
    d->addValueButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->delValueButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")));
    d->repValueButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->addValueButton->setWhatsThis(i18n("Add a new value to the list"));
    d->delValueButton->setWhatsThis(i18n("Remove the current selected value from the list"));
    d->repValueButton->setWhatsThis(i18n("Replace the current selected value from the list"));
    d->delValueButton->setEnabled(false);
    d->repValueButton->setEnabled(false);

    d->valueBox = new QListWidget(this);
    d->valueBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored);
    d->valueBox->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    d->dataList = new SqueezedComboBox(this);
    d->dataList->model()->sort(0);
    d->dataList->setWhatsThis(desc);

    // --------------------------------------------------------

    grid->setAlignment( Qt::AlignTop );
    grid->addWidget(d->valueCheck,      0, 0, 1, 1);
    grid->addWidget(d->addValueButton,  0, 1, 1, 1);
    grid->addWidget(d->delValueButton,  0, 2, 1, 1);
    grid->addWidget(d->repValueButton,  0, 3, 1, 1);
    grid->addWidget(d->valueBox,        0, 4, 3, 1);
    grid->addWidget(d->dataList,        2, 0, 1, 4);
    grid->setRowStretch(1, 10);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(4, 100);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->valueBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addValueButton, SIGNAL(clicked()),
            this, SLOT(slotAddValue()));

    connect(d->delValueButton, SIGNAL(clicked()),
            this, SLOT(slotDeleteValue()));

    connect(d->repValueButton, SIGNAL(clicked()),
            this, SLOT(slotReplaceValue()));

    // --------------------------------------------------------

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->dataList, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->addValueButton, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->delValueButton, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->repValueButton, SLOT(setEnabled(bool)));

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            d->valueBox, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->valueCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->addValueButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->delValueButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->repValueButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));
}

MultiValuesEdit::~MultiValuesEdit()
{
    delete d;
}

void MultiValuesEdit::slotDeleteValue()
{
    QListWidgetItem* const item = d->valueBox->currentItem();
    if (!item) return;

    d->valueBox->takeItem(d->valueBox->row(item));
    delete item;
}

void MultiValuesEdit::slotReplaceValue()
{
    QString newValue = d->dataList->itemHighlighted();
    if (newValue.isEmpty()) return;

    if (!d->valueBox->selectedItems().isEmpty())
        d->valueBox->selectedItems()[0]->setText(newValue);
}

void MultiValuesEdit::slotSelectionChanged()
{
    if (!d->valueBox->selectedItems().isEmpty())
    {
        d->dataList->findText(d->valueBox->selectedItems()[0]->text());
        d->delValueButton->setEnabled(true);
        d->repValueButton->setEnabled(true);
    }
    else
    {
        d->delValueButton->setEnabled(false);
        d->repValueButton->setEnabled(false);
    }
}

void MultiValuesEdit::slotAddValue()
{
    QString newValue = d->dataList->itemHighlighted();
    if (newValue.isEmpty()) return;

    bool found = false;

    for (int i = 0 ; i < d->valueBox->count(); ++i)
    {
        QListWidgetItem* const item = d->valueBox->item(i);

        if (newValue == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
        d->valueBox->insertItem(d->valueBox->count(), newValue);
}

void MultiValuesEdit::setData(const QStringList& data)
{
    d->dataList->clear();

    for (QStringList::const_iterator it = data.constBegin(); it != data.constEnd(); ++it )
        d->dataList->addSqueezedItem(*it);
}

QStringList MultiValuesEdit::getData() const
{
    QStringList data;

    for (int i = 0 ; i < d->dataList->count(); ++i)
    {
        data.append(d->dataList->item(i));
    }

    return data;
}

void MultiValuesEdit::setValues(const QStringList& values)
{
    blockSignals(true);
    d->oldValues = values;

    d->valueBox->clear();
    d->valueCheck->setChecked(false);

    if (!d->oldValues.isEmpty())
    {
        d->valueBox->insertItems(0, d->oldValues);
        d->valueCheck->setChecked(true);
    }

    d->dataList->setEnabled(d->valueCheck->isChecked());
    d->valueBox->setEnabled(d->valueCheck->isChecked());
    d->addValueButton->setEnabled(d->valueCheck->isChecked());
    d->delValueButton->setEnabled(d->valueCheck->isChecked());

    blockSignals(false);
}

bool MultiValuesEdit::getValues(QStringList& oldValues, QStringList& newValues)
{
    oldValues = d->oldValues;
    newValues.clear();

    for (int i = 0 ; i < d->valueBox->count(); ++i)
    {
        QListWidgetItem* const item = d->valueBox->item(i);
        newValues.append(item->text());
    }

    return d->valueCheck->isChecked();
}

void MultiValuesEdit::setValid(bool v)
{
    d->valueCheck->setValid(v);
}

bool MultiValuesEdit::isValid() const
{
    return d->valueCheck->isValid();
}

}  // namespace Digikam
