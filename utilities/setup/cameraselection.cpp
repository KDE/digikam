/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
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
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlineedit.h>

// KDE includes.

#include <klocale.h>

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
        listView  = 0;
        titleEdit = 0;
        portButtonGroup = 0;
        usbButton = 0;
        serialButton = 0;
        portPathLabel = 0;
        portPathComboBox = 0;
        umsMountComboBox = 0;
    }

    QListView*     listView;

    QLineEdit*     titleEdit;

    QVButtonGroup* portButtonGroup;

    QRadioButton*  usbButton;
    QRadioButton*  serialButton;

    QLabel*        portPathLabel;

    QComboBox*     portPathComboBox;
    QComboBox*     umsMountComboBox;

    QString        UMSCameraNameActual;
    QString        UMSCameraNameShown;

    QStringList    serialPortList;
};

CameraSelection::CameraSelection( QWidget* parent )
               : KDialogBase(parent, 0, true, i18n("Camera Selection"),
                             Help|Ok|Cancel, Ok, true)
{
    d = new CameraSelectionPriv;
    setHelp("cameraselection.anchor", "digikam");
    d->UMSCameraNameActual = QString("Directory Browse");   // Don't be i18n!
    d->UMSCameraNameShown  = i18n("Mounted Camera");

    QWidget *page = new QWidget( this );
    setMainWidget(page);
    
    QVBoxLayout *topLayout =
        new QVBoxLayout( page, 5, 5 ); 

    QGroupBox* mainBox = new QGroupBox( page );
    mainBox->setTitle( i18n( "Camera Configuration" ) );
    mainBox->setColumnLayout(0, Qt::Vertical );
    mainBox->layout()->setSpacing( 5 );
    mainBox->layout()->setMargin( 5 );
    QGridLayout* mainBoxLayout = new QGridLayout( mainBox->layout() );
    mainBoxLayout->setAlignment( Qt::AlignTop );

    d->listView = new QListView( mainBox );
    d->listView->addColumn( i18n("Cameras") );
    d->listView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding));
    mainBoxLayout->addMultiCellWidget( d->listView, 0, 4, 0, 0 );

    QGroupBox* titleBox = new QGroupBox( mainBox );
    titleBox->setTitle( i18n("Camera Title") );
    titleBox->setColumnLayout(0, Qt::Vertical );
    titleBox->layout()->setSpacing( 5 );
    titleBox->layout()->setMargin( 5 );
    QVBoxLayout* titleBoxLayout = new QVBoxLayout( titleBox->layout() );

    d->titleEdit = new QLineEdit( titleBox );
    titleBoxLayout->addWidget( d->titleEdit );

    mainBoxLayout->addWidget( titleBox, 0, 1 );
    
    d->portButtonGroup = new QVButtonGroup( mainBox );
    d->portButtonGroup->setTitle( i18n( "Camera Port Type" ) );
    d->portButtonGroup->setRadioButtonExclusive( true );
    d->portButtonGroup->layout()->setSpacing( 5 );
    d->portButtonGroup->layout()->setMargin( 5 );

    d->usbButton = new QRadioButton( d->portButtonGroup );
    d->usbButton->setText( i18n( "USB" ) );

    d->serialButton = new QRadioButton( d->portButtonGroup );
    d->serialButton->setText( i18n( "Serial" ) );

    mainBoxLayout->addWidget( d->portButtonGroup, 1, 1 );

    QGroupBox* portPathBox = new QGroupBox( mainBox );
    portPathBox->setTitle( i18n( "Camera Port Path" ) );
    portPathBox->setColumnLayout(0, Qt::Vertical );
    portPathBox->layout()->setSpacing( 5 );
    portPathBox->layout()->setMargin( 5 );
    QVBoxLayout* portPathBoxLayout = new QVBoxLayout( portPathBox->layout() );
    portPathBoxLayout->setAlignment( Qt::AlignTop );

    d->portPathLabel = new QLabel( portPathBox);
    d->portPathLabel->setText( i18n( "only for serial port\n"
			             "cameras" ) );
    portPathBoxLayout->addWidget( d->portPathLabel );

    d->portPathComboBox = new QComboBox( false, portPathBox );
    d->portPathComboBox->setDuplicatesEnabled( false );
    portPathBoxLayout->addWidget( d->portPathComboBox );

    mainBoxLayout->addWidget( portPathBox, 2, 1 );

    QGroupBox* umsMountBox = new QGroupBox( mainBox );
    umsMountBox->setColumnLayout(0, Qt::Vertical );
    umsMountBox->layout()->setSpacing( 5 );
    umsMountBox->layout()->setMargin( 5 );
    QVBoxLayout* umsMountBoxLayout = new QVBoxLayout( umsMountBox->layout() );
    umsMountBoxLayout->setAlignment( Qt::AlignTop );

    QLabel* umsMountLabel = new QLabel( umsMountBox );
    umsMountLabel->setText( i18n( "only for USB/IEEE mass storage\n"
                                  "cameras" ) );
    umsMountBoxLayout->addWidget( umsMountLabel );

    d->umsMountComboBox = new QComboBox( false, umsMountBox );
    umsMountBox->setTitle( i18n( "Camera Mount Path" ) );
    d->umsMountComboBox->setEditable( true );
    d->umsMountComboBox->setInsertionPolicy( QComboBox::AtTop );
    d->umsMountComboBox->setDuplicatesEnabled( false );
    umsMountBoxLayout->addWidget( d->umsMountComboBox );

    mainBoxLayout->addWidget( umsMountBox, 3, 1 );

    QSpacerItem* spacer = new QSpacerItem( 20, 20,
                                           QSizePolicy::Minimum,
                                           QSizePolicy::Expanding );
    mainBoxLayout->addItem( spacer, 4, 1 );

    topLayout->addWidget( mainBox );

    
    // Connections --------------------------------------------------

    connect(d->listView, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(slotSelectionChanged(QListViewItem *)));

    connect(d->portButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(slotPortChanged()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()));
    
    // Initialize  --------------------------------------------------

    getCameraList();
    getSerialPortList();
}

CameraSelection::~CameraSelection()
{
    delete d;
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

    d->umsMountComboBox->setCurrentText(path);
}

void CameraSelection::getCameraList()
{
    int count = 0;
    QStringList clist;

    GPIface::getSupportedCameras(count, clist);

    QString cname;
    for (int i=0; i<count; i++) 
    {
        cname = clist[i];
        if (cname == d->UMSCameraNameActual)
            new QListViewItem(d->listView, d->UMSCameraNameShown);
        else
            new QListViewItem(d->listView, cname);
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

        d->umsMountComboBox->setEnabled(true);
        d->umsMountComboBox->clear();
        d->umsMountComboBox->insertItem(QString("/mnt/camera"), 0);
        return;
    }
    else 
    {
        d->umsMountComboBox->setEnabled(true);
        d->umsMountComboBox->clear();
        d->umsMountComboBox->insertItem(QString("/"), 0);
        d->umsMountComboBox->setEnabled(false);
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
    return d->umsMountComboBox->currentText();
}

void CameraSelection::slotOkClicked()
{
    emit signalOkClicked(currentTitle(),    currentModel(),
                         currentPortPath(), currentCameraPath());
}

}  // namespace Digikam

#include "cameraselection.moc"
