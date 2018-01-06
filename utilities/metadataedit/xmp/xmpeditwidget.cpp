/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-10-11
 * Description : a DConfigDlgWdg to edit XMP metadata
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "xmpeditwidget.h"

// Qt includes

#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QApplication>
#include <QUrl>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "metadataedit.h"
#include "xmpcategories.h"
#include "xmpcontent.h"
#include "xmpcredits.h"
#include "xmpkeywords.h"
#include "xmporigin.h"
#include "xmpproperties.h"
#include "xmpstatus.h"
#include "xmpsubjects.h"
#include "dmetadata.h"

namespace Digikam
{

class XMPEditWidget::Private
{

public:

    Private()
    {
        modified        = false;
        isReadOnly      = false;
        page_content    = 0;
        page_properties = 0;
        page_subjects   = 0;
        page_keywords   = 0;
        page_categories = 0;
        page_credits    = 0;
        page_status     = 0;
        page_origin     = 0;
        keywordsPage    = 0;
        categoriesPage  = 0;
        contentPage     = 0;
        subjectsPage    = 0;
        originPage      = 0;
        creditsPage     = 0;
        statusPage      = 0;
        propertiesPage  = 0;
        dlg             = 0;
    }

    bool                  modified;
    bool                  isReadOnly;

    QByteArray            exifData;
    QByteArray            iptcData;
    QByteArray            xmpData;

    DConfigDlgWdgItem*      page_content;
    DConfigDlgWdgItem*      page_origin;
    DConfigDlgWdgItem*      page_subjects;
    DConfigDlgWdgItem*      page_keywords;
    DConfigDlgWdgItem*      page_categories;
    DConfigDlgWdgItem*      page_credits;
    DConfigDlgWdgItem*      page_status;
    DConfigDlgWdgItem*      page_properties;

    QList<QUrl>           urls;

    QList<QUrl>::iterator currItem;

    XMPContent*           contentPage;
    XMPKeywords*          keywordsPage;
    XMPCategories*        categoriesPage;
    XMPSubjects*          subjectsPage;
    XMPOrigin*            originPage;
    XMPCredits*           creditsPage;
    XMPStatus*            statusPage;
    XMPProperties*        propertiesPage;

    MetadataEditDialog*   dlg;
};

XMPEditWidget::XMPEditWidget(MetadataEditDialog* const parent)
    : DConfigDlgWdg(parent),
      d(new Private)
{
    d->dlg           = parent;

    d->contentPage   = new XMPContent(this);
    d->page_content  = addPage(d->contentPage, i18n("Content"));
    d->page_content->setIcon(QIcon::fromTheme(QLatin1String("draw-text")));

    d->originPage  = new XMPOrigin(this);
    d->page_origin = addPage(d->originPage, i18n("Origin"));
    d->page_origin->setIcon(QIcon::fromTheme(QLatin1String("configure")));

    d->creditsPage  = new XMPCredits(this);
    d->page_credits = addPage(d->creditsPage, i18n("Credits"));
    d->page_credits->setIcon(QIcon::fromTheme(QLatin1String("view-pim-contacts")));

    d->subjectsPage  = new XMPSubjects(this);
    d->page_subjects = addPage(d->subjectsPage, i18n("Subjects"));
    d->page_subjects->setIcon(QIcon::fromTheme(QLatin1String("feed-subscribe")));

    d->keywordsPage  = new XMPKeywords(this);
    d->page_keywords = addPage(d->keywordsPage, i18n("Keywords"));
    d->page_keywords->setIcon(QIcon::fromTheme(QLatin1String("bookmark-new")));

    d->categoriesPage  = new XMPCategories(this);
    d->page_categories = addPage(d->categoriesPage, i18n("Categories"));
    d->page_categories->setIcon(QIcon::fromTheme(QLatin1String("folder")));

    d->statusPage  = new XMPStatus(this);
    d->page_status = addPage(d->statusPage, i18n("Status"));
    d->page_status->setIcon(QIcon::fromTheme(QLatin1String("view-pim-tasks")));

    d->propertiesPage  = new XMPProperties(this);
    d->page_properties = addPage(d->propertiesPage, i18n("Properties"));
    d->page_properties->setIcon(QIcon::fromTheme(QLatin1String("draw-freehand")));

    // ------------------------------------------------------------

    connect(d->contentPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->propertiesPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->originPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->subjectsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->keywordsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->categoriesPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->creditsPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->statusPage, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    // ------------------------------------------------------------

    readSettings();
    slotItemChanged();
}

XMPEditWidget::~XMPEditWidget()
{
    delete d;
}

void XMPEditWidget::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));

    showPage(group.readEntry(QLatin1String("All XMP Edit Page"), 0));
    d->contentPage->setCheckedSyncJFIFComment(group.readEntry(QLatin1String("All Sync JFIF Comment"), true));
    d->contentPage->setCheckedSyncEXIFComment(group.readEntry(QLatin1String("All Sync EXIF Comment"), true));
    d->originPage->setCheckedSyncEXIFDate(group.readEntry(QLatin1String("All Sync EXIF Date"), true));
}

void XMPEditWidget::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("All Metadata Edit Settings"));

    group.writeEntry(QLatin1String("All XMP Edit Page"),     activePageIndex());
    group.writeEntry(QLatin1String("All Sync JFIF Comment"), d->contentPage->syncJFIFCommentIsChecked());
    group.writeEntry(QLatin1String("All Sync EXIF Comment"), d->contentPage->syncEXIFCommentIsChecked());
    group.writeEntry(QLatin1String("All Sync EXIF Date"),    d->originPage->syncEXIFDateIsChecked());
    config->sync();
}

void XMPEditWidget::slotItemChanged()
{
    d->page_content->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Content Information<br/>"
                     "<i>Use this panel to describe the visual content of the image</i></qt>")));
    d->page_origin->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Origin Information<br/>"
                    "<i>Use this panel for formal descriptive information about the image</i></qt>")));
    d->page_credits->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Credit Information<br/>"
                     "<i>Use this panel to record copyright information about the image</i></qt>")));
    d->page_subjects->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Subject Information<br/>"
                      "<i>Use this panel to record subject information about the image</i></qt>")));
    d->page_keywords->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Keyword Information<br/>"
                      "<i>Use this panel to record keywords relevant to the image</i></qt>")));
    d->page_categories->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Category Information<br/>"
                        "<i>Use this panel to record categories relevant to the image</i></qt>")));
    d->page_status->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Status Information<br/>"
                    "<i>Use this panel to record workflow information</i></qt>")));
    d->page_properties->setHeader(d->dlg->currentItemTitleHeader(i18n("<qt>Status Properties<br/>"
                      "<i>Use this panel to record workflow properties</i></qt>")));

    DMetadata meta;
    meta.load((*d->dlg->currentItem()).toLocalFile());

    d->exifData = meta.getExifEncoded();
    d->iptcData = meta.getIptc();
    d->xmpData  = meta.getXmp();

    d->contentPage->readMetadata(d->xmpData);
    d->originPage->readMetadata(d->xmpData);
    d->subjectsPage->readMetadata(d->xmpData);
    d->keywordsPage->readMetadata(d->xmpData);
    d->categoriesPage->readMetadata(d->xmpData);
    d->creditsPage->readMetadata(d->xmpData);
    d->statusPage->readMetadata(d->xmpData);
    d->propertiesPage->readMetadata(d->xmpData);

    d->isReadOnly = !DMetadata::canWriteXmp((*d->dlg->currentItem()).toLocalFile());
    emit signalSetReadOnly(d->isReadOnly);

    d->page_content->setEnabled(!d->isReadOnly);
    d->page_origin->setEnabled(!d->isReadOnly);
    d->page_subjects->setEnabled(!d->isReadOnly);
    d->page_keywords->setEnabled(!d->isReadOnly);
    d->page_categories->setEnabled(!d->isReadOnly);
    d->page_credits->setEnabled(!d->isReadOnly);
    d->page_status->setEnabled(!d->isReadOnly);
    d->page_properties->setEnabled(!d->isReadOnly);
}

void XMPEditWidget::apply()
{
    if (d->modified && !d->isReadOnly)
    {
        d->contentPage->applyMetadata(d->exifData, d->xmpData);
        d->originPage->applyMetadata(d->exifData, d->xmpData);
        d->subjectsPage->applyMetadata(d->xmpData);
        d->keywordsPage->applyMetadata(d->xmpData);
        d->categoriesPage->applyMetadata(d->xmpData);
        d->creditsPage->applyMetadata(d->xmpData);
        d->statusPage->applyMetadata(d->xmpData);
        d->propertiesPage->applyMetadata(d->xmpData);

        DMetadata meta;

        meta.load((*d->dlg->currentItem()).toLocalFile());
        meta.setExif(d->exifData);
        meta.setIptc(d->iptcData);
        meta.setXmp(d->xmpData);
        meta.save((*d->dlg->currentItem()).toLocalFile());
        d->modified = false;
    }
}

void XMPEditWidget::slotModified()
{
    if (!d->isReadOnly)
    {
        d->modified = true;
        emit signalModified();
    }
}

void XMPEditWidget::showPage(int page)
{
    switch (page)
    {
        case 0:
            setCurrentPage(d->page_content);
            break;
        case 1:
            setCurrentPage(d->page_origin);
            break;
        case 2:
            setCurrentPage(d->page_credits);
            break;
        case 3:
            setCurrentPage(d->page_subjects);
            break;
        case 4:
            setCurrentPage(d->page_keywords);
            break;
        case 5:
            setCurrentPage(d->page_categories);
            break;
        case 6:
            setCurrentPage(d->page_status);
            break;
        case 7:
            setCurrentPage(d->page_properties);
            break;
        default:
            setCurrentPage(d->page_content);
            break;
    }
}

int XMPEditWidget::activePageIndex() const
{
    DConfigDlgWdgItem* const cur = currentPage();

    if (cur == d->page_content)    return 0;
    if (cur == d->page_origin)     return 1;
    if (cur == d->page_credits)    return 2;
    if (cur == d->page_subjects)   return 3;
    if (cur == d->page_keywords)   return 4;
    if (cur == d->page_categories) return 5;
    if (cur == d->page_status)     return 6;
    if (cur == d->page_properties) return 7;

    return 0;
}

bool XMPEditWidget::isModified() const
{
    return d->modified;
}

}  // namespace Digikam
