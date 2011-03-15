/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupcamera.moc"

// Qt includes

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPixmap>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// Local includes

#include "cameralist.h"
#include "cameraselection.h"
#include "cameratype.h"
#include "config-digikam.h"
#include "gpcamera.h"

namespace Digikam
{

class SetupCameraItem : public QTreeWidgetItem
{

public:

    SetupCameraItem(QTreeWidget* parent, CameraType* ctype)
        : QTreeWidgetItem(parent), m_ctype(0)
    {
        setCameraType(ctype);
    };

    ~SetupCameraItem()
    {
        delete m_ctype;
    };

    void setCameraType(CameraType* ctype)
    {
        if (m_ctype)
        {
            delete m_ctype;
        }

        m_ctype = new CameraType(*ctype);

        if (m_ctype)
        {
            setText(0, m_ctype->title());
            setText(1, m_ctype->model());
            setText(2, m_ctype->port());
            setText(3, m_ctype->path());
        }
    };

    CameraType* cameraType() const
    {
        return m_ctype;
    };

private:

    CameraType* m_ctype;
};

// -------------------------------------------------------------------

class CameraAutoDetectThread::CameraAutoDetectThreadPriv
{
    public:

        CameraAutoDetectThreadPriv()
        {
            result = 0;
        }

    int     result;

    QString model;
    QString port;
};

CameraAutoDetectThread::CameraAutoDetectThread(QObject* parent)
    : DBusyThread(parent), d(new CameraAutoDetectThreadPriv)
{
    d->result = -1;
}

CameraAutoDetectThread::~CameraAutoDetectThread()
{
    delete d;
}

void CameraAutoDetectThread::run()
{
    d->result = GPCamera::autoDetect(d->model, d->port);
    emit signalComplete();
}

int CameraAutoDetectThread::result() const
{
    return(d->result);
}

QString CameraAutoDetectThread::model() const
{
    return(d->model);
}

QString CameraAutoDetectThread::port() const
{
    return(d->port);
}

// -------------------------------------------------------------------

class SetupCamera::SetupCameraPriv
{
public:

    SetupCameraPriv() :
        addButton(0),
        removeButton(0),
        editButton(0),
        autoDetectButton(0),
        listView(0)
    {
    }

    QPushButton* addButton;
    QPushButton* removeButton;
    QPushButton* editButton;
    QPushButton* autoDetectButton;

    QTreeWidget* listView;
};

SetupCamera::SetupCamera( QWidget* parent )
    : QScrollArea(parent), d(new SetupCameraPriv)
{
    QWidget* panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QGridLayout* grid = new QGridLayout(panel);
    d->listView       = new QTreeWidget(panel);
    d->listView->setColumnCount(4);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setWhatsThis(i18n("Here you can see the digital camera list used by digiKam "
                                   "via the Gphoto interface."));

    QStringList labels;
    labels.append( i18n("Title") );
    labels.append( i18n("Model") );
    labels.append( i18n("Port") );
    labels.append( i18n("Path") );
    d->listView->setHeaderLabels(labels);
    d->listView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
    d->listView->header()->setResizeMode(1, QHeaderView::Stretch);
    d->listView->header()->setResizeMode(2, QHeaderView::Stretch);
    d->listView->header()->setResizeMode(3, QHeaderView::Stretch);

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

    KUrlLabel* gphotoLogoLabel = new KUrlLabel(panel);
    gphotoLogoLabel->setText(QString());
    gphotoLogoLabel->setUrl("http://www.gphoto.org");
    gphotoLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-gphoto.png")));
    gphotoLogoLabel->setToolTip(i18n("Visit Gphoto project website"));

#ifndef HAVE_GPHOTO2
    // If digiKam is compiled without Gphoto2 support, we hide widgets relevant.
    d->autoDetectButton->hide();
    gphotoLogoLabel->hide();
#endif /* HAVE_GPHOTO2 */

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

    // Populate cameras --------------------------------------

    CameraList* clist = CameraList::defaultList();

    if (clist)
    {
        QList<CameraType*>* cl = clist->cameraList();

        foreach (CameraType* ctype, *cl)
        {
            new SetupCameraItem(d->listView, ctype);
        }
    }

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
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
    QTreeWidgetItem* item = d->listView->currentItem();

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
    CameraSelection* select = new CameraSelection;

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&,
                                           const QString&, const QString&)),
            this,   SLOT(slotAddedCamera(const QString&, const QString&,
                                         const QString&, const QString&)));

    select->show();
}

void SetupCamera::slotAddedCamera(const QString& title, const QString& model,
                                  const QString& port,  const QString& path)
{
    CameraType ctype(title, model, port, path, 1);
    new SetupCameraItem(d->listView, &ctype);
}

void SetupCamera::slotRemoveCamera()
{
    SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    if (item)
    {
        delete item;
    }
}

void SetupCamera::slotEditCamera()
{
    SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    if (!item)
    {
        return;
    }

    CameraType* ctype = item->cameraType();

    if (!ctype)
    {
        return;
    }

    CameraSelection* select = new CameraSelection;
    select->setCamera(ctype->title(), ctype->model(), ctype->port(), ctype->path());

    connect(select, SIGNAL(signalOkClicked(const QString&, const QString&,
                                           const QString&, const QString&)),
            this,   SLOT(slotEditedCamera(const QString&, const QString&,
                                          const QString&, const QString&)));

    select->show();
}

void SetupCamera::slotEditedCamera(const QString& title, const QString& model,
                                   const QString& port, const QString& path)
{
    SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(d->listView->currentItem());

    if (!item)
    {
        return;
    }

    CameraType ctype(title, model, port, path, 1);
    item->setCameraType(&ctype);
}

void SetupCamera::slotAutoDetectCamera()
{
    DBusyDlg* dlg                  = new DBusyDlg(i18n("Device detection under progress, please wait..."), this);
    CameraAutoDetectThread* thread = new CameraAutoDetectThread(this);
    dlg->setBusyThread(thread);
    dlg->exec();

    QString model = thread->model();
    QString port  = thread->port();
    int ret       = thread->result();

    if (ret != 0)
    {
        KMessageBox::error(this,i18n("Failed to auto-detect camera.\n"
                                     "Please check if your camera is turned on "
                                     "and retry or try setting it manually."));
        return;
    }

    // NOTE: See note in digikam/digikam/cameralist.cpp
    if (port.startsWith(QLatin1String("usb:")))
    {
        port = "usb:";
    }

    if (!d->listView->findItems(model, Qt::MatchExactly, 1).isEmpty())
    {
        KMessageBox::information(this, i18n("Camera '%1' (%2) is already in list.", model, port));
    }
    else
    {
        KMessageBox::information(this, i18n("Found camera '%1' (%2) and added it to the list.", model, port));
        slotAddedCamera(model, model, port, QString("/"));
    }
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
            SetupCameraItem* item = dynamic_cast<SetupCameraItem*>(*it);

            if (item)
            {
                CameraType* ctype = item->cameraType();

                if (ctype)
                {
                    clist->insert(new CameraType(*ctype));
                }
            }

            ++it;
        }

        clist->save();
    }
}

}  // namespace Digikam
