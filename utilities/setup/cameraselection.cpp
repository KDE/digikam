/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qwhatsthis.h>

// KDE includes.

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

    QStringList    serialPortList;
    
    KLineEdit     *titleEdit;
    
    KListView     *listView;

    KURLRequester *umsMountURL;
};

CameraSelection::CameraSelection( QWidget* parent )
               : KDialogBase(parent, 0, true, i18n("Camera Selection"),
                             Help|Ok|Cancel, Ok, true)
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    d = new CameraSelectionPriv;
    setHelp("cameraselection.anchor", "digikam");
    d->UMSCameraNameActual = QString("Directory Browse");   // Don't be i18n!
    d->UMSCameraNameShown  = i18n("Mounted Camera");

    QWidget *page = new QWidget( this );
    setMainWidget(page);
    
    QVBoxLayout *topLayout = new QVBoxLayout( page ); 

    // --------------------------------------------------------------

    QGroupBox* mainBox = new QGroupBox( 0, Qt::Vertical, i18n( "Camera Configuration" ), page );
    mainBox->setInsideMargin(KDialogBase::marginHint());
    mainBox->setInsideSpacing(KDialogBase::spacingHint());  
    QGridLayout* mainBoxLayout = new QGridLayout( mainBox->layout(), 6, 1, KDialog::spacingHint() );
    
    // --------------------------------------------------------------

    d->listView = new KListView( mainBox );
    d->listView->addColumn( i18n("Camera List") );
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setResizeMode(KListView::LastColumn);
    d->listView->setMinimumWidth(350);
    QWhatsThis::add( d->listView, i18n("<p>Select here the camera name that you want to use. All default settings on the right "
                                       "will be set automaticly.</p><p>This list have been generated "
                                       "using gphoto2 library installed in your computer.</p>"));
    
    // --------------------------------------------------------------

    QGroupBox* titleBox = new QGroupBox( 1, Qt::Vertical, i18n("Camera Title"), mainBox );
    d->titleEdit = new KLineEdit( titleBox );
    QWhatsThis::add( d->titleEdit, i18n("<p>Set here the name used in digiKam interface to indentify this camera.</p>"));
    
    // --------------------------------------------------------------
    
    d->portButtonGroup = new QVButtonGroup( i18n("Camera Port Type"), mainBox );
    d->portButtonGroup->setRadioButtonExclusive( true );

    d->usbButton = new QRadioButton( d->portButtonGroup );
    d->usbButton->setText( i18n( "USB" ) );
    QWhatsThis::add( d->usbButton, i18n("<p>Select this option if your camera is connected to your computer using an USB cable.</p>"));

    d->serialButton = new QRadioButton( d->portButtonGroup );
    d->serialButton->setText( i18n( "Serial" ) );
    QWhatsThis::add( d->serialButton, i18n("<p>Select this option if your camera is connected to your computer using a serial cable.</p>"));

    // --------------------------------------------------------------
    
    QGroupBox* portPathBox = new QGroupBox( 1, Qt::Horizontal, i18n( "Camera Port Path" ), mainBox );
    d->portPathLabel = new QLabel( portPathBox);
    d->portPathLabel->setText( i18n( "Note: only for serial port camera" ) );

    d->portPathComboBox = new QComboBox( false, portPathBox );
    d->portPathComboBox->setDuplicatesEnabled( false );
    QWhatsThis::add( d->portPathComboBox, i18n("<p>Select here the serial port to use on your computer. This option is only "
                                               "require if you use a serial camera.</p>"));

    // --------------------------------------------------------------

    QGroupBox* umsMountBox = new QGroupBox( 1, Qt::Horizontal, i18n( "Camera Mount Path"), mainBox );

    QLabel* umsMountLabel = new QLabel( umsMountBox );
    umsMountLabel->setText( i18n( "Note: only for USB/IEEE mass storage camera" ) );

    d->umsMountURL = new KURLRequester( QString("/mnt/camera"), umsMountBox);
    d->umsMountURL->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    QWhatsThis::add( d->umsMountURL, i18n("<p>Set here the mount path to use on your computer. This option is only "
                                          "require if you use an <b>Usb Mass Storage</b> camera.</p>"));
    
    // --------------------------------------------------------------
    
    KActiveLabel* link = new KActiveLabel(mainBox);
    link->setText(i18n("<p>To set an <b>Usb Mass Storage</b> camera (which appears like a "
                       "removable drive), please use <a href=\"umscamera\">%1</a> from camera list.</p>")
                       .arg(d->UMSCameraNameShown));
    
    KActiveLabel* explanation = new KActiveLabel(mainBox);
    explanation->setText(i18n("<p>To see a fresh list of supported cameras, take a look at "
                              "<a href='http://www.teaser.fr/~hfiguiere/linux/digicam.html'>this url</a>.</p>"));    

    // --------------------------------------------------------------
    
    mainBoxLayout->addMultiCellWidget( d->listView, 0, 6, 0, 0 );
    mainBoxLayout->addMultiCellWidget( titleBox, 0, 0, 1, 1 );
    mainBoxLayout->addMultiCellWidget( d->portButtonGroup, 1, 1, 1, 1 );
    mainBoxLayout->addMultiCellWidget( portPathBox, 2, 2, 1, 1 );
    mainBoxLayout->addMultiCellWidget( umsMountBox, 3, 3, 1, 1 );
    mainBoxLayout->addMultiCellWidget( link, 4, 4, 1, 1 );
    mainBoxLayout->addMultiCellWidget( explanation, 5, 5, 1, 1 );
    mainBoxLayout->setColStretch( 0, 10 );
    mainBoxLayout->setRowStretch( 6, 10 );

    topLayout->addWidget( mainBox );

    // Connections --------------------------------------------------

    disconnect(link, SIGNAL(linkClicked(const QString &)),
               link, SLOT(openLink(const QString &)));
    
    connect(link, SIGNAL(linkClicked(const QString &)),
            this, SLOT(slotUMSCameraLinkUsed()));
    
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
        return QString::null;

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
