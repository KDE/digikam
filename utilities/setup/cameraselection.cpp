/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : Camera type selection dialog
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cameraselection.moc"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTreeWidget>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <kvbox.h>

// Local includes

#include "config-digikam.h"
#include "gpcamera.h"

namespace Digikam
{

class CameraSelection::Private
{
public:

    Private() :
        portButtonGroup(0),
        usbButton(0),
        serialButton(0),
        portPathLabel(0),
        portPathComboBox(0),
        listView(0),
        titleEdit(0),
        umsMountURL(0),
        searchBar(0)
    {
    }

    QButtonGroup*  portButtonGroup;

    QRadioButton*  usbButton;
    QRadioButton*  serialButton;

    QLabel*        portPathLabel;

    KComboBox*     portPathComboBox;

    QString        UMSCameraNameActual;
    QString        UMSCameraNameShown;
    QString        PTPCameraNameShown;

    QStringList    serialPortList;

    QTreeWidget*   listView;

    KLineEdit*     titleEdit;

    KUrlRequester* umsMountURL;

    SearchTextBar* searchBar;
};

CameraSelection::CameraSelection(QWidget* const parent)
    : KDialog(parent), d(new Private)
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    setHelp("cameraselection.anchor", "digikam");
    setCaption(i18n("Camera Configuration"));
    setButtons(KDialog::Help | KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    setModal(true);

    d->UMSCameraNameActual = QString("Directory Browse");   // Don't be i18n!
    d->UMSCameraNameShown  = i18n("Mounted Camera");
    d->PTPCameraNameShown  = QString("USB PTP Class Camera");

    setMainWidget(new QWidget(this));

    QGridLayout* mainBoxLayout = new QGridLayout(mainWidget());

    // --------------------------------------------------------------

    d->listView = new QTreeWidget(mainWidget());
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setMinimumWidth(350);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(1);
    d->listView->setHeaderLabels(QStringList() << i18n("Camera List"));
    d->listView->setWhatsThis(i18n("<p>Select the camera name that you want to use here. All "
                                   "default settings on the right panel "
                                   "will be set automatically.</p><p>This list has been generated "
                                   "using the gphoto2 library installed on your computer.</p>"));

    d->searchBar = new SearchTextBar(mainWidget(), "CameraSelectionSearchBar");

    // --------------------------------------------------------------

    QGroupBox* titleBox   = new QGroupBox(i18n("Camera Title"), mainWidget());
    QVBoxLayout* gLayout1 = new QVBoxLayout(titleBox);
    d->titleEdit          = new KLineEdit(titleBox);
    d->titleEdit->setWhatsThis(i18n("<p>Set here the name used in digiKam interface to "
                                    "identify this camera.</p>"));

    gLayout1->addWidget(d->titleEdit);
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------

    QGroupBox* portBox    = new QGroupBox(i18n("Camera Port Type"), mainWidget());
    QVBoxLayout* gLayout2 = new QVBoxLayout(portBox);
    d->portButtonGroup    = new QButtonGroup(portBox);
    d->portButtonGroup->setExclusive(true);

    d->usbButton = new QRadioButton(i18n("USB"), portBox);
    d->usbButton->setWhatsThis(i18n("<p>Select this option if your camera is connected to your "
                                    "computer using a USB cable.</p>"));

    d->serialButton = new QRadioButton(i18n("Serial"), portBox);
    d->serialButton->setWhatsThis(i18n("<p>Select this option if your camera is connected to your "
                                       "computer using a serial cable.</p>"));

    d->portButtonGroup->addButton(d->usbButton);
    d->portButtonGroup->addButton(d->serialButton);

    gLayout2->addWidget(d->usbButton);
    gLayout2->addWidget(d->serialButton);
    gLayout2->setMargin(KDialog::spacingHint());
    gLayout2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------

    QGroupBox* portPathBox = new QGroupBox(i18n("Camera Port Path"), mainWidget());
    QVBoxLayout* gLayout3  = new QVBoxLayout(portPathBox);

    d->portPathLabel = new QLabel(portPathBox);
    d->portPathLabel->setText(i18n("Note: only for serial port cameras."));

    d->portPathComboBox = new KComboBox(portPathBox);
    d->portPathComboBox->setDuplicatesEnabled(false);
    d->portPathComboBox->setWhatsThis(i18n("<p>Select the serial port to use on your computer here. "
                                           "This option is only required if you use a serial camera.</p>"));

    gLayout3->addWidget(d->portPathLabel);
    gLayout3->addWidget(d->portPathComboBox);
    gLayout3->setMargin(KDialog::spacingHint());
    gLayout3->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------

    QGroupBox* umsMountBox = new QGroupBox(i18n("Camera Mount Path"), mainWidget());
    QVBoxLayout* gLayout4  = new QVBoxLayout(umsMountBox);

    QLabel* umsMountLabel = new QLabel(umsMountBox);
    umsMountLabel->setText(i18n("Note: only for USB/IEEE mass storage cameras."));

    d->umsMountURL = new KUrlRequester(QString("/mnt/camera"), umsMountBox);
    d->umsMountURL->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    d->umsMountURL->setWhatsThis(i18n("<p>Set here the mount path to use on your computer. This "
                                      "option is only required if you use a <b>USB Mass Storage</b> "
                                      "camera.</p>"));

    gLayout4->addWidget(umsMountLabel);
    gLayout4->addWidget(d->umsMountURL);
    gLayout4->setMargin(KDialog::spacingHint());
    gLayout4->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------------

    QWidget* box2         = new QWidget(mainWidget());
    QGridLayout* gLayout5 = new QGridLayout(box2);

    QLabel* logo = new QLabel(box2);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* link = new QLabel(box2);
    link->setText(i18n("<p>To set a <b>USB Mass Storage</b> camera<br/>"
                       "(which looks like a removable drive when mounted<br/>"
                       "on your desktop), please use<br/>"
                       "<a href=\"umscamera\">%1</a> from the camera list.</p>",
                       d->UMSCameraNameShown));

    QLabel* link2 = new QLabel(box2);
    link2->setText(i18n("<p>To set a <b>Generic PTP USB Device</b><br/>"
                        "(which uses Picture Transfer Protocol), please<br/>"
                        "use <a href=\"ptpcamera\">%1</a> from the camera list.</p>",
                        d->PTPCameraNameShown));

    QLabel* explanation = new QLabel(box2);
    explanation->setOpenExternalLinks(true);
    explanation->setText(i18n("<p>A complete list of camera settings to use is<br/>"
                              "available at <a href='http://www.teaser.fr/~hfiguiere/linux/digicam.html'>"
                              "this URL</a>.</p>"));

    gLayout5->setMargin(KDialog::spacingHint());
    gLayout5->setSpacing(KDialog::spacingHint());
    gLayout5->addWidget(logo,        0, 0, 1, 1);
    gLayout5->addWidget(link,        0, 1, 2, 1);
    gLayout5->addWidget(link2,       2, 1, 2, 1);
    gLayout5->addWidget(explanation, 4, 1, 2, 1);

    // --------------------------------------------------------------

    mainBoxLayout->addWidget(d->listView,  0, 0, 6, 1);
    mainBoxLayout->addWidget(d->searchBar, 7, 0, 1, 1);
    mainBoxLayout->addWidget(titleBox,     0, 1, 1, 1);
    mainBoxLayout->addWidget(portBox,      1, 1, 1, 1);
    mainBoxLayout->addWidget(portPathBox,  2, 1, 1, 1);
    mainBoxLayout->addWidget(umsMountBox,  3, 1, 1, 1);
    mainBoxLayout->addWidget(box2,         4, 1, 2, 1);
    mainBoxLayout->setColumnStretch(0, 10);
    mainBoxLayout->setRowStretch(6, 10);
    mainBoxLayout->setSpacing(KDialog::spacingHint());
    mainBoxLayout->setMargin(0);

    // Connections --------------------------------------------------

    connect(link, SIGNAL(linkActivated(QString)),
            this, SLOT(slotUMSCameraLinkUsed()));

    connect(link2, SIGNAL(linkActivated(QString)),
            this, SLOT(slotPTPCameraLinkUsed()));

    connect(d->listView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem*,int)));

    connect(d->portButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotPortChanged()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    // Initialize  --------------------------------------------------

#ifndef HAVE_GPHOTO2
    // If digiKam is compiled without Gphoto2 support, we hide widgets relevant.
    d->listView->hide();
    d->searchBar->hide();
    box2->hide();
    slotUMSCameraLinkUsed();
#else
    getCameraList();
    getSerialPortList();
#endif /* HAVE_GPHOTO2 */

    kapp->restoreOverrideCursor();
}

CameraSelection::~CameraSelection()
{
    delete d;
}

void CameraSelection::slotUMSCameraLinkUsed()
{
    QList<QTreeWidgetItem*> list = d->listView->findItems(d->UMSCameraNameShown, Qt::MatchExactly, 0);

    if (!list.isEmpty())
    {
        QTreeWidgetItem* const item = list.first();

        if (item)
        {
            d->listView->setCurrentItem(item);
            d->listView->scrollToItem(item);
        }
    }
}

void CameraSelection::slotPTPCameraLinkUsed()
{
    QList<QTreeWidgetItem*> list = d->listView->findItems(d->PTPCameraNameShown, Qt::MatchExactly, 0);

    if (!list.isEmpty())
    {
        QTreeWidgetItem* const item = list.first();

        if (item)
        {
            d->listView->setCurrentItem(item);
            d->listView->scrollToItem(item);
        }
    }
}

void CameraSelection::setCamera(const QString& title, const QString& model,
                                const QString& port,  const QString& path)
{
    QString camModel(model);

    if (camModel == d->UMSCameraNameActual)
    {
        camModel = d->UMSCameraNameShown;
    }

    QList<QTreeWidgetItem*> list = d->listView->findItems(camModel, Qt::MatchExactly, 0);

    if (!list.isEmpty())
    {
        QTreeWidgetItem* item = list.first();

        if (!item)
        {
            return;
        }

        d->listView->setCurrentItem(item);
        d->listView->scrollToItem(item);

        d->titleEdit->setText(title);

        if (port.contains("usb"))
        {
            d->usbButton->setChecked(true);
            slotPortChanged();
        }
        else if (port.contains("serial"))
        {
            d->serialButton->setChecked(true);

            for (int i = 0 ; i < d->portPathComboBox->count() ; ++i)
            {
                if (port == d->portPathComboBox->itemText(i))
                {
                    d->portPathComboBox->setCurrentIndex(i);
                    break;
                }
            }

            slotPortChanged();
        }

        d->umsMountURL->setUrl(path);
    }
}

void CameraSelection::getCameraList()
{
    int count = 0;
    QStringList clist;
    QString cname;

    GPCamera::getSupportedCameras(count, clist);

    for (int i = 0 ; i < count ; ++i)
    {
        cname = clist.at(i);

        if (cname == d->UMSCameraNameActual)
        {
            new QTreeWidgetItem(d->listView, QStringList() << d->UMSCameraNameShown);
        }
        else
        {
            new QTreeWidgetItem(d->listView, QStringList() << cname);
        }
    }
}

void CameraSelection::getSerialPortList()
{
    QStringList plist;

    GPCamera::getSupportedPorts(plist);

    d->serialPortList.clear();

    for (int i = 0; i < plist.count() ; ++i)
    {
        if ((plist.at(i)).startsWith(QLatin1String("serial:")))
        {
            d->serialPortList.append(plist.at(i));
        }
    }
}

void CameraSelection::slotSelectionChanged(QTreeWidgetItem* item, int)
{
    if (!item)
    {
        return;
    }

    QString model(item->text(0));

    if (model == d->UMSCameraNameShown)
    {
        model = d->UMSCameraNameActual;

        d->titleEdit->setText(model);

        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(false);
        d->serialButton->setEnabled(false);
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(false);
        d->usbButton->setEnabled(false);
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->insertItem(0, QString("NONE"));
        d->portPathComboBox->setEnabled(false);

        d->umsMountURL->setEnabled(true);
        d->umsMountURL->clear();
        d->umsMountURL->setUrl(QString("/mnt/camera"));
        return;
    }
    else
    {
        d->umsMountURL->setEnabled(true);
        d->umsMountURL->clear();
        d->umsMountURL->setUrl(QString("/"));
        d->umsMountURL->setEnabled(false);
    }

    d->titleEdit->setText(model);

    QStringList plist;
    GPCamera::getCameraSupportedPorts(model, plist);

    if (plist.contains("serial"))
    {
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(true);
    }
    else
    {
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(false);
        d->serialButton->setEnabled(false);
    }

    if (plist.contains("usb"))
    {
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(true);
    }
    else
    {
        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(false);
        d->usbButton->setEnabled(false);
    }

    slotPortChanged();
}

void CameraSelection::slotPortChanged()
{
    if (d->usbButton->isChecked())
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItem(0, QString("usb:"));
        d->portPathComboBox->setEnabled(false);
        return;
    }

    if (d->serialButton->isChecked())
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItems(0, d->serialPortList);
    }
}

QString CameraSelection::currentTitle() const
{
    return d->titleEdit->text();
}

QString CameraSelection::currentModel() const
{
    QTreeWidgetItem* const item = d->listView->currentItem();

    if (!item)
    {
        return QString();
    }

    QString model(item->text(0));

    if (model == d->UMSCameraNameShown)
    {
        model = d->UMSCameraNameActual;
    }

    return model;
}

QString CameraSelection::currentPortPath() const
{
    return d->portPathComboBox->currentText();
}

QString CameraSelection::currentCameraPath() const
{
    return d->umsMountURL->url().toLocalFile();
}

void CameraSelection::slotOkClicked()
{
    emit signalOkClicked(currentTitle(),    currentModel(),
                         currentPortPath(), currentCameraPath());
}

void CameraSelection::slotSearchTextChanged(const SearchTextSettings& settings)
{
    bool query     = false;
    QString search = settings.text;

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        QTreeWidgetItem* const item  = *it;

        if (item->text(0).contains(search, settings.caseSensitive))
        {
            query = true;
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }

        ++it;
    }

    d->searchBar->slotSearchResult(query);
}

}  // namespace Digikam
