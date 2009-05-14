/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupcamera.h"
#include "setupcamera.moc"

// Qt includes

#include <QGroupBox>
#include <QPushButton>
#include <QDateTime>
#include <QGridLayout>
#include <QPixmap>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

// KDE includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kcursor.h>
#include <kapplication.h>
#include <ktoolinvocation.h>

// Local includes

#include "config-digikam.h"
#include "gpcamera.h"
#include "cameraselection.h"
#include "cameralist.h"
#include "cameratype.h"

namespace Digikam
{
class SetupCameraPriv
{
public:

    SetupCameraPriv()
    {
        listView         = 0;
        addButton        = 0;
        removeButton     = 0;
        editButton       = 0;
        autoDetectButton = 0;
    }

    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *editButton;
    QPushButton *autoDetectButton;

    QTreeWidget *listView;
};

SetupCamera::SetupCamera( QWidget* parent )
           : QScrollArea(parent), d(new SetupCameraPriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QGridLayout* grid = new QGridLayout(panel);
    d->listView       = new QTreeWidget(panel);
    d->listView->setColumnCount(5);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setWhatsThis( i18n("Here you can see the digital camera list used by digiKam "
                                    "via the Gphoto interface."));

    QStringList labels;
    labels.append( i18n("Title") );
    labels.append( i18n("Model") );
    labels.append( i18n("Port") );
    labels.append( i18n("Path") );
    labels.append( "Last Access Date" );  // No i18n here. Hidden column with the last access date.
    d->listView->setHeaderLabels(labels);
    d->listView->hideColumn(4);

    // -------------------------------------------------------------

    d->addButton        = new QPushButton(panel);
    d->removeButton     = new QPushButton(panel);
    d->editButton       = new QPushButton(panel);
    d->autoDetectButton = new QPushButton(panel);

    d->addButton->setText( i18n( "&Add..." ) );
    d->addButton->setIcon(SmallIcon("list-add"));
    d->removeButton->setText( i18n( "&Remove" ) );
    d->removeButton->setIcon(SmallIcon("list-remove"));
    d->editButton->setText( i18n( "&Edit..." ) );
    d->editButton->setIcon(SmallIcon("configure"));
    d->autoDetectButton->setText( i18n( "Auto-&Detect" ) );
    d->autoDetectButton->setIcon(SmallIcon("system-search"));
    d->removeButton->setEnabled(false);
    d->editButton->setEnabled(false);

    // -------------------------------------------------------------

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );

    KUrlLabel *gphotoLogoLabel = new KUrlLabel(panel);
    gphotoLogoLabel->setText(QString());
    gphotoLogoLabel->setUrl("http://www.gphoto.org");
    gphotoLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-gphoto.png")));
    gphotoLogoLabel->setToolTip(i18n("Visit Gphoto project website"));

#ifndef ENABLE_GPHOTO2
    // If digiKam is compiled without Gphoto2 support, we hide widgets relevant.
    d->autoDetectButton->hide();
    gphotoLogoLabel->hide();
#endif /* ENABLE_GPHOTO2 */

    // -------------------------------------------------------------

    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    grid->setAlignment(Qt::AlignTop);
    grid->addWidget(d->listView,         0, 0, 6, 1);
    grid->addWidget(d->addButton,        0, 1, 1, 1);
    grid->addWidget(d->removeButton,     1, 1, 1, 1);
    grid->addWidget(d->editButton,       2, 1, 1, 1);
    grid->addWidget(d->autoDetectButton, 3, 1, 1, 1);
    grid->addItem(spacer,                4, 1, 1, 1);
    grid->addWidget(gphotoLogoLabel,     5, 1, 1, 1);

    adjustSize();

    // -------------------------------------------------------------

    connect(gphotoLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(slotProcessGphotoUrl(const QString&)));

    connect(d->listView, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddCamera()));

    connect(d->removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveCamera()));

    connect(d->editButton, SIGNAL(clicked()),
            this, SLOT(slotEditCamera()));

    connect(d->autoDetectButton, SIGNAL(clicked()),
            this, SLOT(slotAutoDetectCamera()));

    // Add cameras --------------------------------------

    CameraList* clist = CameraList::defaultList();

    if (clist)
    {
        Q3PtrList<CameraType>* cl = clist->cameraList();

        for (CameraType *ctype = cl->first(); ctype;
             ctype = cl->next())
        {
            QStringList labels;
            labels.append(ctype->title());
            labels.append(ctype->model());
            labels.append(ctype->port());
            labels.append(ctype->path());
            new QTreeWidgetItem(d->listView, labels);
        }
    }
}

SetupCamera::~SetupCamera()
{
    delete d;
}

void SetupCamera::slotProcessGphotoUrl(const QString& url)
{
    KToolInvocation::self()->invokeBrowser(url);
}

void SetupCamera::slotSelectionChanged()
{
    QTreeWidgetItem *item = d->listView->currentItem();
    if (!item)
    {
        d->removeButton->setEnabled(false);
        d->editButton->setEnabled(false);
        return;
    }

    d->removeButton->setEnabled(true);
    d->editButton->setEnabled(true);
}

void SetupCamera::slotAddCamera()
{
    CameraSelection *select = new CameraSelection;

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&,
                                           const QString&, const QString&)),
            this,   SLOT(slotAddedCamera(const QString&, const QString&,
                                         const QString&, const QString&)));

    select->show();
}

void SetupCamera::slotRemoveCamera()
{
    QTreeWidgetItem *item = d->listView->currentItem();
    if (!item) return;

    delete item;
}

void SetupCamera::slotEditCamera()
{
    QTreeWidgetItem *item = d->listView->currentItem();
    if (!item) return;

    CameraSelection *select = new CameraSelection;
    select->setCamera(item->text(0), item->text(1), item->text(2), item->text(3));

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&,
                                           const QString&, const QString&)),
            this,   SLOT(slotEditedCamera(const QString&, const QString&,
                                          const QString&, const QString&)));

    select->show();
}

void SetupCamera::slotAutoDetectCamera()
{
    QString model, port;

    kapp->setOverrideCursor( Qt::WaitCursor );
    int ret = GPCamera::autoDetect(model, port);
    kapp->restoreOverrideCursor();

    if (ret != 0)
    {
        KMessageBox::error(this,i18n("Failed to auto-detect camera.\n"
                                     "Please check if your camera is turned on "
                                     "and retry or try setting it manually."));
        return;
    }

    // NOTE: See note in digikam/digikam/cameralist.cpp
    if (port.startsWith("usb:"))
    port = "usb:";

    if (!d->listView->findItems(model, Qt::MatchExactly, 1).isEmpty())
    {
        KMessageBox::information(this, i18n("Camera '%1' (%2) is already in list.", model, port));
    }
    else
    {
        KMessageBox::information(this, i18n("Found camera '%1' (%2) and added it to the list.", model, port));
        QStringList labels;
        labels.append(model);
        labels.append(model);
        labels.append(port);
        labels.append("/");
        labels.append(QDateTime::currentDateTime().toString(Qt::ISODate));
        new QTreeWidgetItem(d->listView, labels);
    }
}

void SetupCamera::slotAddedCamera(const QString& title, const QString& model,
                                  const QString& port, const QString& path)
{
    QStringList labels;
    labels.append(title);
    labels.append(model);
    labels.append(port);
    labels.append(path);
    labels.append(QDateTime::currentDateTime().toString(Qt::ISODate));
    new QTreeWidgetItem(d->listView, labels);
}

void SetupCamera::slotEditedCamera(const QString& title, const QString& model,
                                   const QString& port, const QString& path)
{
    QTreeWidgetItem *item = d->listView->currentItem();
    if (!item) return;

    item->setText(0, title);
    item->setText(1, model);
    item->setText(2, port);
    item->setText(3, path);
}

void SetupCamera::applySettings()
{
    CameraList* clist = CameraList::defaultList();

    if (clist)
    {
        clist->clear();

        QTreeWidgetItemIterator it(d->listView);
        while (*it)
        {
            QTreeWidgetItem *item = *it;
            if (item)
            {
                CameraType *ctype = new CameraType(item->text(0), item->text(1), item->text(2),
                                                   item->text(3));
                clist->insert(ctype);
            }
            ++it;
        }

        clist->save();
    }
}

}  // namespace Digikam
