/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : Camera type selection dialog
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cameraselection.h"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTreeWidget>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QLineEdit>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dfileselector.h"
#include "digikam_config.h"
#include "gpcamera.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class CameraSelection::Private
{
public:

    Private() :
        buttons(0),
        portButtonGroup(0),
        usbButton(0),
        serialButton(0),
        networkButton(0),
        portPathComboBox(0),
        listView(0),
        titleEdit(0),
        networkEdit(0),
        umsMountURL(0),
        searchBar(0)
    {
    }

    QDialogButtonBox* buttons;

    QButtonGroup*     portButtonGroup;

    QRadioButton*     usbButton;
    QRadioButton*     serialButton;
    QRadioButton*     networkButton;

    QComboBox*        portPathComboBox;

    QString           UMSCameraNameActual;
    QString           UMSCameraNameShown;
    QString           PTPCameraNameShown;
    QString           PTPIPCameraNameShown;

    QStringList       serialPortList;

    QTreeWidget*      listView;

    QLineEdit*        titleEdit;
    QLineEdit*        networkEdit;

    DFileSelector*    umsMountURL;

    SearchTextBar*    searchBar;
};

CameraSelection::CameraSelection(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    qApp->setOverrideCursor(Qt::WaitCursor);

    setWindowTitle(i18n("Camera Configuration"));
    setModal(true);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->buttons        = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    d->UMSCameraNameActual  = QLatin1String("Directory Browse");   // Don't be i18n!
    d->UMSCameraNameShown   = i18n("Mounted Camera");
    d->PTPCameraNameShown   = QLatin1String("USB PTP Class Camera");
    d->PTPIPCameraNameShown = QLatin1String("PTP/IP Camera");

    QWidget* const page        = new QWidget(this);
    QGridLayout* mainBoxLayout = new QGridLayout(page);

    // --------------------------------------------------------------

    d->listView = new QTreeWidget(page);
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

    d->searchBar = new SearchTextBar(page, QLatin1String("CameraSelectionSearchBar"));

    // --------------------------------------------------------------

    QGroupBox* const titleBox   = new QGroupBox(i18n("Camera Title"), page);
    QVBoxLayout* const gLayout1 = new QVBoxLayout(titleBox);
    d->titleEdit                = new QLineEdit(titleBox);
    d->titleEdit->setWhatsThis(i18n("<p>Set here the name used in digiKam interface to "
                                    "identify this camera.</p>"));

    gLayout1->addWidget(d->titleEdit);
    gLayout1->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout1->setSpacing(spacing);

    // --------------------------------------------------------------

    QGroupBox* const portPathBox = new QGroupBox(i18n("Camera Port Type"), page);
    QGridLayout* const gLayout2  = new QGridLayout(portPathBox);
    d->portButtonGroup           = new QButtonGroup(portPathBox);
    d->portButtonGroup->setExclusive(true);

    d->usbButton        = new QRadioButton(i18n("USB"), portPathBox);
    d->usbButton->setWhatsThis(i18n("<p>Select this option if your camera is connected to your "
                                    "computer using a USB cable.</p>"));

    d->serialButton     = new QRadioButton(i18n("Serial"), portPathBox);
    d->serialButton->setWhatsThis(i18n("<p>Select this option if your camera is connected to your "
                                       "computer using a serial cable.</p>"));

    d->networkButton    = new QRadioButton(i18n("Network"), portPathBox);
    d->networkButton->setWhatsThis(i18n("<p>Select this option if your camera is connected to your "
                                        "computer network.</p>"));

    d->portPathComboBox = new QComboBox(portPathBox);
    d->portPathComboBox->setDuplicatesEnabled(false);
    d->portPathComboBox->setWhatsThis(i18n("<p>Select the serial port to use on your computer here. "
                                           "This option is only required if you use a serial camera.</p>"));

    d->networkEdit      = new QLineEdit(portPathBox);
    d->networkEdit->setWhatsThis(i18n("<p>Enter here the network address of your camera.</p>"));
    d->networkEdit->setInputMask(QLatin1String("000.000.000.000"));
    d->networkEdit->setText(QLatin1String("192.168.001.001"));

    d->portButtonGroup->addButton(d->usbButton);
    d->portButtonGroup->addButton(d->serialButton);
    d->portButtonGroup->addButton(d->networkButton);

    gLayout2->addWidget(d->usbButton,        0, 0, 1, 2);
    gLayout2->addWidget(d->serialButton,     1, 0, 1, 2);
    gLayout2->addWidget(d->portPathComboBox, 1, 1, 1, 2);
    gLayout2->addWidget(d->networkButton,    2, 0, 1, 2);
    gLayout2->addWidget(d->networkEdit,      2, 1, 1, 2);
    gLayout2->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout2->setSpacing(spacing);

    // --------------------------------------------------------------

    QGroupBox* const umsMountBox = new QGroupBox(i18n("Camera Mount Path"), page);
    QVBoxLayout* const gLayout3  = new QVBoxLayout(umsMountBox);

    QLabel* const umsMountLabel = new QLabel(umsMountBox);
    umsMountLabel->setText(i18n("Note: only for USB/IEEE mass storage cameras."));

    d->umsMountURL = new DFileSelector(umsMountBox);
    d->umsMountURL->setFileDlgPath(QLatin1String("/mnt/camera"));
    d->umsMountURL->setFileDlgMode(DFileDialog::Directory);
    d->umsMountURL->setWhatsThis(i18n("<p>Set here the mount path to use on your computer. This "
                                      "option is only required if you use a <b>USB Mass Storage</b> "
                                      "camera.</p>"));

    gLayout3->addWidget(umsMountLabel);
    gLayout3->addWidget(d->umsMountURL);
    gLayout3->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout3->setSpacing(spacing);

    // --------------------------------------------------------------

    QWidget* const box2         = new QWidget(page);
    QGridLayout* const gLayout4 = new QGridLayout(box2);

    QLabel* const logo = new QLabel(box2);
    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    QLabel* const link = new QLabel(box2);
    link->setText(i18n("<p>To set a <b>USB Mass Storage</b> camera<br/>"
                       "(which looks like a removable drive when mounted<br/>"
                       "on your desktop), please use<br/>"
                       "<a href=\"umscamera\">%1</a> from the camera list.</p>",
                       d->UMSCameraNameShown));

    QLabel* const link2 = new QLabel(box2);
    link2->setText(i18n("<p>To set a <b>Generic PTP USB Device</b><br/>"
                        "(which uses Picture Transfer Protocol), please<br/>"
                        "use <a href=\"ptpcamera\">%1</a> from the camera list.</p>",
                        d->PTPCameraNameShown));

    QLabel* const link3 = new QLabel(box2);
    link3->setText(i18n("<p>To set a <b>Generic PTP/IP Network Device</b><br/>"
                        "(which uses Picture Transfer Protocol), please<br/>"
                        "use <a href=\"ptpipcamera\">%1</a> from the camera list.</p>",
                        d->PTPIPCameraNameShown));

    QLabel* const explanation = new QLabel(box2);
    explanation->setOpenExternalLinks(true);
    explanation->setText(i18n("<p>A complete list of camera settings to use is<br/>"
                              "available at <a href='http://www.teaser.fr/~hfiguiere/linux/digicam.html'>"
                              "this URL</a>.</p>"));

    gLayout4->setContentsMargins(spacing, spacing, spacing, spacing);
    gLayout4->setSpacing(spacing);
    gLayout4->addWidget(logo,        0, 0, 1, 1);
    gLayout4->addWidget(link,        0, 1, 2, 1);
    gLayout4->addWidget(link2,       2, 1, 2, 1);
    gLayout4->addWidget(link3,       4, 1, 2, 1);
    gLayout4->addWidget(explanation, 6, 1, 2, 1);

    // --------------------------------------------------------------

    mainBoxLayout->addWidget(d->listView,  0, 0, 6, 1);
    mainBoxLayout->addWidget(d->searchBar, 7, 0, 1, 1);
    mainBoxLayout->addWidget(titleBox,     0, 1, 1, 1);
    mainBoxLayout->addWidget(portPathBox,  1, 1, 1, 1);
    mainBoxLayout->addWidget(umsMountBox,  2, 1, 1, 1);
    mainBoxLayout->addWidget(box2,         3, 1, 2, 1);
    mainBoxLayout->setColumnStretch(0, 10);
    mainBoxLayout->setRowStretch(6, 10);
    mainBoxLayout->setContentsMargins(QMargins());
    mainBoxLayout->setSpacing(spacing);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    // Connections --------------------------------------------------

    connect(link, SIGNAL(linkActivated(QString)),
            this, SLOT(slotUMSCameraLinkUsed()));

    connect(link2, SIGNAL(linkActivated(QString)),
            this, SLOT(slotPTPCameraLinkUsed()));

    connect(link3, SIGNAL(linkActivated(QString)),
            this, SLOT(slotPTPIPCameraLinkUsed()));

    connect(d->networkEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotNetworkEditChanged(QString)));

    connect(d->listView, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotSelectionChanged(QTreeWidgetItem*,int)));

    connect(d->portButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotPortChanged()));

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOkClicked()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

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

    qApp->restoreOverrideCursor();
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

void CameraSelection::slotPTPIPCameraLinkUsed()
{
    QList<QTreeWidgetItem*> list = d->listView->findItems(d->PTPIPCameraNameShown, Qt::MatchExactly, 0);

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

void CameraSelection::slotNetworkEditChanged(const QString& text)
{
    int cursorPosition   = d->networkEdit->cursorPosition();
    QStringList ipRanges = text.split(QLatin1Char('.'));

    for (int i = 0; i < ipRanges.count(); ++i)
    {
        bool ok;

        for (int a = ipRanges.at(i).count(); a < 3; ++a)
        {
            ipRanges[i].append(QLatin1Char('0'));
        }

        if (ipRanges.at(i).toInt(&ok) > 255)
        {
            ipRanges[i] = QLatin1String("255");
        }

        if (!ok)
        {
            ipRanges[i] = QLatin1String("000");
        }
    }

    d->networkEdit->setText(ipRanges.join(QLatin1Char('.')));
    d->networkEdit->setCursorPosition(cursorPosition);
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
        QTreeWidgetItem* const item = list.first();

        if (!item)
        {
            return;
        }

        d->listView->setCurrentItem(item);
        d->listView->scrollToItem(item);

        d->titleEdit->setText(title);

        if (port.contains(QLatin1String("usb")))
        {
            d->usbButton->setChecked(true);
            slotPortChanged();
        }
        else if (port.contains(QLatin1String("serial")))
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
        else if (port.contains(QLatin1String("ptpip")))
        {
            d->networkButton->setChecked(true);
            d->networkEdit->setText(port);
            slotPortChanged();
        }

        d->umsMountURL->setFileDlgPath(path);
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

        d->usbButton->setEnabled(true);
        d->usbButton->setChecked(false);
        d->usbButton->setEnabled(false);
        d->serialButton->setEnabled(true);
        d->serialButton->setChecked(false);
        d->serialButton->setEnabled(false);
        d->networkButton->setEnabled(true);
        d->networkButton->setChecked(false);
        d->networkButton->setEnabled(false);
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItem(0, QLatin1String("NONE"));
        d->portPathComboBox->setEnabled(false);
        d->networkEdit->setEnabled(true);
        d->networkEdit->setText(QLatin1String("192.168.001.001"));
        d->networkEdit->setEnabled(false);

        d->umsMountURL->setEnabled(true);
        d->umsMountURL->setFileDlgPath(QLatin1String("/mnt/camera"));
        return;
    }
    else
    {
        d->umsMountURL->setEnabled(true);
        d->umsMountURL->setFileDlgPath(QLatin1String("/"));
        d->umsMountURL->setEnabled(false);
    }

    d->titleEdit->setText(model);

    QStringList plist;
    GPCamera::getCameraSupportedPorts(model, plist);

    if (plist.contains(QLatin1String("serial")))
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

    if (plist.contains(QLatin1String("ptpip")))
    {
        d->networkButton->setEnabled(true);
        d->networkButton->setChecked(true);
    }
    else
    {
        d->networkButton->setEnabled(true);
        d->networkButton->setChecked(false);
        d->networkButton->setEnabled(false);
    }

    if (plist.contains(QLatin1String("usb")))
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
        d->portPathComboBox->insertItem(0, QLatin1String("usb:"));
        d->portPathComboBox->setEnabled(false);
        d->networkEdit->setEnabled(false);
        return;
    }

    if (d->networkButton->isChecked())
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItem(0, QLatin1String("ptpip:"));
        d->portPathComboBox->setEnabled(false);
        d->networkEdit->setEnabled(true);
        return;
    }

    if (d->serialButton->isChecked())
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertItems(0, d->serialPortList);
        d->networkEdit->setEnabled(false);
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
    if (d->networkButton->isChecked())
    {
        return (d->portPathComboBox->currentText() +
                d->networkEdit->text());
    }
    else
    {
        return d->portPathComboBox->currentText();
    }
}

QString CameraSelection::currentCameraPath() const
{
    return d->umsMountURL->fileDlgPath();
}

void CameraSelection::slotOkClicked()
{
    emit signalOkClicked(currentTitle(),    currentModel(),
                         currentPortPath(), currentCameraPath());
    accept();
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

void CameraSelection::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

} // namespace Digikam
