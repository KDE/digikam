/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-08
 * Description : A tab to display camera item informations
 *
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
 
#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "gpiteminfo.h"
#include "navigatebarwidget.h"
#include "cameraitempropertiestab.h"

namespace Digikam
{

class CameraItemPropertiesTabPriv
{
public:

    CameraItemPropertiesTabPriv()
    {
        navigateBar            = 0;
        labelFileName          = 0;
        labelFolder            = 0;
        labelFileIsReadable    = 0;
        labelFileIsWritable    = 0;
        labelFileDate          = 0;
        labelFileSize          = 0;
        labelFileMime          = 0;
        labelImageWidth        = 0;
        labelImageHeight       = 0;
        labelAlreadyDownloaded = 0;
    }

    KSqueezedTextLabel *labelFileName;
    KSqueezedTextLabel *labelFolder;
    KSqueezedTextLabel *labelFileIsReadable;
    KSqueezedTextLabel *labelFileIsWritable;
    KSqueezedTextLabel *labelFileDate;
    KSqueezedTextLabel *labelFileSize;
    KSqueezedTextLabel *labelFileMime;
    KSqueezedTextLabel *labelImageWidth;
    KSqueezedTextLabel *labelImageHeight;
    KSqueezedTextLabel *labelAlreadyDownloaded;
    
    NavigateBarWidget  *navigateBar;
};

CameraItemPropertiesTab::CameraItemPropertiesTab(QWidget* parent, bool navBar)
                       : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new CameraItemPropertiesTabPriv;
    QGridLayout *topLayout = new QGridLayout(this, 13, 2, KDialog::marginHint(), KDialog::spacingHint());

    d->navigateBar = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(d->navigateBar, 0, 0, 0, 2);

    QLabel *title      = new QLabel(i18n("Camera File Properties:"), this);
    QLabel *fileName   = new QLabel(i18n("<b>Name</b>:"), this);
    QLabel *folder     = new QLabel(i18n("<b>Folder</b>:"), this);
    QLabel *isReadable = new QLabel(i18n("<b>Readable</b>:"), this);
    QLabel *isWritable = new QLabel(i18n("<b>Writable</b>:"), this);
    QLabel *date       = new QLabel(i18n("<b>Date</b>:"), this);
    QLabel *size       = new QLabel(i18n("<b>Size</b>:"), this);
    QLabel *mime       = new QLabel(i18n("<b>Mime</b>:"), this);
    QLabel *width      = new QLabel(i18n("<b>Width</b>:"), this);
    QLabel *height     = new QLabel(i18n("<b>Height</b>:"), this);
    QLabel *downloaded = new QLabel(i18n("<b>Downloaded</b>:"), this);
                            
    d->labelFileName          = new KSqueezedTextLabel(0, this);
    d->labelFolder            = new KSqueezedTextLabel(0, this);
    d->labelFileIsReadable    = new KSqueezedTextLabel(0, this);
    d->labelFileIsWritable    = new KSqueezedTextLabel(0, this);
    d->labelFileDate          = new KSqueezedTextLabel(0, this);
    d->labelFileSize          = new KSqueezedTextLabel(0, this);
    d->labelFileMime          = new KSqueezedTextLabel(0, this);
    d->labelImageWidth        = new KSqueezedTextLabel(0, this);
    d->labelImageHeight       = new KSqueezedTextLabel(0, this);
    d->labelAlreadyDownloaded = new KSqueezedTextLabel(0, this);

    topLayout->addMultiCellWidget(title, 1, 1, 0, 2);
    topLayout->setRowStretch(2, 0);
    topLayout->addMultiCellWidget(fileName, 3, 3, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileName, 3, 3, 1, 2);
    topLayout->addMultiCellWidget(folder, 4, 4, 0, 0);
    topLayout->addMultiCellWidget(d->labelFolder, 4, 4, 1, 2);
    topLayout->addMultiCellWidget(isReadable, 5, 5, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileIsReadable, 5, 5, 1, 2);
    topLayout->addMultiCellWidget(isWritable, 6, 6, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileIsWritable, 6, 6, 1, 2);
    topLayout->addMultiCellWidget(date, 7, 7, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileDate, 7, 7, 1, 2);
    topLayout->addMultiCellWidget(size, 8, 8, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileSize, 8, 8, 1, 2);
    topLayout->addMultiCellWidget(mime, 9, 9, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileMime, 9, 9, 1, 2);
    topLayout->addMultiCellWidget(width, 10, 10, 0, 0);
    topLayout->addMultiCellWidget(d->labelImageWidth, 10, 10, 1, 2);
    topLayout->addMultiCellWidget(height, 11, 11, 0, 0);
    topLayout->addMultiCellWidget(d->labelImageHeight, 11, 11, 1, 2);
    topLayout->addMultiCellWidget(downloaded, 12, 12, 0, 0);
    topLayout->addMultiCellWidget(d->labelAlreadyDownloaded, 12, 12, 1, 2);
    topLayout->setRowStretch(13, 10);
            
    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));

    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
}

CameraItemPropertiesTab::~CameraItemPropertiesTab()
{
    delete d;
}

void CameraItemPropertiesTab::setCurrentItem(const GPItemInfo* itemInfo, int itemType)
{
    if (!itemInfo)
    {
        d->navigateBar->setFileName();
        d->labelFileName->setText(QString::null);
        d->labelFolder->setText(QString::null);
        d->labelFileIsReadable->setText(QString::null);
        d->labelFileIsWritable->setText(QString::null);
        d->labelFileDate->setText(QString::null);
        d->labelFileSize->setText(QString::null);
        d->labelFileMime->setText(QString::null);
        d->labelImageWidth->setText(QString::null);
        d->labelImageHeight->setText(QString::null);
        d->labelAlreadyDownloaded->setText(QString::null);
        setEnabled(false);
        return;
    }
    
    QString str;
    QDateTime date;

    setEnabled(true);

    d->navigateBar->setFileName(itemInfo->name);
    d->navigateBar->setButtonsState(itemType);

    d->labelFileName->setText(itemInfo->name);
    d->labelFolder->setText(itemInfo->folder);
    
    if (itemInfo->readPermissions < 0)
        str = i18n("Unknown");
    else if (itemInfo->readPermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");

    d->labelFileIsReadable->setText(str);
    
    if (itemInfo->writePermissions < 0)
        str = i18n("Unknown");
    else if (itemInfo->writePermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    d->labelFileIsWritable->setText(str);
    
    date.setTime_t(itemInfo->mtime);
    d->labelFileDate->setText(KGlobal::locale()->formatDateTime(date, true, true));
    
    str = i18n("%1 (%2)").arg(KIO::convertSize(itemInfo->size))
                         .arg(KGlobal::locale()->formatNumber(itemInfo->size, 0));
    d->labelFileSize->setText(str);
    
    d->labelFileMime->setText(itemInfo->mime);

    str = (itemInfo->width <= 0) ? i18n("Unknown") : QString::number(itemInfo->width);
    d->labelImageWidth->setText(str);
    
    str = (itemInfo->height <= 0) ? i18n("Unknown") : QString::number(itemInfo->height);
    d->labelImageHeight->setText(str);
    
    if (itemInfo->downloaded < 0)
        str = i18n("Unknown");
    else if (itemInfo->downloaded == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    d->labelAlreadyDownloaded->setText(str);
}
    
}  // NameSpace Digikam

#include "cameraitempropertiestab.moc"
