/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : caption editor
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "captionedit.moc"

// KDE includes

#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>

// Local includes

#include <libkexiv2/version.h>
#include <libkexiv2/altlangstredit.h>
#include <libkexiv2/msgtextedit.h>

using namespace KExiv2Iface;

namespace Digikam
{

class CaptionEdit::Private
{
public:

    Private()
    {
        altLangStrEdit = 0;
        authorEdit     = 0;
    }

    KLineEdit*      authorEdit;

    AltLangStrEdit* altLangStrEdit;

    CaptionsMap     captionsValues;

    QString         lastDeletedLanguage;
    CaptionValues   lastDeletedValues;
};

CaptionEdit::CaptionEdit(QWidget* const parent)
    : KVBox(parent), d(new Private)
{

    d->altLangStrEdit = new AltLangStrEdit(this);
    d->altLangStrEdit->setTitle(i18n("Captions: "));
    d->altLangStrEdit->setClickMessage(i18n("Enter caption text here."));

    d->authorEdit = new KLineEdit(this);
    d->authorEdit->setClearButtonShown(true);
    d->authorEdit->setClickMessage(i18n("Enter caption author name here."));

    setMargin(0);
    setSpacing(0);

    connect(d->altLangStrEdit, SIGNAL(signalSelectionChanged(QString)),
            this, SLOT(slotSelectionChanged(QString)));

    connect(d->altLangStrEdit, SIGNAL(signalModified(QString,QString)),
            this, SLOT(slotCaptionModified(QString,QString)));

    connect(d->altLangStrEdit, SIGNAL(signalValueAdded(QString,QString)),
            this, SLOT(slotAddValue(QString,QString)));

    connect(d->altLangStrEdit, SIGNAL(signalValueDeleted(QString)),
            this, SLOT(slotDeleteValue(QString)));

    connect(d->authorEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotAuthorChanged(QString)));
}

CaptionEdit::~CaptionEdit()
{
    delete d;
}

void CaptionEdit::reset()
{
    d->altLangStrEdit->reset();

    d->authorEdit->blockSignals(true);
    d->authorEdit->clear();
    d->authorEdit->blockSignals(false);

    d->captionsValues.clear();
}

QString CaptionEdit::currentLanguageCode() const
{
    return d->altLangStrEdit->currentLanguageCode();
}

void CaptionEdit::setCurrentLanguageCode(const QString& lang)
{
#if KEXIV2_VERSION >= 0x020101
    if(d->altLangStrEdit->currentLanguageCode().isEmpty())
    {
        d->altLangStrEdit->setCurrentLanguageCode("x-default");
    }
    else
    {
        d->altLangStrEdit->setCurrentLanguageCode(lang);
    }
#else
    Q_UNUSED(lang);
#endif
}

void CaptionEdit::slotAddValue(const QString& lang, const QString& text)
{
    CaptionValues val;
    val.caption = text;
    val.author  = d->authorEdit->text();
    val.date    = QDateTime::currentDateTime();

    // The user may have removed the text and directly entered a new one. Do not drop author then.
    if (val.author.isEmpty() && d->lastDeletedLanguage == lang)
    {
        val.author = d->lastDeletedValues.author;
        d->authorEdit->blockSignals(true);
        d->authorEdit->setText(val.author);
        d->authorEdit->blockSignals(false);
    }

    d->lastDeletedLanguage.clear();

    d->captionsValues.insert(lang, val);
    emit signalModified();
}

void CaptionEdit::slotCaptionModified(const QString& lang, const QString& text)
{
    slotAddValue(lang, text);
}

void CaptionEdit::slotDeleteValue(const QString& lang)
{
    d->lastDeletedLanguage = lang;
    d->lastDeletedValues   = d->captionsValues.value(lang);

    d->captionsValues.remove(lang);
    d->authorEdit->blockSignals(true);
    d->authorEdit->clear();
    d->authorEdit->blockSignals(false);
    emit signalModified();
}

void CaptionEdit::slotSelectionChanged(const QString& lang)
{
    QString author = d->captionsValues.value(lang).author;
    d->authorEdit->blockSignals(true);
    d->authorEdit->setText(author);
    d->authorEdit->blockSignals(false);
}

void CaptionEdit::setValues(const CaptionsMap& values)
{
    d->lastDeletedLanguage.clear();

    d->captionsValues = values;
    d->altLangStrEdit->setValues(d->captionsValues.toAltLangMap());
    slotSelectionChanged(d->altLangStrEdit->currentLanguageCode());
}

CaptionsMap& CaptionEdit::values() const
{
    return d->captionsValues;
}

void CaptionEdit::slotAuthorChanged(const QString& text)
{
    CaptionValues captionValues = d->captionsValues.value(d->altLangStrEdit->currentLanguageCode());

    if (text != captionValues.author)
    {
        d->altLangStrEdit->addCurrent();
    }
}

MsgTextEdit* CaptionEdit::textEdit() const
{
#if KEXIV2_VERSION >= 0x020400
    return d->altLangStrEdit->textEdit();
#else
    return 0;
#endif
}

}  // namespace Digikam
