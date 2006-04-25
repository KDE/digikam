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
        labelFolder            = 0;
        labelFileIsReadable    = 0;
        labelFileIsWritable    = 0;
        labelFileDate          = 0;
        labelFileSize          = 0;
        labelImageMime         = 0;
        labelImageDimensions   = 0;
        labelNewFileName       = 0;
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
    
    QVBoxLayout *vLayout        = new QVBoxLayout(this);
    d->navigateBar              = new NavigateBarWidget(this, navBar);
    QWidget *settingsArea       = new QWidget(this);
    QGridLayout *settingsLayout = new QGridLayout(settingsArea, 18, 1, KDialog::marginHint(), 0);

    // --------------------------------------------------
        
    QLabel *title               = new QLabel(i18n("<u><i>Camera File Properties:</i></u>"), settingsArea);
    QLabel *folder              = new QLabel(i18n("<b>Folder</b>:"), settingsArea);
    QLabel *date                = new QLabel(i18n("<b>Date</b>:"), settingsArea);
    QLabel *size                = new QLabel(i18n("<b>Size</b>:"), settingsArea);
    QLabel *isReadable          = new QLabel(i18n("<b>Readable</b>:"), settingsArea);
    QLabel *isWritable          = new QLabel(i18n("<b>Writable</b>:"), settingsArea);
    
    KSeparator *line            = new KSeparator (Horizontal, settingsArea);
    QLabel *title2              = new QLabel(i18n("<u><i>Image Properties:</i></u>"), settingsArea);    
    QLabel *mime                = new QLabel(i18n("<b>Type</b>:"), settingsArea);
    QLabel *dimensions          = new QLabel(i18n("<b>Dimensions</b>:"), settingsArea);
    
    KSeparator *line2           = new KSeparator (Horizontal, settingsArea);
    QLabel *title3              = new QLabel(i18n("<u><i>Download Status:</i></u>"), settingsArea);
    QLabel *newFileName         = new QLabel(i18n("<nobr><b>New Name</b></nobr>:"), settingsArea);
    QLabel *downloaded          = new QLabel(i18n("<b>Downloaded</b>:"), settingsArea);
                            
    d->labelFolder              = new KSqueezedTextLabel(0, settingsArea);
    d->labelFileDate            = new KSqueezedTextLabel(0, settingsArea);
    d->labelFileSize            = new KSqueezedTextLabel(0, settingsArea);
    d->labelFileIsReadable      = new KSqueezedTextLabel(0, settingsArea);
    d->labelFileIsWritable      = new KSqueezedTextLabel(0, settingsArea);
    
    d->labelImageMime           = new KSqueezedTextLabel(0, settingsArea);
    d->labelImageDimensions     = new KSqueezedTextLabel(0, settingsArea);
    
    d->labelNewFileName         = new KSqueezedTextLabel(0, settingsArea);
    d->labelAlreadyDownloaded   = new KSqueezedTextLabel(0, settingsArea);

    // --------------------------------------------------
                            
    settingsLayout->addMultiCellWidget(title, 0, 0, 0, 1);
    settingsLayout->addMultiCellWidget(folder, 1, 1, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFolder, 1, 1, 1, 1);
    settingsLayout->addMultiCellWidget(date, 2, 2, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileDate, 2, 2, 1, 1);
    settingsLayout->addMultiCellWidget(size, 3, 3, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileSize, 3, 3, 1, 1);
    settingsLayout->addMultiCellWidget(isReadable, 4, 4, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileIsReadable, 4, 4, 1, 1);
    settingsLayout->addMultiCellWidget(isWritable, 5, 5, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileIsWritable, 5, 5, 1, 1);

    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 6, 6, 0, 1);    
    settingsLayout->addMultiCellWidget(line, 7, 7, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 8, 8, 0, 1);    

    settingsLayout->addMultiCellWidget(title2, 9, 9, 0, 1);                                
    settingsLayout->addMultiCellWidget(mime, 10, 10, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageMime, 10, 10, 1, 1);
    settingsLayout->addMultiCellWidget(dimensions, 11, 11, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageDimensions, 11, 11, 1, 1);

    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 12, 12, 0, 1);
    settingsLayout->addMultiCellWidget(line2, 13, 13, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 14, 14, 0, 1);  
    
    settingsLayout->addMultiCellWidget(title3, 15, 15, 0, 1);
    settingsLayout->addMultiCellWidget(newFileName, 16, 16, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelNewFileName, 16, 16, 1, 1);
    settingsLayout->addMultiCellWidget(downloaded, 17, 17, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelAlreadyDownloaded, 17, 17, 1, 1);
    
    settingsLayout->setRowStretch(18, 10);
    settingsLayout->setColStretch(1, 10);
    
    // --------------------------------------------------
    
    vLayout->addWidget(d->navigateBar);
    vLayout->addWidget(settingsArea);    
    
    // --------------------------------------------------            
    
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
    
    setEnabled(true);
    
    QString str;
    QString unknow(i18n("<i>unknow</i>"));
    
    d->navigateBar->setFileName(itemInfo->name);
    d->navigateBar->setButtonsState(itemType);
    
    // -- Camera file system informations ------------------------------------------
    
    d->labelFolder->setText(itemInfo->folder);
    
    if (itemInfo->readPermissions < 0)
        str = unknow;
    else if (itemInfo->readPermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");

    d->labelFileIsReadable->setText(str);
    
    if (itemInfo->writePermissions < 0)
        str = unknow;
    else if (itemInfo->writePermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    d->labelFileIsWritable->setText(str);
    
    QDateTime date;
    date.setTime_t(itemInfo->mtime);
    d->labelFileDate->setText(KGlobal::locale()->formatDateTime(date, true, true));
    
    str = i18n("%1 (%2)").arg(KIO::convertSize(itemInfo->size))
                         .arg(KGlobal::locale()->formatNumber(itemInfo->size, 0));
    d->labelFileSize->setText(str);
    
    // -- Image Properties --------------------------------------------------
    
    d->labelImageMime->setText( (itemInfo->mime == QString("image/x-raw")) ? 
                               i18n("RAW Image") : KMimeType::mimeType(itemInfo->mime)->comment() );

    QString mpixels;
    QSize dims(itemInfo->width, itemInfo->height);
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 1);
    str = (!dims.isValid()) ? unknow : i18n("%1x%2 (%3Mpx)")
          .arg(dims.width()).arg(dims.height()).arg(mpixels);
    d->labelImageDimensions->setText(str);
    
    // -- Download informations ------------------------------------------

    d->labelNewFileName->setText(newFileName);
    
    if (itemInfo->downloaded < 0)
        str = unknow;
    else if (itemInfo->downloaded == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    d->labelAlreadyDownloaded->setText(str);
}
    
}  // NameSpace Digikam

#include "cameraitempropertiestab.moc"
