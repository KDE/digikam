/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2003-02-10
 * Description : Camera type selection dialog
 * 
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kactivelabel.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <klistview.h>
#include <klineedit.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes.

#include "cameraselection.h"
#include "gpiface.h"

namespace Digikam
{

class CameraSelectionPriv
{
public:

    CameraSelectionPriv()
    {
        listView         = 0;
        titleEdit        = 0;
        portButtonGroup  = 0;
        usbButton        = 0;
        serialButton     = 0;
        portPathLabel    = 0;
        portPathComboBox = 0;
        umsMountURL      = 0;
    }

    QVButtonGroup *portButtonGroup;

    QRadioButton  *usbButton;
    QRadioButton  *serialButton;

    QLabel        *portPathLabel;

    QComboBox     *portPathComboBox;

    QString        UMSCameraNameActual;
    QString        UMSCameraNameShown;
    QString        PTPCameraNameShown;

    QStringList    serialPortList;
    
    KLineEdit     *titleEdit;
    
    KListView     *listView;

    KURLRequester *umsMountURL;
};

CameraSelection::CameraSelection( QWidget* parent )
               : KDialogBase(Plain, i18n("Camera Configuration"),
                             Help|Ok|Cancel, Ok, parent, 0, true, true)
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    d = new CameraSelectionPriv;
    setHelp("cameraselection.anchor", "digikam");
    d->UMSCameraNameActual = QString("Directory Browse");   // Don't be i18n!
    d->UMSCameraNameShown  = i18n("Mounted Camera");
    d->PTPCameraNameShown  = QString("USB PTP Class Camera");

    QGridLayout* mainBoxLayout = new QGridLayout( plainPage(), 6, 1, 0, KDialog::spacingHint() );
    mainBoxLayout->setColStretch( 0, 10 );
    mainBoxLayout->setRowStretch( 6, 10 );
    
    // --------------------------------------------------------------

    d->listView = new KListView( plainPage() );
    d->listView->addColumn( i18n("Camera List") );
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setResizeMode(KListView::LastColumn);
    d->listView->setMinimumWidth(350);
    QWhatsThis::add( d->listView, i18n("<p>Select here the camera name that you want to use. All "
                                       "default settings on the right panel "
                                       "will be set automatically.</p><p>This list has been generated "
                                       "using the gphoto2 library installed on your computer.</p>"));
    
    // --------------------------------------------------------------

    QVGroupBox* titleBox = new QVGroupBox( i18n("Camera Title"), plainPage() );
    d->titleEdit = new KLineEdit( titleBox );
    QWhatsThis::add( d->titleEdit, i18n("<p>Set here the name used in digiKam interface to "
                                        "identify this camera.</p>"));
    
    // --------------------------------------------------------------
    
    d->portButtonGroup = new QVButtonGroup( i18n("Camera Port Type"), plainPage() );
    d->portButtonGroup->setRadioButtonExclusive( true );

    d->usbButton = new QRadioButton( d->portButtonGroup );
    d->usbButton->setText( i18n( "USB" ) );
    QWhatsThis::add( d->usbButton, i18n("<p>Select this option if your camera is connected to your "
                     "computer using an USB cable.</p>"));

    d->serialButton = new QRadioButton( d->portButtonGroup );
    d->serialButton->setText( i18n( "Serial" ) );
    QWhatsThis::add( d->serialButton, i18n("<p>Select this option if your camera is connected to your "
                     "computer using a serial cable.</p>"));

    // --------------------------------------------------------------
    
    QVGroupBox* portPathBox = new QVGroupBox( i18n( "Camera Port Path" ), plainPage() );
    d->portPathLabel = new QLabel( portPathBox);
    d->portPathLabel->setText( i18n( "Note: only for serial port camera" ) );

    d->portPathComboBox = new QComboBox( false, portPathBox );
    d->portPathComboBox->setDuplicatesEnabled( false );
    QWhatsThis::add( d->portPathComboBox, i18n("<p>Select here the serial port to use on your computer. "
                     "This option is only required if you use a serial camera.</p>"));

    // --------------------------------------------------------------

    QVGroupBox* umsMountBox = new QVGroupBox( i18n( "Camera Mount Path"), plainPage() );

    QLabel* umsMountLabel = new QLabel( umsMountBox );
    umsMountLabel->setText( i18n( "Note: only for USB/IEEE mass storage camera" ) );

    d->umsMountURL = new KURLRequester( QString("/mnt/camera"), umsMountBox);
    d->umsMountURL->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    QWhatsThis::add( d->umsMountURL, i18n("<p>Set here the mount path to use on your computer. This "
                                          "option is only required if you use an <b>USB Mass Storage</b> "
                                          "camera.</p>"));
    
    // --------------------------------------------------------------
    
    QGroupBox* box2 = new QGroupBox( 0, Qt::Vertical, plainPage() );
    box2->setFrameStyle( QFrame::NoFrame );
    QGridLayout* box2Layout = new QGridLayout( box2->layout(), 1, 5 );

    QLabel* logo = new QLabel( box2 );

    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 64, 
                    KIcon::DefaultState, 0, true));

    KActiveLabel* link = new KActiveLabel(box2);
    link->setText(i18n("<p>To set an <b>USB Mass Storage</b> camera<br>"
                       "(which appears like a removable drive), please<br>"
                       "use <a href=\"umscamera\">%1</a> from camera list.</p>") 
                       .arg(d->UMSCameraNameShown));

    KActiveLabel* link2 = new KActiveLabel(box2);
    link2->setText(i18n("<p>To set a <b>Generic PTP USB Device</b><br>"
                        "(which use Picture Transfer Protocol), please<br>"
                        "use <a href=\"ptpcamera\">%1</a> from camera list.</p>")
                        .arg(d->PTPCameraNameShown));
                           
    KActiveLabel* explanation = new KActiveLabel(box2);
    explanation->setText(i18n("<p>A complete list of camera settings to use is<br>"
                              "available at <a href='http://www.teaser.fr/~hfiguiere/linux/digicam.html'>"
                              "this url</a>.</p>"));

    box2Layout->addMultiCellWidget( logo, 0, 0, 0, 0 );
    box2Layout->addMultiCellWidget( link, 0, 1, 1, 1 );
    box2Layout->addMultiCellWidget( link2, 2, 3, 1, 1 );
    box2Layout->addMultiCellWidget( explanation, 4, 5, 1, 1 );

    // --------------------------------------------------------------
    
    mainBoxLayout->addMultiCellWidget( d->listView, 0, 6, 0, 0 );
    mainBoxLayout->addMultiCellWidget( titleBox, 0, 0, 1, 1 );
    mainBoxLayout->addMultiCellWidget( d->portButtonGroup, 1, 1, 1, 1 );
    mainBoxLayout->addMultiCellWidget( portPathBox, 2, 2, 1, 1 );
    mainBoxLayout->addMultiCellWidget( umsMountBox, 3, 3, 1, 1 );
    mainBoxLayout->addMultiCellWidget( box2, 4, 5, 1, 1 );

    // Connections --------------------------------------------------

    disconnect(link, SIGNAL(linkClicked(const QString &)),
               link, SLOT(openLink(const QString &)));
    
    connect(link, SIGNAL(linkClicked(const QString &)),
            this, SLOT(slotUMSCameraLinkUsed()));

    disconnect(link2, SIGNAL(linkClicked(const QString &)),
               link2, SLOT(openLink(const QString &)));
    
    connect(link2, SIGNAL(linkClicked(const QString &)),
            this, SLOT(slotPTPCameraLinkUsed()));
                
    connect(d->listView, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(slotSelectionChanged(QListViewItem *)));

    connect(d->portButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(slotPortChanged()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()));
    
    // Initialize  --------------------------------------------------
    
    getCameraList();
    getSerialPortList();
    kapp->restoreOverrideCursor();
}

CameraSelection::~CameraSelection()
{
    delete d;
}

void CameraSelection::slotUMSCameraLinkUsed()
{
    QListViewItem *item = d->listView->findItem(d->UMSCameraNameShown, 0);
    if (item)
    {
        d->listView->setCurrentItem(item);
        d->listView->ensureItemVisible(item);
    }
}

void CameraSelection::slotPTPCameraLinkUsed()
{
    QListViewItem *item = d->listView->findItem(d->PTPCameraNameShown, 0);
    if (item)
    {
        d->listView->setCurrentItem(item);
        d->listView->ensureItemVisible(item);
    }
}

void CameraSelection::setCamera(const QString& title, const QString& model,
                                const QString& port, const QString& path)
{
    QString camModel(model);

    if (camModel == d->UMSCameraNameActual)
        camModel = d->UMSCameraNameShown;

    QListViewItem* item = d->listView->findItem(camModel, 0);
    if (!item) return;

    d->listView->setSelected(item, true);
    d->listView->ensureItemVisible(item);
    
    d->titleEdit->setText(title);

    if (port.contains("usb"))
        d->usbButton->setChecked(true);
    else if (port.contains("serial")) 
    {
        d->serialButton->setChecked(true);

        for (int i=0; i<d->portPathComboBox->count(); i++) 
        {
            if (port == d->portPathComboBox->text(i)) 
            {
                d->portPathComboBox->setCurrentItem(i);
                break;
            }
        }
    }

    d->umsMountURL->setURL(path);
}

void CameraSelection::getCameraList()
{
    int count = 0;
    QStringList clist;
    QString cname;
    
    GPIface::getSupportedCameras(count, clist);
    
    for (int i = 0 ; i < count ; i++) 
    {
        cname = clist[i];
        if (cname == d->UMSCameraNameActual)
            new KListViewItem(d->listView, d->UMSCameraNameShown);
        else
            new KListViewItem(d->listView, cname);
    }
}

void CameraSelection::getSerialPortList()
{
    QStringList plist;

    GPIface::getSupportedPorts(plist);

    d->serialPortList.clear();
    
    for (unsigned int i=0; i<plist.count(); i++) 
    {
        if ((plist[i]).startsWith("serial:"))
            d->serialPortList.append(plist[i]);
    }
}

void CameraSelection::slotSelectionChanged(QListViewItem *item)
{
    if (!item) return;

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
        d->portPathComboBox->insertItem(QString("NONE"), 0);
        d->portPathComboBox->setEnabled(false);

        d->umsMountURL->setEnabled(true);
        d->umsMountURL->clear();
        d->umsMountURL->setURL(QString("/mnt/camera"));
        return;
    }
    else 
    {
        d->umsMountURL->setEnabled(true);
        d->umsMountURL->clear();
        d->umsMountURL->setURL(QString("/"));
        d->umsMountURL->setEnabled(false);
    }

    d->titleEdit->setText(model);
    
    QStringList plist;
    GPIface::getCameraSupportedPorts(model, plist);

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
        d->portPathComboBox->insertItem( QString("usb:"), 0 );
        d->portPathComboBox->setEnabled(false);
        return;
    }

    if (d->serialButton->isChecked()) 
    {
        d->portPathComboBox->setEnabled(true);
        d->portPathComboBox->clear();
        d->portPathComboBox->insertStringList(d->serialPortList);
    }
}

QString CameraSelection::currentTitle()
{
    return d->titleEdit->text();    
}

QString CameraSelection::currentModel()
{
    QListViewItem* item = d->listView->currentItem();
    if (!item)
        return QString();

    QString model(item->text(0));
    if (model == d->UMSCameraNameShown)
        model = d->UMSCameraNameActual;

    return model;
}

QString CameraSelection::currentPortPath()
{
    return d->portPathComboBox->currentText();
}

QString CameraSelection::currentCameraPath()
{
    return d->umsMountURL->url();
}

void CameraSelection::slotOkClicked()
{
    emit signalOkClicked(currentTitle(),    currentModel(),
                         currentPortPath(), currentCameraPath());
}

}  // namespace Digikam

#include "cameraselection.moc"
