/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-18
 * Description : XMP content settings page.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "xmpcontent.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QTextEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "altlangstringedit.h"
#include "dmetadata.h"
#include "dexpanderbox.h"

namespace Digikam
{

class XMPContent::Private
{
public:

    Private()
    {
        writerCheck          = 0;
        headlineCheck        = 0;
        captionEdit          = 0;
        writerEdit           = 0;
        headlineEdit         = 0;
        syncJFIFCommentCheck = 0;
        syncEXIFCommentCheck = 0;
        copyrightEdit        = 0;
    }

    QCheckBox*          headlineCheck;
    QCheckBox*          syncJFIFCommentCheck;
    QCheckBox*          syncEXIFCommentCheck;
    QCheckBox*          writerCheck;

    QLineEdit*          headlineEdit;
    QLineEdit*          writerEdit;

    AltLangStringsEdit* captionEdit;
    AltLangStringsEdit* copyrightEdit;
};

XMPContent::XMPContent(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(this);

    // --------------------------------------------------------

    d->headlineCheck = new QCheckBox(i18n("Headline:"), this);
    d->headlineEdit  = new QLineEdit(this);
    d->headlineEdit->setClearButtonEnabled(true);
    d->headlineEdit->setWhatsThis(i18n("Enter here the content synopsis."));

    // --------------------------------------------------------

    d->captionEdit          = new AltLangStringsEdit(this, i18nc("content description", "Caption:"),
                                                     i18n("Enter the content description."));

    QGroupBox* const syncOptions  = new QGroupBox(i18n("Default Language Caption Options"), this);
    QVBoxLayout* const vlay       = new QVBoxLayout(syncOptions);

    d->syncJFIFCommentCheck = new QCheckBox(i18n("Sync JFIF Comment section"), syncOptions);
    d->syncEXIFCommentCheck = new QCheckBox(i18n("Sync EXIF Comment"), syncOptions);

    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);
    vlay->addWidget(d->syncJFIFCommentCheck);
    vlay->addWidget(d->syncEXIFCommentCheck);

    // --------------------------------------------------------

    d->writerCheck = new QCheckBox(i18n("Caption Writer:"), this);
    d->writerEdit  = new QLineEdit(this);
    d->writerEdit->setClearButtonEnabled(true);
    d->writerEdit->setWhatsThis(i18n("Enter the name of the caption author."));

    d->copyrightEdit = new AltLangStringsEdit(this, i18n("Copyright:"),
                                              i18n("Enter the necessary copyright notice."));

    // --------------------------------------------------------

    grid->addWidget(d->headlineCheck,                      0, 0, 1, 1);
    grid->addWidget(d->headlineEdit,                       0, 1, 1, 2);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this), 1, 0, 1, 3);
    grid->addWidget(d->captionEdit,                        2, 0, 1, 3);
    grid->addWidget(syncOptions,                           3, 0, 1, 3);
    grid->addWidget(d->writerCheck,                        4, 0, 1, 1);
    grid->addWidget(d->writerEdit,                         4, 1, 1, 2);
    grid->addWidget(d->copyrightEdit,                      5, 0, 1, 3);
    grid->setRowStretch(6, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(signalDefaultLanguageEnabled(bool)),
            this, SLOT(slotSyncOptionsEnabled(bool)));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            d->headlineEdit, SLOT(setEnabled(bool)));

    connect(d->writerCheck, SIGNAL(toggled(bool)),
            d->writerEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(signalToggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->copyrightEdit, SIGNAL(signalToggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->writerCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->copyrightEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->headlineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->writerEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

XMPContent::~XMPContent()
{
    delete d;
}

bool XMPContent::syncJFIFCommentIsChecked() const
{
    return d->syncJFIFCommentCheck->isChecked();
}

bool XMPContent::syncEXIFCommentIsChecked() const
{
    return d->syncEXIFCommentCheck->isChecked();
}

QString XMPContent::getXMPCaption() const
{
    return d->captionEdit->defaultAltLang();
}

void XMPContent::setCheckedSyncJFIFComment(bool c)
{
    d->syncJFIFCommentCheck->setChecked(c);
}

void XMPContent::setCheckedSyncEXIFComment(bool c)
{
    d->syncEXIFCommentCheck->setChecked(c);
}

void XMPContent::readMetadata(QByteArray& xmpData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setXmp(xmpData);

    DMetadata::AltLangMap map;
    QString data;

    d->headlineEdit->clear();
    d->headlineCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.Headline", false);

    if (!data.isNull())
    {
        d->headlineEdit->setText(data);
        d->headlineCheck->setChecked(true);
    }

    d->headlineEdit->setEnabled(d->headlineCheck->isChecked());

    d->captionEdit->setValues(map);
    d->captionEdit->setValid(false);
    map = meta.getXmpTagStringListLangAlt("Xmp.dc.description", false);

    if (!map.isEmpty())
    {
        d->captionEdit->setValues(map);
        d->captionEdit->setValid(true);
    }

    d->writerEdit->clear();
    d->writerCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.CaptionWriter", false);

    if (!data.isNull())
    {
        d->writerEdit->setText(data);
        d->writerCheck->setChecked(true);
    }

    d->writerEdit->setEnabled(d->writerCheck->isChecked());

    map.clear();
    d->copyrightEdit->setValues(map);
    d->copyrightEdit->setValid(false);
    map = meta.getXmpTagStringListLangAlt("Xmp.dc.rights", false);

    if (!map.isEmpty())
    {
        d->copyrightEdit->setValues(map);
        d->copyrightEdit->setValid(true);
    }

    blockSignals(false);
}

void XMPContent::applyMetadata(QByteArray& exifData, QByteArray& xmpData)
{
    DMetadata meta;
    meta.setExif(exifData);
    meta.setXmp(xmpData);

    if (d->headlineCheck->isChecked())
        meta.setXmpTagString("Xmp.photoshop.Headline", d->headlineEdit->text());
    else
        meta.removeXmpTag("Xmp.photoshop.Headline");

    DMetadata::AltLangMap oldAltLangMap, newAltLangMap;

    if (d->captionEdit->getValues(oldAltLangMap, newAltLangMap))
    {
        meta.setXmpTagStringListLangAlt("Xmp.dc.description", newAltLangMap);

        if (syncEXIFCommentIsChecked())
            meta.setExifComment(getXMPCaption());

        if (syncJFIFCommentIsChecked())
            meta.setComments(getXMPCaption().toUtf8());
    }
    else if (d->captionEdit->isValid())
    {
        meta.removeXmpTag("Xmp.dc.description");
    }

    if (d->writerCheck->isChecked())
        meta.setXmpTagString("Xmp.photoshop.CaptionWriter", d->writerEdit->text());
    else
        meta.removeXmpTag("Xmp.photoshop.CaptionWriter");

    if (d->copyrightEdit->getValues(oldAltLangMap, newAltLangMap))
        meta.setXmpTagStringListLangAlt("Xmp.dc.rights", newAltLangMap);
    else if (d->copyrightEdit->isValid())
        meta.removeXmpTag("Xmp.dc.rights");

    exifData = meta.getExifEncoded();
    xmpData  = meta.getXmp();
}

void XMPContent::slotSyncOptionsEnabled(bool defaultLangAlt)
{
    bool cond = defaultLangAlt & d->captionEdit->isValid();
    d->syncJFIFCommentCheck->setEnabled(cond);
    d->syncEXIFCommentCheck->setEnabled(cond);
}

}  // namespace Digikam
