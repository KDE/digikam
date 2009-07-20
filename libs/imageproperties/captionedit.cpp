/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : caption editor
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "captionedit.h"
#include "captionedit.moc"

// KDE includes

#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>

// Local includes

#include "altlangstredit.h"

namespace Digikam
{

class CaptionEditPriv
{
public:

    CaptionEditPriv()
    {
        altLangStrEdit = 0;
        authorEdit     = 0;
    }

    KLineEdit      *authorEdit;

    AltLangStrEdit *altLangStrEdit;

    CaptionsMap     captionsValues;
};

CaptionEdit::CaptionEdit(QWidget* parent)
           : KVBox(parent), d(new CaptionEditPriv)
{

    d->altLangStrEdit = new AltLangStrEdit(this);
    d->altLangStrEdit->setTitle(i18n("Captions: "));
    d->altLangStrEdit->setFixedHeight(100);
    d->altLangStrEdit->setClickMessage(i18n("Enter caption text here."));

    d->authorEdit = new KLineEdit(this);
    d->authorEdit->setClearButtonShown(true);
    d->authorEdit->setClickMessage(i18n("Enter caption author name here."));

    setMargin(0);
    setSpacing(0);

    connect(d->altLangStrEdit, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->altLangStrEdit, SIGNAL(signalSelectionChanged(const QString&)),
            this, SLOT(slotSelectionChanged(const QString&)));

    connect(d->altLangStrEdit, SIGNAL(signalAddValue(const QString&, const QString&)),
            this, SLOT(slotAddValue(const QString&, const QString&)));

    connect(d->altLangStrEdit, SIGNAL(signalDeleteValue(const QString&)),
            this, SLOT(slotDeleteValue(const QString&)));

    connect(d->authorEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotAuthorChanged(const QString&)));

}

CaptionEdit::~CaptionEdit()
{
    delete d;
}

void CaptionEdit::reset()
{
    d->altLangStrEdit->reset();
    d->authorEdit->clear();
    d->captionsValues.clear();
}

void CaptionEdit::slotModified()
{
    emit signalModified();
}

void CaptionEdit::slotAddValue(const QString& lang, const QString& text)
{
    CaptionValues val;
    val.caption = text;
    val.author  = d->authorEdit->text();
    val.date    = QDateTime::currentDateTime();
    d->captionsValues.insert(lang, val);
}

void CaptionEdit::slotDeleteValue(const QString& lang)
{
    d->captionsValues.remove(lang);
}

void CaptionEdit::slotSelectionChanged(const QString& lang)
{
    QString author = d->captionsValues[lang].author;
    d->authorEdit->setText(author);
}

void CaptionEdit::setValues(const CaptionsMap& values)
{
    d->captionsValues = values;
    d->altLangStrEdit->setValues(d->captionsValues.toAltLangMap());
    slotSelectionChanged(d->altLangStrEdit->currentLanguageCode());
}

CaptionsMap& CaptionEdit::values()
{
    return d->captionsValues;
}

void CaptionEdit::apply()
{
    d->altLangStrEdit->apply();
}

void CaptionEdit::slotAuthorChanged(const QString& text)
{
    bool dirty = (text != d->captionsValues[d->altLangStrEdit->currentLanguageCode()].author);
    d->altLangStrEdit->setDirty(dirty);

    if (dirty)
        emit signalModified();
}

}  // namespace Digikam
