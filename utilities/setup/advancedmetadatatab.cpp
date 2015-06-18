/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-16
 * Description : Advanced Configuration tab for metadata.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail.com>
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

#include "advancedmetadatatab.h"

#include <QVBoxLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>

#include "klocale.h"


namespace Digikam
{

class AdvancedMetadataTab::Private
{
public:
    Private()
    {

    }
    QComboBox* metadataType;
    QComboBox* operationType;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* moveUpButton;
    QPushButton* moveDownButton;
    QPushButton* resetButton;
};
AdvancedMetadataTab::AdvancedMetadataTab(QWidget* parent)
    :QWidget(parent), d(new Private())
{
    // ---------- Advanced Configuration Panel -----------------------------

//    QWidget* const advancedConfPanel      = new QWidget;
    //QGridLayout* const advancedConfLayout = new QGridLayout;

    QVBoxLayout* const advancedConfLayout = new QVBoxLayout;

    QHBoxLayout* const topLayout = new QHBoxLayout;
    QHBoxLayout* const bottomLayout = new QHBoxLayout;

    //--- Top layout ----------------
    d->metadataType = new QComboBox;

    d->operationType = new QComboBox;

    d->metadataType->insertItems(0, QStringList() << i18n("Tags") << i18n("Ratings") << i18n("Comments"));

    d->operationType->insertItems(0, QStringList() << i18n("Read Options") << i18n("Write Options"));

    QCheckBox* checkBox = new QCheckBox(i18n("Unify read and write"));

    topLayout->addWidget(d->metadataType);
    topLayout->addWidget(d->operationType);
    topLayout->addWidget(checkBox);

    //------------ Bottom Layout-------------
    QListView* nameSpaceView = new QListView();
    QVBoxLayout* buttonsLayout = new QVBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignTop);
    d->addButton = new QPushButton(i18n("Add"));
    d->deleteButton = new QPushButton(i18n("Delete"));
    d->moveUpButton = new QPushButton(i18n("Move Up"));
    d->moveDownButton = new QPushButton(i18n("Move Down"));
    d->resetButton = new QPushButton(i18n("Reset"));

    buttonsLayout->addWidget(d->addButton);
    buttonsLayout->addWidget(d->deleteButton);
    buttonsLayout->addWidget(d->moveUpButton);
    buttonsLayout->addWidget(d->moveDownButton);
    buttonsLayout->addWidget(d->resetButton);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(nameSpaceView);

    bottomLayout->addLayout(vbox);
    bottomLayout->addLayout(buttonsLayout);

    advancedConfLayout->addLayout(topLayout);
    advancedConfLayout->addLayout(bottomLayout);

    this->setLayout(advancedConfLayout);
}

AdvancedMetadataTab::~AdvancedMetadataTab()
{

}

}


