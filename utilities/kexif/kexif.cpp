/***************************************************************************
                          kexif.cpp  -  description
                             -------------------
    begin                : Tue Aug 27 18:41:58 CDT 2002
    copyright            : (C) 2002 by Renchi Raju
    email                : renchi@green.tam.uiuc.edu

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtabwidget.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qtextedit.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qapplication.h>

#include <klocale.h>
#include <klistview.h>
#include <kurl.h>

#include "kexififd.h"
#include "kexifentry.h"
#include "kexifdata.h"
#include "kexiflistview.h"
#include "kexiflistviewitem.h"

#include "kexif.h"

KExif::KExif(QWidget *parent, const char *name, WFlags fl)
    : QWidget(parent, name, fl|Qt::WShowModal)
{
    int W=400, H=400;
    resize(W, H);
    move(QApplication::desktop()->width ()/2-(W/2), QApplication::desktop()->height()/2-(H/2));
    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(6);
    layout->setMargin(6);

    // ------------------------------------------------------

    mMainBox = new QGroupBox(1, Qt::Vertical, this);
    layout->addWidget(mMainBox, 0, 0);

    mTabWidget = new QTabWidget(mMainBox);

    connect(mTabWidget, SIGNAL(currentChanged(QWidget*)),
           this, SLOT(slot_tabPageChanged(QWidget*)));

    // ------------------------------------------------------

    QGroupBox *textBox = new QGroupBox(1, Qt::Vertical, this);
    layout->addWidget(textBox, 1, 0);

    mTextEdit = new QTextEdit(textBox);
    mTextEdit->setReadOnly(true);
    mTextEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Minimum));
    textBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Minimum));

    // ------------------------------------------------------

    QButtonGroup *buttonGroup = new QButtonGroup(1, Qt::Horizontal,
                                                 this);
    layout->addWidget(buttonGroup, 2, 0);

    QPushButton *okButton = new QPushButton(i18n("&Close"),
                                            buttonGroup);
    okButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Fixed));
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(slot_close()));

    okButton->setFocus();

    // ------------------------------------------------------


    mExifData = 0;

}

KExif::~KExif()
{
    if (mExifData)
        delete mExifData;
}

int KExif::loadFile(const QString& filename)
{
    if (filename.isNull()) return -1;
    mFileName = QString(filename);

    mMainBox->setTitle(KURL(filename).filename());

    if (mExifData) {
        delete mExifData;
        mExifData = 0;
    }

    mExifData = new KExifData;
    int result = mExifData->readFromFile(filename);

    if (result == KExifData::SUCCESS) {

        QValueVector<KExifIfd>::iterator ifdIterator;
        for (ifdIterator = mExifData->ifdVector.begin();
             ifdIterator != mExifData->ifdVector.end();
             ++ifdIterator) {

            KExifListView* listview = new KExifListView(mTabWidget);
            listview->addItems((*ifdIterator).entryList);
            mTabWidget->addTab(listview, (*ifdIterator).getName());

            connect(listview, SIGNAL(signal_itemDescription(const QString&)),
                    this, SLOT(slot_showItemDescription(const QString&)));
        }

        QImage thumbnail;
        if (mExifData->getThumbnail(thumbnail) == KExifData::SUCCESS) {
            QWidget* widget = new QWidget(mTabWidget);
            mTabWidget->addTab(widget,i18n("Thumbnail"));
            QGridLayout* layout = new QGridLayout(widget);
            QLabel* label = new QLabel(widget);
            label->setFixedSize(thumbnail.size());
            label->setPixmap(QPixmap(thumbnail));
            layout->addWidget(label, 0, 0);
        }

        return 0;

    }

    return -1;

}

int KExif::loadData(const QString& filename, char *data, int size)
{
    if (data == 0 || size == 0) {
        qWarning("KEXIF:: Invalid data");
        return -1;
    }
    mMainBox->setTitle(filename);

    if (mExifData) {
        delete mExifData;
        mExifData = 0;
    }

    mExifData = new KExifData;
    int result = mExifData->readFromData(data, size);

    if (result == KExifData::SUCCESS) {

        QValueVector<KExifIfd>::iterator ifdIterator;
        for (ifdIterator = mExifData->ifdVector.begin();
             ifdIterator != mExifData->ifdVector.end();
             ++ifdIterator) {

            KExifListView* listview = new KExifListView(mTabWidget);
            listview->addItems((*ifdIterator).entryList);
            mTabWidget->addTab(listview, (*ifdIterator).getName());

            connect(listview, SIGNAL(signal_itemDescription(const QString&)),
                    this, SLOT(slot_showItemDescription(const QString&)));
        }

        QImage thumbnail;
        if (mExifData->getThumbnail(thumbnail) == KExifData::SUCCESS) {
            QWidget* widget = new QWidget(mTabWidget);
            mTabWidget->addTab(widget,i18n("Thumbnail"));
            QGridLayout* layout = new QGridLayout(widget);
            QLabel* label = new QLabel(widget);
            label->setFixedSize(thumbnail.size());
            label->setPixmap(QPixmap(thumbnail));
            layout->addWidget(label, 0, 0);
        }

        return 0;

    }

    
    qWarning("Failed to load exif from data");
    return -1;
    
}

void KExif::slot_showItemDescription(const QString& desc)
{
    if (desc.isEmpty())
        mTextEdit->setText(i18n("Select an item to see its description"));
    else
        mTextEdit->setText(desc);
}

void KExif::slot_tabPageChanged(QWidget*)
{
    slot_showItemDescription(QString(""));
}

void KExif::slot_close()
{
    close();
}

#include "kexif.moc"
