/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : IPTC content settings page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iptccontent.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QValidator>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QLineEdit>
#include <QTextEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "multistringsedit.h"
#include "dmetadata.h"
#include "dexpanderbox.h"

namespace Digikam
{

class IPTCContent::Private
{
public:

    Private()
    {
        headlineCheck        = 0;
        captionEdit          = 0;
        writerEdit           = 0;
        headlineEdit         = 0;
        captionCheck         = 0;
        syncJFIFCommentCheck = 0;
        syncEXIFCommentCheck = 0;
    }

    QCheckBox*        captionCheck;
    QCheckBox*        headlineCheck;
    QCheckBox*        syncJFIFCommentCheck;
    QCheckBox*        syncEXIFCommentCheck;

    QTextEdit*        captionEdit;

    QLineEdit*        headlineEdit;

    MultiStringsEdit* writerEdit;
};

IPTCContent::IPTCContent(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);

    // IPTC only accept printable Ascii char.
    QRegExp asciiRx(QLatin1String("[\x20-\x7F]+$"));
    QValidator* const asciiValidator = new QRegExpValidator(asciiRx, this);

    // --------------------------------------------------------

    d->headlineCheck = new QCheckBox(i18n("Headline:"), this);
    d->headlineEdit  = new QLineEdit(this);
    d->headlineEdit->setClearButtonEnabled(true);
    d->headlineEdit->setValidator(asciiValidator);
    d->headlineEdit->setMaxLength(256);
    d->headlineEdit->setWhatsThis(i18n("Enter here the content synopsis. This field is limited "
                                       "to 256 ASCII characters."));

    // --------------------------------------------------------

    d->captionCheck         = new QCheckBox(i18nc("content description", "Caption:"), this);
    d->captionEdit          = new QTextEdit(this);
    d->syncJFIFCommentCheck = new QCheckBox(i18n("Sync JFIF Comment section"), this);
    d->syncEXIFCommentCheck = new QCheckBox(i18n("Sync EXIF Comment"), this);

/*
    d->captionEdit->setValidator(asciiValidator);
    d->captionEdit->setMaxLength(2000);
*/
    d->captionEdit->setWhatsThis(i18n("Enter the content description. This field is limited "
                                      "to 2000 ASCII characters."));

    // --------------------------------------------------------

    d->writerEdit  = new MultiStringsEdit(this, i18n("Caption Writer:"),
                                          i18n("Enter the name of the caption author."),
                                          true, 32);

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note: "
                 "<b><a href='http://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a></b> "
                 "text tags only support the printable "
                 "<b><a href='http://en.wikipedia.org/wiki/Ascii'>ASCII</a></b> "
                 "characters and limit string sizes. "
                 "Use contextual help for details.</b>"), this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    grid->addWidget(d->headlineCheck,                       0, 0, 1, 1);
    grid->addWidget(d->headlineEdit,                        0, 1, 1, 2);
    grid->addWidget(d->captionCheck,                        1, 0, 1, 3);
    grid->addWidget(d->captionEdit,                         2, 0, 1, 3);
    grid->addWidget(d->syncJFIFCommentCheck,                3, 0, 1, 3);
    grid->addWidget(d->syncEXIFCommentCheck,                5, 0, 1, 3);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  6, 0, 1, 3);
    grid->addWidget(d->writerEdit,                          7, 0, 1, 3);
    grid->addWidget(note,                                   8, 0, 1, 3);
    grid->setRowStretch(9, 10);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------------------------------------------

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            d->captionEdit, SLOT(setEnabled(bool)));

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            d->syncJFIFCommentCheck, SLOT(setEnabled(bool)));

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            d->syncEXIFCommentCheck, SLOT(setEnabled(bool)));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            d->headlineEdit, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->captionCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->writerEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    connect(d->headlineCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->captionEdit, SIGNAL(textChanged()),
            this, SIGNAL(signalModified()));

    connect(d->headlineEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

IPTCContent::~IPTCContent()
{
    delete d;
}

bool IPTCContent::syncJFIFCommentIsChecked() const
{
    return d->syncJFIFCommentCheck->isChecked();
}

bool IPTCContent::syncEXIFCommentIsChecked() const
{
    return d->syncEXIFCommentCheck->isChecked();
}

QString IPTCContent::getIPTCCaption() const
{
    return d->captionEdit->toPlainText();
}

void IPTCContent::setCheckedSyncJFIFComment(bool c)
{
    d->syncJFIFCommentCheck->setChecked(c);
}

void IPTCContent::setCheckedSyncEXIFComment(bool c)
{
    d->syncEXIFCommentCheck->setChecked(c);
}

void IPTCContent::readMetadata(QByteArray& iptcData)
{
    blockSignals(true);
    DMetadata meta;
    meta.setIptc(iptcData);
    QString     data;
    QStringList list;

    d->captionEdit->clear();
    d->captionCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Caption", false);
    if (!data.isNull())
    {
        d->captionEdit->setText(data);
        d->captionCheck->setChecked(true);
    }
    d->captionEdit->setEnabled(d->captionCheck->isChecked());
    d->syncJFIFCommentCheck->setEnabled(d->captionCheck->isChecked());
    d->syncEXIFCommentCheck->setEnabled(d->captionCheck->isChecked());

    list = meta.getIptcTagsStringList("Iptc.Application2.Writer", false);
    d->writerEdit->setValues(list);

    d->headlineEdit->clear();
    d->headlineCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.Headline", false);
    if (!data.isNull())
    {
        d->headlineEdit->setText(data);
        d->headlineCheck->setChecked(true);
    }
    d->headlineEdit->setEnabled(d->headlineCheck->isChecked());

    blockSignals(false);
}

void IPTCContent::applyMetadata(QByteArray& exifData, QByteArray& iptcData)
{
    DMetadata meta;
    meta.setExif(exifData);
    meta.setIptc(iptcData);

    if (d->captionCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.Caption", d->captionEdit->toPlainText());

        if (syncEXIFCommentIsChecked())
            meta.setExifComment(getIPTCCaption());

        if (syncJFIFCommentIsChecked())
            meta.setComments(getIPTCCaption().toUtf8());
    }
    else
        meta.removeIptcTag("Iptc.Application2.Caption");

    QStringList oldList, newList;

    if (d->writerEdit->getValues(oldList, newList))
        meta.setIptcTagsStringList("Iptc.Application2.Writer", 32, oldList, newList);
    else
        meta.removeIptcTag("Iptc.Application2.Writer");

    if (d->headlineCheck->isChecked())
        meta.setIptcTagString("Iptc.Application2.Headline", d->headlineEdit->text());
    else
        meta.removeIptcTag("Iptc.Application2.Headline");

    exifData = meta.getExifEncoded();

    iptcData = meta.getIptc();
}

}  // namespace Digikam
