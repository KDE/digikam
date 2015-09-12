/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a dialog to edit EXIF,IPTC and XMP metadata
 *
 * Copyright (C) 2011      by Victor Dodon <dodon dot victor at gmail dot com>
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadataedit.h"

// Qt includes

#include <QCloseEvent>
#include <QKeyEvent>
#include <QPointer>
#include <QObject>
#include <QApplication>
#include <QMenu>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

// KDE includes

#include <kconfig.h>
#include <kguiitem.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kwindowconfig.h>

// Local includes

#include "exifeditwidget.h"
#include "iptceditwidget.h"
#include "xmpeditwidget.h"

namespace Digikam
{

class MetadataEditDialog::Private
{
public:

    Private()
    {
        isReadOnly = false;
        tabWidget  = 0;
        tabExif    = 0;
        tabIptc    = 0;
        tabXmp     = 0;
    }

    bool                  isReadOnly;

    QList<QUrl>           urls;
    QList<QUrl>::iterator currItem;

    QTabWidget*           tabWidget;

    QDialogButtonBox*     buttons;

    EXIFEditWidget*       tabExif;
    IPTCEditWidget*       tabIptc;
    XMPEditWidget*        tabXmp;
};

MetadataEditDialog::MetadataEditDialog(QWidget* const parent, const QList<QUrl>& urls)
    : QDialog(parent),
      d(new Private)
{
    setWindowTitle(i18n("Metadata edit dialog"));
    setModal(true);

    d->urls     = urls;
    d->currItem = d->urls.begin();

    QDialogButtonBox::StandardButtons btns = QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Close;

    if (d->urls.count() > 1)
        btns = btns | QDialogButtonBox::No | QDialogButtonBox::Yes;

    d->buttons = new QDialogButtonBox(btns, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(false);

    if (d->urls.count() > 1)
    {
        d->buttons->button(QDialogButtonBox::No)->setText(i18nc("@action:button",  "Previous"));
        d->buttons->button(QDialogButtonBox::Yes)->setText(i18nc("@action:button", "Next"));
    }

    d->tabWidget = new QTabWidget(this);
    d->tabExif   = new EXIFEditWidget(this);
    d->tabIptc   = new IPTCEditWidget(this);
    d->tabXmp    = new XMPEditWidget(this);
    d->tabWidget->addTab(d->tabExif, i18n("Edit EXIF"));
    d->tabWidget->addTab(d->tabIptc, i18n("Edit IPTC"));
    d->tabWidget->addTab(d->tabXmp,  i18n("Edit XMP"));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->tabWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    //----------------------------------------------------------

    connect(d->tabExif, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabIptc, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->tabXmp, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(this, SIGNAL(applyClicked()),
            this, SLOT(slotApply()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));

    connect(this, SIGNAL(closeClicked()),
            this, SLOT(slotClose()));

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotNext()));

    connect(this, SIGNAL(user2Clicked()),
            this, SLOT(slotPrevious()));

    connect(d->tabExif, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(d->tabIptc, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    connect(d->tabXmp, SIGNAL(signalSetReadOnly(bool)),
            this, SLOT(slotSetReadOnly(bool)));

    //----------------------------------------------------------

    readSettings();
    slotItemChanged();
}

MetadataEditDialog::~MetadataEditDialog()
{
    delete d;
}

QList<QUrl>::iterator MetadataEditDialog::currentItem() const
{
    return d->currItem;
}

void MetadataEditDialog::slotModified()
{
    bool modified = false;

    switch (d->tabWidget->currentIndex())
    {
        case 0:
            modified = d->tabExif->isModified();
            break;

        case 1:
            modified = d->tabIptc->isModified();
            break;

        case 2:
            modified = d->tabXmp->isModified();
            break;
    }

    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(modified);
}

void MetadataEditDialog::slotOk()
{
    slotApply();
    saveSettings();
    accept();
}

void MetadataEditDialog::slotClose()
{
    saveSettings();
    close();
}

void MetadataEditDialog::slotApply()
{
    d->tabExif->apply();
    d->tabIptc->apply();
    d->tabXmp->apply();
    slotItemChanged();
}

void MetadataEditDialog::slotNext()
{
    slotApply();
    d->currItem++;
    slotItemChanged();
}

void MetadataEditDialog::slotPrevious()
{
    slotApply();
    d->currItem--;
    slotItemChanged();
}

void MetadataEditDialog::readSettings()
{
    KConfig config(QLatin1String("kipirc")); // FIXME
    KConfigGroup group = config.group(QLatin1String("Metadata Edit Dialog"));
    d->tabWidget->setCurrentIndex(group.readEntry(QLatin1String("Tab Index"), 0));
    KWindowConfig::restoreWindowSize(windowHandle(), group);
}

void MetadataEditDialog::saveSettings()
{
    KConfig config(QLatin1String("kipirc")); // FIXME
    KConfigGroup group = config.group(QLatin1String("Metadata Edit Dialog"));
    group.writeEntry(QLatin1String("Tab Index"), d->tabWidget->currentIndex());
    KWindowConfig::saveWindowSize(windowHandle(), group);

    d->tabExif->saveSettings();
    d->tabIptc->saveSettings();
    d->tabXmp->saveSettings();
}

void MetadataEditDialog::slotItemChanged()
{
    d->tabExif->slotItemChanged();
    d->tabIptc->slotItemChanged();
    d->tabXmp->slotItemChanged();

    setWindowTitle(i18n("%1 (%2/%3) - Edit Metadata")
        .arg((*d->currItem).fileName())
        .arg(d->urls.indexOf(*(d->currItem))+1)
        .arg(d->urls.count()));

    if (d->urls.count() > 1)
    {
        d->buttons->button(QDialogButtonBox::No)->setEnabled(*(d->currItem) != d->urls.last());
        d->buttons->button(QDialogButtonBox::Yes)->setEnabled(*(d->currItem) != d->urls.first());
    }

    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(!d->isReadOnly);
}

bool MetadataEditDialog::eventFilter(QObject*, QEvent* e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent* const k = (QKeyEvent*)e;

        if (k->modifiers() == Qt::ControlModifier &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();

            if ((d->urls.count() > 1) && d->buttons->button(QDialogButtonBox::No)->isEnabled())
                slotNext();

            return true;
        }
        else if (k->modifiers() == Qt::ShiftModifier &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();

            if ((d->urls.count() > 1) && d->buttons->button(QDialogButtonBox::Yes)->isEnabled())
                slotPrevious();

            return true;
        }

        return false;
    }

    return false;
}

void MetadataEditDialog::closeEvent(QCloseEvent* e)
{
    if (!e) return;
    saveSettings();
    e->accept();
}

void MetadataEditDialog::slotSetReadOnly(bool state)
{
    d->isReadOnly = state;
}

}  // namespace Digikam
