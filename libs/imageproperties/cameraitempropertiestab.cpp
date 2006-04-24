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
#include <kmimetype.h>
#include <kdebug.h>
#include <kseparator.h>
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
        labelNewFileName       = 0;
        labelFolder            = 0;
        labelFileIsReadable    = 0;
        labelFileIsWritable    = 0;
        labelFileDate          = 0;
        labelFileSize          = 0;
        labelImageMime         = 0;
        labelImageDimensions   = 0;
        labelAlreadyDownloaded = 0;
    }

    KSqueezedTextLabel *labelFolder;
    KSqueezedTextLabel *labelFileIsReadable;
    KSqueezedTextLabel *labelFileIsWritable;
    KSqueezedTextLabel *labelFileDate;
    KSqueezedTextLabel *labelFileSize;
    
    KSqueezedTextLabel *labelImageMime;
    KSqueezedTextLabel *labelImageDimensions;
    
    KSqueezedTextLabel *labelNewFileName;
    KSqueezedTextLabel *labelAlreadyDownloaded;
    
    NavigateBarWidget  *navigateBar;
};

CameraItemPropertiesTab::CameraItemPropertiesTab(QWidget* parent, bool navBar)
                       : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new CameraItemPropertiesTabPriv;
    QGridLayout *topLayout = new QGridLayout(this, 19, 1, KDialog::marginHint(), 0);

    d->navigateBar = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(d->navigateBar, 0, 0, 0, 1);

    QLabel *title             = new QLabel(i18n("<u><i>Camera File Properties:</i></u>"), this);
    QLabel *folder            = new QLabel(i18n("<b>Folder</b>:"), this);
    QLabel *date              = new QLabel(i18n("<b>Date</b>:"), this);
    QLabel *size              = new QLabel(i18n("<b>Size</b>:"), this);
    QLabel *isReadable        = new QLabel(i18n("<b>Readable</b>:"), this);
    QLabel *isWritable        = new QLabel(i18n("<b>Writable</b>:"), this);
    
    KSeparator *line          = new KSeparator (Horizontal, this);
    QLabel *title2            = new QLabel(i18n("<u><i>Image Properties:</i></u>"), this);    
    QLabel *mime              = new QLabel(i18n("<b>Type</b>:"), this);
    QLabel *dimensions        = new QLabel(i18n("<b>Dimensions</b>:"), this);
    
    KSeparator *line2         = new KSeparator (Horizontal, this);
    QLabel *title3            = new QLabel(i18n("<u><i>Download Status:</i></u>"), this);
    QLabel *newFileName       = new QLabel(i18n("<nobr><b>New Name</b></nobr>:"), this);
    QLabel *downloaded        = new QLabel(i18n("<b>Downloaded</b>:"), this);
                            
    d->labelFolder            = new KSqueezedTextLabel(0, this);
    d->labelFileDate          = new KSqueezedTextLabel(0, this);
    d->labelFileSize          = new KSqueezedTextLabel(0, this);
    d->labelFileIsReadable    = new KSqueezedTextLabel(0, this);
    d->labelFileIsWritable    = new KSqueezedTextLabel(0, this);
    
    d->labelImageMime         = new KSqueezedTextLabel(0, this);
    d->labelImageDimensions   = new KSqueezedTextLabel(0, this);
    
    d->labelNewFileName       = new KSqueezedTextLabel(0, this);
    d->labelAlreadyDownloaded = new KSqueezedTextLabel(0, this);

    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 0, 0, 0, 1);
                            
    topLayout->addMultiCellWidget(title, 1, 1, 0, 1);
    topLayout->addMultiCellWidget(folder, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(d->labelFolder, 2, 2, 1, 1);
    topLayout->addMultiCellWidget(date, 3, 3, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileDate, 3, 3, 1, 1);
    topLayout->addMultiCellWidget(size, 4, 4, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileSize, 4, 4, 1, 1);
    topLayout->addMultiCellWidget(isReadable, 5, 5, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileIsReadable, 5, 5, 1, 1);
    topLayout->addMultiCellWidget(isWritable, 6, 6, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileIsWritable, 6, 6, 1, 1);

    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 7, 7, 0, 1);    
    topLayout->addMultiCellWidget(line, 8, 8, 0, 1);
    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 9, 9, 0, 1);    

    topLayout->addMultiCellWidget(title2, 10, 10, 0, 1);                                
    topLayout->addMultiCellWidget(mime, 11, 11, 0, 0);
    topLayout->addMultiCellWidget(d->labelImageMime, 11, 11, 1, 1);
    topLayout->addMultiCellWidget(dimensions, 12, 12, 0, 0);
    topLayout->addMultiCellWidget(d->labelImageDimensions, 12, 12, 1, 1);

    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 13, 13, 0, 1);
    topLayout->addMultiCellWidget(line2, 14, 14, 0, 1);
    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 15, 15, 0, 1);  
    
    topLayout->addMultiCellWidget(title3, 16, 16, 0, 1);
    topLayout->addMultiCellWidget(newFileName, 17, 17, 0, 0);
    topLayout->addMultiCellWidget(d->labelNewFileName, 17, 17, 1, 1);
    topLayout->addMultiCellWidget(downloaded, 18, 18, 0, 0);
    topLayout->addMultiCellWidget(d->labelAlreadyDownloaded, 18, 18, 1, 1);
    
    topLayout->setRowStretch(19, 10);
    topLayout->setColStretch(1, 10);
            
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

void CameraItemPropertiesTab::setCurrentItem(const GPItemInfo* itemInfo, int itemType, 
                                             const QString &newFileName)
{
    if (!itemInfo)
    {
        d->navigateBar->setFileName();
        
        d->labelFolder->setText(QString::null);
        d->labelFileIsReadable->setText(QString::null);
        d->labelFileIsWritable->setText(QString::null);
        d->labelFileDate->setText(QString::null);
        d->labelFileSize->setText(QString::null);
        
        d->labelImageMime->setText(QString::null);
        d->labelImageDimensions->setText(QString::null);
        
        d->labelNewFileName->setText(QString::null);
        d->labelAlreadyDownloaded->setText(QString::null);
        
        setEnabled(false);
        return;
    }
    
    QString str;
    QDateTime date;

    setEnabled(true);

    d->navigateBar->setFileName(itemInfo->name);
    d->navigateBar->setButtonsState(itemType);

    d->labelNewFileName->setText(newFileName);
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
    
    d->labelImageMime->setText( (itemInfo->mime == QString("image/x-raw")) ? 
                               i18n("RAW Image") : KMimeType::mimeType(itemInfo->mime)->comment() );

    QString mpixels;
    QSize dims(itemInfo->width, itemInfo->height);
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 1);
    str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
          .arg(dims.width()).arg(dims.height()).arg(mpixels);
    d->labelImageDimensions->setText(str);
    
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
