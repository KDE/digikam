/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-10
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#include <klocale.h>

#include "cameraselection.h"
#include "gpiface.h"

CameraSelection::CameraSelection( QWidget* parent )
    : KDialogBase(parent, 0, true, i18n("Camera Selection"),
                  Help|Ok|Cancel, Ok, true)
{
    setHelp("cameraselection.anchor", "digikam");
    UMSCameraNameActual_ = QString("Directory Browse");
    UMSCameraNameShown_  = QString("USB Mass Storage");

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

    listView_ = new QListView( mainBox );
    listView_->addColumn( i18n("Cameras") );
    listView_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding));
    mainBoxLayout->addMultiCellWidget( listView_, 0, 4, 0, 0 );

    QGroupBox* titleBox = new QGroupBox( mainBox );
    titleBox->setTitle( i18n("Camera Title") );
    titleBox->setColumnLayout(0, Qt::Vertical );
    titleBox->layout()->setSpacing( 5 );
    titleBox->layout()->setMargin( 5 );
    QVBoxLayout* titleBoxLayout = new QVBoxLayout( titleBox->layout() );

    titleEdit_ = new QLineEdit( titleBox );
    titleBoxLayout->addWidget( titleEdit_ );

    mainBoxLayout->addWidget( titleBox, 0, 1 );
    
    portButtonGroup_ = new QVButtonGroup( mainBox );
    portButtonGroup_->setTitle( i18n( "Camera Port Type" ) );
    portButtonGroup_->setRadioButtonExclusive( true );
    portButtonGroup_->layout()->setSpacing( 5 );
    portButtonGroup_->layout()->setMargin( 5 );

    usbButton_ = new QRadioButton( portButtonGroup_ );
    usbButton_->setText( i18n( "USB" ) );

    serialButton_ = new QRadioButton( portButtonGroup_ );
    serialButton_->setText( i18n( "Serial" ) );

    mainBoxLayout->addWidget( portButtonGroup_, 1, 1 );

    QGroupBox* portPathBox = new QGroupBox( mainBox );
    portPathBox->setTitle( i18n( "Camera Port Path" ) );
    portPathBox->setColumnLayout(0, Qt::Vertical );
    portPathBox->layout()->setSpacing( 5 );
    portPathBox->layout()->setMargin( 5 );
    QVBoxLayout* portPathBoxLayout = new QVBoxLayout( portPathBox->layout() );
    portPathBoxLayout->setAlignment( Qt::AlignTop );

    QLabel* portPathLabel_ = new QLabel( portPathBox);
    portPathLabel_->setText( i18n( "only for serial port\n"
			    "cameras" ) );
    portPathBoxLayout->addWidget( portPathLabel_ );

    portPathComboBox_ = new QComboBox( false, portPathBox );
    portPathComboBox_->setDuplicatesEnabled( FALSE );
    portPathBoxLayout->addWidget( portPathComboBox_ );

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

    umsMountComboBox_ = new QComboBox( FALSE, umsMountBox );
    umsMountBox->setTitle( i18n( "Camera Mount Path" ) );
    umsMountComboBox_->setEditable( TRUE );
    umsMountComboBox_->setInsertionPolicy( QComboBox::AtTop );
    umsMountComboBox_->setDuplicatesEnabled( FALSE );
    umsMountBoxLayout->addWidget( umsMountComboBox_ );

    mainBoxLayout->addWidget( umsMountBox, 3, 1 );

    QSpacerItem* spacer = new QSpacerItem( 20, 20,
                                           QSizePolicy::Minimum,
                                           QSizePolicy::Expanding );
    mainBoxLayout->addItem( spacer, 4, 1 );


    topLayout->addWidget( mainBox );

    
    // Connections --------------------------------------------------

    connect(listView_, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(slotSelectionChanged(QListViewItem *)));

    connect(portButtonGroup_, SIGNAL(clicked(int)),
            this, SLOT(slotPortChanged()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()));
    
    // Initialize  --------------------------------------------------

    getCameraList();
    getSerialPortList();

}

CameraSelection::~CameraSelection()
{
}

void CameraSelection::setCamera(const QString& title, const QString& model,
                                const QString& port, const QString& path)
{
    QString camModel(model);
    if (camModel == UMSCameraNameActual_)
        camModel = UMSCameraNameShown_;
    QListViewItem* item = listView_->findItem(camModel, 0);
    if (!item) return;

    listView_->setSelected(item, true);
    listView_->ensureItemVisible(item);
    
    titleEdit_->setText(title);

    if (port.contains("usb"))
        usbButton_->setChecked(true);
    else if (port.contains("serial")) {
        serialButton_->setChecked(true);
        for (int i=0; i<portPathComboBox_->count(); i++) {
            if (port == portPathComboBox_->text(i)) {
                portPathComboBox_->setCurrentItem(i);
                break;
            }
        }
    }

    umsMountComboBox_->setCurrentText(path);
}

void CameraSelection::getCameraList()
{
    int count = 0;
    QStringList clist;

    GPIface::getSupportedCameras(count, clist);

    QString cname;
    for (int i=0; i<count; i++) {
        cname = clist[i];
        if (cname == UMSCameraNameActual_)
            new QListViewItem(listView_, UMSCameraNameShown_);
        else
            new QListViewItem(listView_, cname);
    }
}

void CameraSelection::getSerialPortList()
{
    QStringList plist;

    GPIface::getSupportedPorts(plist);

    serialPortList_.clear();
    for (unsigned int i=0; i<plist.count(); i++) {
        if ((plist[i]).startsWith("serial:"))
            serialPortList_.append(plist[i]);
    }
}

void CameraSelection::slotSelectionChanged(QListViewItem *item)
{
    if (!item) return;

    QString model(item->text(0));
    if (model == UMSCameraNameShown_) {

        model = UMSCameraNameActual_;

        titleEdit_->setText(model);
        
        serialButton_->setEnabled(true);
        serialButton_->setChecked(false);
        serialButton_->setEnabled(false);
        usbButton_->setEnabled(true);
        usbButton_->setChecked(false);
        usbButton_->setEnabled(false);
        portPathComboBox_->setEnabled(true);
        portPathComboBox_->insertItem(QString("NONE"), 0);
        portPathComboBox_->setEnabled(false);

        umsMountComboBox_->setEnabled(true);
        umsMountComboBox_->clear();
        umsMountComboBox_->insertItem(QString("/mnt/camera"), 0);
        return;
    }
    else {

        umsMountComboBox_->setEnabled(true);
        umsMountComboBox_->clear();
        umsMountComboBox_->insertItem(QString("/"), 0);
        umsMountComboBox_->setEnabled(false);
    }

    titleEdit_->setText(model);
    
    QStringList plist;
    GPIface::getCameraSupportedPorts(model, plist);

    if (plist.contains("serial")) {
        serialButton_->setEnabled(true);
        serialButton_->setChecked(true);
    }
    else {
        serialButton_->setEnabled(true);
        serialButton_->setChecked(false);
        serialButton_->setEnabled(false);
    }

    if (plist.contains("usb")) {
        usbButton_->setEnabled(true);
        usbButton_->setChecked(true);
    }
    else {
        usbButton_->setEnabled(true);
        usbButton_->setChecked(false);
        usbButton_->setEnabled(false);
    }

    slotPortChanged();

}

void CameraSelection::slotPortChanged()
{
    if (usbButton_->isChecked()) {
        portPathComboBox_->setEnabled(true);
        portPathComboBox_->clear();
        portPathComboBox_->insertItem( QString("usb:"), 0 );
        portPathComboBox_->setEnabled(false);
        return;
    }

    if (serialButton_->isChecked()) {
        portPathComboBox_->setEnabled(true);
        portPathComboBox_->clear();
        portPathComboBox_->insertStringList(serialPortList_);
    }
}

QString CameraSelection::currentTitle()
{
    return titleEdit_->text();    
}

QString CameraSelection::currentModel()
{
    QListViewItem* item = listView_->currentItem();
    if (!item)
        return QString::null;

    QString model(item->text(0));
    if (model == UMSCameraNameShown_)
        model = UMSCameraNameActual_;

    return model;
        
}

QString CameraSelection::currentPortPath()
{
    return portPathComboBox_->currentText();
}

QString CameraSelection::currentCameraPath()
{
    return umsMountComboBox_->currentText();
}

void CameraSelection::slotOkClicked()
{
    emit signalOkClicked(currentTitle(), currentModel(),
                         currentPortPath(), currentCameraPath());
}

#include "cameraselection.moc"
