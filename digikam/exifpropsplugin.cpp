/* ============================================================
 * File  : exifproposplugin.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-11-13
 * Description : an image exif viewer dialog.
 * 
 * Copyright 2004 by Gilles Caulier
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
 
#include <qlayout.h>
#include <qptrlist.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kselect.h>
#include <kdialogbase.h>

// LibKexif includes.

#include <libkexif/kexififd.h>
#include <libkexif/kexifentry.h>
#include <libkexif/kexifdata.h>
#include <libkexif/kexiflistview.h>
#include <libkexif/kexiflistviewitem.h>

// Local includes.

#include "exifpropsplugin.h"

ExifPropsPlugin::ExifPropsPlugin( KPropertiesDialog *propsDlg, QString fileName )
               : KPropsDlgPlugin(propsDlg)
{    
    mExifData = new KExifData;

    if (mExifData->readFromFile(fileName))
        setupGui(propsDlg);
}

ExifPropsPlugin::~ExifPropsPlugin()
{
    if (mExifData)
        delete mExifData;
}

void ExifPropsPlugin::setupGui(KPropertiesDialog *dialog)
{
    QPtrList<KExifIfd> ifdList(mExifData->ifdList());

    QFrame *page = dialog->addPage( i18n("Exif") );
    QGridLayout* layout = new QGridLayout(page);
    KExifListView* listview = new KExifListView(page, true);
    
    QGroupBox *textBox = new QGroupBox(1, Qt::Vertical, page);
    QTextEdit *textEdit = new QTextEdit(textBox);
    textEdit->setReadOnly(true);
    textEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    textBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    textEdit->setText(i18n("Select an item to see its description"));
    
    layout->addWidget(listview, 0, 0);
    layout->addWidget(textBox, 1, 0);
    
    for (KExifIfd* ifd = ifdList.first(); ifd; ifd = ifdList.next())
        {
        KExifListViewItem *item = new KExifListViewItem(listview, listview->lastItem(), ifd->getName());
        listview->addItems(ifd->entryList(), item );
        }
        
    connect(listview, SIGNAL(signal_itemDescription(const QString&)),
            textEdit, SLOT(setText(const QString&)));

    // Exif embedded thumbnails listview entry creation.
    
    QImage thumbnail(mExifData->getThumbnail());
        
    if (!thumbnail.isNull()) 
        {
        KExifListViewItem *item = new KExifListViewItem(listview, listview->lastItem(), i18n("Thumbnail"));
        KExifListViewItem *item2 = new KExifListViewItem(item, listview->lastItem(), "");
        item2->setPixmap(1, QPixmap(thumbnail));
        }        
}


#include "exifpropsplugin.moc"
