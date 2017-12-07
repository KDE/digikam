/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a widget to edit EXIF metadata
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Victor Dodon <dodon dot victor at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "exifeditwidget.h"

// Qt includes

#include <QCloseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QMenu>
#include <QUrl>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "metadataedit.h"
#include "exifadjust.h"
#include "exifcaption.h"
#include "exifdatetime.h"
#include "exifdevice.h"
#include "exiflens.h"
#include "exiflight.h"
#include "digikam_debug.h"
#include "dmetadata.h"

namespace Digikam
{

class EXIFEditWidget::Private
{

public:

    Private()
    {
        modified      = false;
        isReadOnly    = false;
        page_caption  = 0;
        page_datetime = 0;
        page_lens     = 0;
        page_device   = 0;
        page_light    = 0;
        page_adjust   = 0;
        captionPage   = 0;
        datetimePage  = 0;
        lensPage      = 0;
        devicePage    = 0;
        lightPage     = 0;
        adjustPage    = 0;
        dlg           = 0;
    }

    bool                 modified;
    bool                 isReadOnly;

    QByteArray           exifData;
    QByteArray           iptcData;
    QByteArray           xmpData;

    DConfigDlgWdgItem*     page_caption;
    DConfigDlgWdgItem*     page_datetime;
    DConfigDlgWdgItem*     page_lens;
    DConfigDlgWdgItem*     page_device;
    DConfigDlgWdgItem*     page_light;
    DConfigDlgWdgItem*     page_adjust;

    EXIFCaption*         captionPage;
    EXIFDateTime*        datetimePage;
    EXIFLens*            lensPage;
    EXIFDevice*          devicePage;
    EXIFLight*           lightPage;
    EXIFAdjust*          adjustPage;

    MetadataEditDialog*  dlg;
};

EXIFEditWidget::EXIFEditWidget(MetadataEditDialog* const parent)
    : DConfigDlgWdg(parent),
      d(new Private)
{
    d->dlg           = parent;

    d->captionPage   = new EXIFCaption(this);
    d->page_caption  = addPage(d->captionPage, i18nc("image caption", "Caption"));
    d->page_caption->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));

    d->datetimePage  = new EXIFDateTime(this);
    d->page_datetime = addPage(d->datetimePage, i18n("Date & Time"));
    d->page_datetime->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));

    d->lensPage      = new EXIFLens(this);
    d->page_lens     = addPage(d->lensPage, i18n("Lens"));
    d->page_lens->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));

    d->devicePage    = new EXIFDevice(this);
    d->page_device   = addPage(d->devicePage, i18n("Device"));
    d->page_device->setIcon(QIcon::fromTheme(QLatin1String("scanner")));

    d->lightPage     = new EXIFLight(this);
    d->page_light    = addPage(d->lightPage, i18n("Light"));
    d->page_light->setIcon(QIcon::fromTheme(QLatin1String("view-preview")));

    d->adjustPage    = new EXIFAdjust(this);
    d->page_adjust   = addPage(d->adjustPage, i18nc("Picture adjustments", "Adjustments"));
    d->page_adjust->setIcon(QIcon::fromTheme(QLatin1String("fill-color")));

    // ------------------------------------------------------------

    connect(d->captionPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->datetimePage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->lensPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->devicePage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->lightPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->adjustPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    // ------------------------------------------------------------

    readSettings();
    slotItemChanged();
}

EXIFEditWidget::~EXIFEditWidget()
{
    delete d;
}

void EXIFEditWidget::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));
    showPage(group.readEntry(QLatin1String("All EXIF Edit Page"), 0));
    d->captionPage->setCheckedSyncJFIFComment(group.readEntry(QLatin1String("All Sync JFIF Comment"), true));
    d->captionPage->setCheckedSyncXMPCaption(group.readEntry(QLatin1String("All Sync XMP Caption"), true));
    d->captionPage->setCheckedSyncIPTCCaption(group.readEntry(QLatin1String("All Sync IPTC Caption"), true));
    d->datetimePage->setCheckedSyncXMPDate(group.readEntry(QLatin1String("All Sync XMP Date"), true));
    d->datetimePage->setCheckedSyncIPTCDate(group.readEntry(QLatin1String("All Sync IPTC Date"), true));
}

void EXIFEditWidget::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));
    group.writeEntry(QLatin1String("All EXIF Edit Page"),    activePageIndex());
    group.writeEntry(QLatin1String("All Sync JFIF Comment"), d->captionPage->syncJFIFCommentIsChecked());
    group.writeEntry(QLatin1String("All Sync XMP Caption"),  d->captionPage->syncXMPCaptionIsChecked());
    group.writeEntry(QLatin1String("All Sync IPTC Caption"), d->captionPage->syncIPTCCaptionIsChecked());
    group.writeEntry(QLatin1String("All Sync XMP Date"),     d->datetimePage->syncXMPDateIsChecked());
    group.writeEntry(QLatin1String("All Sync IPTC Date"),    d->datetimePage->syncIPTCDateIsChecked());
    config->sync();
}

void EXIFEditWidget::slotItemChanged()
{
    d->page_caption->setHeader(d->dlg->currentItemTitleHeader(i18n("Caption Information")));
    d->page_datetime->setHeader(d->dlg->currentItemTitleHeader(i18n("Date and Time Information")));
    d->page_lens->setHeader(d->dlg->currentItemTitleHeader(i18n("Lens Settings")));
    d->page_device->setHeader(d->dlg->currentItemTitleHeader(i18n("Capture Device Settings")));
    d->page_light->setHeader(d->dlg->currentItemTitleHeader(i18n("Light Source Information")));
    d->page_adjust->setHeader(d->dlg->currentItemTitleHeader(i18n("Pictures Adjustments")));

    DMetadata meta;
    meta.load((*d->dlg->currentItem()).toLocalFile());

    d->exifData = meta.getExifEncoded();
    d->iptcData = meta.getIptc();
    d->xmpData  = meta.getXmp();
    d->captionPage->readMetadata(d->exifData);
    d->datetimePage->readMetadata(d->exifData);
    d->lensPage->readMetadata(d->exifData);
    d->devicePage->readMetadata(d->exifData);
    d->lightPage->readMetadata(d->exifData);
    d->adjustPage->readMetadata(d->exifData);

    d->isReadOnly = !DMetadata::canWriteExif((*d->dlg->currentItem()).toLocalFile());
    emit signalSetReadOnly(d->isReadOnly);
    d->page_caption->setEnabled(!d->isReadOnly);
    d->page_datetime->setEnabled(!d->isReadOnly);
    d->page_lens->setEnabled(!d->isReadOnly);
    d->page_device->setEnabled(!d->isReadOnly);
    d->page_light->setEnabled(!d->isReadOnly);
    d->page_adjust->setEnabled(!d->isReadOnly);
}

void EXIFEditWidget::apply()
{
    if (d->modified && !d->isReadOnly)
    {
        d->captionPage->applyMetadata(d->exifData, d->iptcData, d->xmpData);
        d->datetimePage->applyMetadata(d->exifData, d->iptcData, d->xmpData);

        d->lensPage->applyMetadata(d->exifData);
        d->devicePage->applyMetadata(d->exifData);
        d->lightPage->applyMetadata(d->exifData);
        d->adjustPage->applyMetadata(d->exifData);

        DMetadata meta;

        meta.load((*d->dlg->currentItem()).toLocalFile());
        meta.setExif(d->exifData);
        meta.setIptc(d->iptcData);
        meta.setXmp(d->xmpData);
        meta.save((*d->dlg->currentItem()).toLocalFile());

        d->modified = false;
    }
}

void EXIFEditWidget::slotModified()
{
    if (!d->isReadOnly)
    {
        d->modified = true;
        emit signalModified();
    }
}

void EXIFEditWidget::showPage(int page)
{
    switch(page)
    {
        case 0:
            setCurrentPage(d->page_caption);
            break;
        case 1:
            setCurrentPage(d->page_datetime);
            break;
        case 2:
            setCurrentPage(d->page_lens);
            break;
        case 3:
            setCurrentPage(d->page_device);
            break;
        case 4:
            setCurrentPage(d->page_light);
            break;
        case 5:
            setCurrentPage(d->page_adjust);
            break;
        default:
            setCurrentPage(d->page_caption);
            break;
    }
}

int EXIFEditWidget::activePageIndex() const
{
    DConfigDlgWdgItem* const cur = currentPage();

    if (cur == d->page_caption)  return 0;
    if (cur == d->page_datetime) return 1;
    if (cur == d->page_lens)     return 2;
    if (cur == d->page_device)   return 3;
    if (cur == d->page_light)    return 4;
    if (cur == d->page_adjust)   return 5;

    return 0;
}

bool EXIFEditWidget::isModified() const
{
    return d->modified;
}

}  // namespace Digikam
