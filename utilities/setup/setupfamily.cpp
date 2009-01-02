/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : album family setup tab.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupfamily.h"
#include "setupfamily.moc"

// Qt includes.

#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDir>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kpagedialog.h>
#include <kurl.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <klistwidget.h>

// Local includes.

#include "thumbnailsize.h"
#include "albumsettings.h"

namespace Digikam
{

class SetupFamilyPriv
{
public:

    SetupFamilyPriv()
    {
        familyEdit      = 0;
        albumFamilyBox  = 0;
        addFamilyButton = 0;
        delFamilyButton = 0;
        repFamilyButton = 0;
    }

    QPushButton *addFamilyButton;
    QPushButton *delFamilyButton;
    QPushButton *repFamilyButton;

    KListWidget *albumFamilyBox;

    KLineEdit   *familyEdit;
};

SetupFamily::SetupFamily(QWidget* parent )
              : QWidget(parent), d(new SetupFamilyPriv)
{
    QGridLayout *grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->familyEdit = new KLineEdit(this);
    d->familyEdit->setClearButtonShown(true);

    d->albumFamilyBox = new KListWidget(this);
    d->albumFamilyBox->setWhatsThis(i18n("You can add or remove Album "
                                             "family types here to improve how "
                                             "your Albums are sorted in digiKam."));

    d->albumFamilyBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addFamilyButton = new QPushButton(i18n("&Add..."), this);
    d->delFamilyButton = new QPushButton(i18n("&Remove"), this);
    d->repFamilyButton = new QPushButton(i18n("&Replace"), this);

    d->addFamilyButton->setIcon(SmallIcon("list-add"));
    d->delFamilyButton->setIcon(SmallIcon("list-remove"));
    d->repFamilyButton->setIcon(SmallIcon("view-refresh"));
    d->delFamilyButton->setEnabled(false);
    d->repFamilyButton->setEnabled(false);

    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->familyEdit,      0, 0, 1, 1);
    grid->addWidget(d->albumFamilyBox,  1, 0, 5, 1);
    grid->addWidget(d->addFamilyButton, 1, 1, 1, 1);
    grid->addWidget(d->delFamilyButton, 2, 1, 1, 1);
    grid->addWidget(d->repFamilyButton, 3, 1, 1, 1);
    grid->setRowStretch(4, 10);
    grid->setColumnStretch(0, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->albumFamilyBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotFamilySelectionChanged()));

    connect(d->addFamilyButton, SIGNAL(clicked()),
            this, SLOT(slotAddFamily()));

    connect(d->delFamilyButton, SIGNAL(clicked()),
            this, SLOT(slotDelFamily()));

    connect(d->repFamilyButton, SIGNAL(clicked()),
            this, SLOT(slotRepFamily()));

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupFamily::~SetupFamily()
{
    delete d;
}

void SetupFamily::slotDelFamily()
{
    QListWidgetItem *item = d->albumFamilyBox->currentItem();
    if (!item) return;
    d->albumFamilyBox->takeItem(d->albumFamilyBox->row(item));
    delete item;
}

void SetupFamily::slotRepFamily()
{
    QString newFamily = d->familyEdit->text();
    if (newFamily.isEmpty()) return;

    if (!d->albumFamilyBox->selectedItems().isEmpty())
    {
        d->albumFamilyBox->selectedItems()[0]->setText(newFamily);
        d->familyEdit->clear();
    }
}

void SetupFamily::slotFamilySelectionChanged()
{
    if (!d->albumFamilyBox->selectedItems().isEmpty())
    {
        d->familyEdit->setText(d->albumFamilyBox->selectedItems()[0]->text());
        d->delFamilyButton->setEnabled(true);
        d->repFamilyButton->setEnabled(true);
    }
    else
    {
        d->delFamilyButton->setEnabled(false);
        d->repFamilyButton->setEnabled(false);
    }
}

void SetupFamily::slotAddFamily()
{
    QString newFamily = d->familyEdit->text();
    if (newFamily.isEmpty()) return;

    bool found = false;
    for (int i = 0 ; i < d->albumFamilyBox->count(); i++)
    {
        QListWidgetItem *item = d->albumFamilyBox->item(i);
        if (newFamily == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->albumFamilyBox->insertItem(d->albumFamilyBox->count(), newFamily);
        d->familyEdit->clear();
    }
}

void SetupFamily::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    QStringList familyList;

    for (int i = 0 ; i < d->albumFamilyBox->count(); i++)
    {
        QListWidgetItem *item = d->albumFamilyBox->item(i);
        familyList.append(item->text());
    }

    settings->setAlbumFamilyNames(familyList);
    settings->saveSettings();
}

void SetupFamily::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->albumFamilyBox->insertItems(0, settings->getAlbumFamilyNames());
}

}  // namespace Digikam
