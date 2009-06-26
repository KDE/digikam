/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages string editor
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

#include "altlangstredit.h"
#include "altlangstredit.moc"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include <QToolButton>
#include <QGridLayout>

// KDE includes

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ktextedit.h>
#include <kcombobox.h>
#include <kdebug.h>

using namespace KExiv2Iface;

namespace Digikam
{

class AltLangStrEditPriv
{
public:

    AltLangStrEditPriv()
    {
        addValueButton = 0;
        delValueButton = 0;
        languageCB     = 0;

        // We cannot use KLocale::allLanguagesList() here because KDE only
        // support 2 characters country codes. XMP require 2+2 characters language+country
        // following ISO 3066 (http://babelwiki.babelzilla.org/index.php?title=Language_codes)

        // The first one from the list is the Default Language code specified by XMP paper
        languageCodeMap.insert( "x-default", i18n("Default Language") );

        // Standard ISO 3066 country codes.

        languageCodeMap.insert( "af-ZA", i18n("Afrikaans (South Africa)") );
        languageCodeMap.insert( "am-ET", i18n("Amharic (Ethiopia)") );
        languageCodeMap.insert( "ar-AE", i18n("Arabic (UAE)") );
        languageCodeMap.insert( "ar-BH", i18n("Arabic (Bahrain)") );
        languageCodeMap.insert( "ar-DZ", i18n("Arabic (Algeria)") );
        languageCodeMap.insert( "ar-EG", i18n("Arabic (Egypt)") );
        languageCodeMap.insert( "ar-IQ", i18n("Arabic (Iraq)") );
        languageCodeMap.insert( "ar-JO", i18n("Arabic (Jordan)") );
        languageCodeMap.insert( "ar-KW", i18n("Arabic (Kuwait)") );
        languageCodeMap.insert( "ar-LB", i18n("Arabic (Lebanon)") );
        languageCodeMap.insert( "ar-LY", i18n("Arabic (Libya)") );
        languageCodeMap.insert( "ar-MA", i18n("Arabic (Morocco)") );
        languageCodeMap.insert( "ar-OM", i18n("Arabic (Oman)") );
        languageCodeMap.insert( "ar-QA", i18n("Arabic (Qatar)") );
        languageCodeMap.insert( "ar-SA", i18n("Arabic (Saudi Arabia)") );
        languageCodeMap.insert( "ar-SY", i18n("Arabic (Syria)") );
        languageCodeMap.insert( "ar-TN", i18n("Arabic (Tunisia)") );
        languageCodeMap.insert( "ar-YE", i18n("Arabic (Yemen)") );
        languageCodeMap.insert( "as-IN", i18n("Assamese (India)") );
        languageCodeMap.insert( "ba-RU", i18n("Bashkir (Russia)") );
        languageCodeMap.insert( "be-BY", i18n("Belarusian (Belarus)") );
        languageCodeMap.insert( "bg-BG", i18n("Bulgarian (Bulgaria)") );
        languageCodeMap.insert( "bn-IN", i18n("Bengali (India)") );
        languageCodeMap.insert( "bo-BT", i18n("Tibetan (Bhutan)") );
        languageCodeMap.insert( "bo-CN", i18n("Tibetan (PRC)") );
        languageCodeMap.insert( "br-FR", i18n("Breton (France)") );
        languageCodeMap.insert( "ca-AD", i18n("Catalan (Andorra)") );
        languageCodeMap.insert( "ca-ES", i18n("Catalan (Spain)") );
        languageCodeMap.insert( "ca-FR", i18n("Catalan (France)") );
        languageCodeMap.insert( "co-FR", i18n("Corsican (France)") );
        languageCodeMap.insert( "cs-CZ", i18n("Czech (Czech Republic)") );
        languageCodeMap.insert( "cy-GB", i18n("Welsh (United Kingdom)") );
        languageCodeMap.insert( "da-DK", i18n("Danish (Denmark)") );
        languageCodeMap.insert( "de-AT", i18n("German (Austria)") );
        languageCodeMap.insert( "de-CH", i18n("German (Switzerland)") );
        languageCodeMap.insert( "de-DE", i18n("German (Germany)") );
        languageCodeMap.insert( "de-LI", i18n("German (Liechtenstein)") );
        languageCodeMap.insert( "de-LU", i18n("German (Luxembourg)") );
        languageCodeMap.insert( "el-GR", i18n("Greek (Greece)") );
        languageCodeMap.insert( "en-AU", i18n("English (Australia)") );
        languageCodeMap.insert( "en-BZ", i18n("English (Belize)") );
        languageCodeMap.insert( "en-CA", i18n("English (Canada)") );
        languageCodeMap.insert( "en-CB", i18n("English (Caribbean)") );
        languageCodeMap.insert( "en-GB", i18n("English (United Kingdom)") );
        languageCodeMap.insert( "en-IE", i18n("English (Ireland)") );
        languageCodeMap.insert( "en-IN", i18n("English (India)") );
        languageCodeMap.insert( "en-JA", i18n("English (Jamaica)") );
        languageCodeMap.insert( "en-MY", i18n("English (Malaysia)") );
        languageCodeMap.insert( "en-NZ", i18n("English (New Zealand)") );
        languageCodeMap.insert( "en-PH", i18n("English (Philippines)") );
        languageCodeMap.insert( "en-SG", i18n("English (Singapore)") );
        languageCodeMap.insert( "en-TT", i18n("English (Trinidad)") );
        languageCodeMap.insert( "en-US", i18n("English (United States)") );
        languageCodeMap.insert( "en-ZA", i18n("English (South Africa)") );
        languageCodeMap.insert( "en-ZW", i18n("English (Zimbabwe)") );
        languageCodeMap.insert( "es-AR", i18n("Spanish (Argentina)") );
        languageCodeMap.insert( "es-BO", i18n("Spanish (Bolivia)") );
        languageCodeMap.insert( "es-CL", i18n("Spanish (Chile)") );
        languageCodeMap.insert( "es-CO", i18n("Spanish (Colombia)") );
        languageCodeMap.insert( "es-CR", i18n("Spanish (Costa Rica)") );
        languageCodeMap.insert( "es-DO", i18n("Spanish (Dominican Republic)") );
        languageCodeMap.insert( "es-EC", i18n("Spanish (Ecuador)") );
        languageCodeMap.insert( "es-ES", i18n("Spanish (Spain)") );
        languageCodeMap.insert( "es-GT", i18n("Spanish (Guatemala)") );
        languageCodeMap.insert( "es-HN", i18n("Spanish (Honduras)") );
        languageCodeMap.insert( "es-MX", i18n("Spanish (Mexico)") );
        languageCodeMap.insert( "es-NI", i18n("Spanish (Nicaragua)") );
        languageCodeMap.insert( "es-PA", i18n("Spanish (Panama)") );
        languageCodeMap.insert( "es-PE", i18n("Spanish (Peru)") );
        languageCodeMap.insert( "es-PR", i18n("Spanish (Puerto Rico)") );
        languageCodeMap.insert( "es-PY", i18n("Spanish (Paraguay)") );
        languageCodeMap.insert( "es-SV", i18n("Spanish (El Salvador)") );
        languageCodeMap.insert( "es-UR", i18n("Spanish (Uruguay)") );
        languageCodeMap.insert( "es-US", i18n("Spanish (United States)") );
        languageCodeMap.insert( "es-VE", i18n("Spanish (Venezuela)") );
        languageCodeMap.insert( "et-EE", i18n("Estonian (Estonia)") );
        languageCodeMap.insert( "eu-ES", i18n("Basque (Basque Country)") );
        languageCodeMap.insert( "fa-IR", i18n("Persian (Iran)") );
        languageCodeMap.insert( "fi-FI", i18n("Finnish (Finland)") );
        languageCodeMap.insert( "fo-FO", i18n("Faeroese (Faero Islands)") );
        languageCodeMap.insert( "fr-BE", i18n("French (Belgium)") );
        languageCodeMap.insert( "fr-CA", i18n("French (Canada)") );
        languageCodeMap.insert( "fr-CH", i18n("French (Switzerland)") );
        languageCodeMap.insert( "fr-FR", i18n("French (France)") );
        languageCodeMap.insert( "fr-LU", i18n("French (Luxembourg)") );
        languageCodeMap.insert( "fr-MC", i18n("French (Monaco)") );
        languageCodeMap.insert( "fy-NL", i18n("Frisian (Netherlands)") );
        languageCodeMap.insert( "ga-IE", i18n("Irish (Ireland)") );
        languageCodeMap.insert( "gl-ES", i18n("Galician (Galicia)") );
        languageCodeMap.insert( "gu-IN", i18n("Gujarati (India)") );
        languageCodeMap.insert( "he-IL", i18n("Hebrew (Israel)") );
        languageCodeMap.insert( "hi-IN", i18n("Hindi (India)") );
        languageCodeMap.insert( "hr-BA", i18n("Croatian (Bosnia and Herzegovina, Latin)") );
        languageCodeMap.insert( "hr-HR", i18n("Croatian (Croatia)") );
        languageCodeMap.insert( "hu-HU", i18n("Hungarian (Hungary)") );
        languageCodeMap.insert( "hy-AM", i18n("Armenian (Armenia)") );
        languageCodeMap.insert( "id-ID", i18n("(Indonesian)") );
        languageCodeMap.insert( "ii-CN", i18n("Yi (PRC)") );
        languageCodeMap.insert( "is-IS", i18n("Icelandic (Iceland)") );
        languageCodeMap.insert( "it-CH", i18n("Italian (Switzerland)") );
        languageCodeMap.insert( "it-IT", i18n("Italian (Italy)") );
        languageCodeMap.insert( "ja-JP", i18n("Japanese (Japan)") );
        languageCodeMap.insert( "ka-GE", i18n("Georgian (Georgia)") );
        languageCodeMap.insert( "kk-KZ", i18n("Kazakh (Kazakhstan)") );
        languageCodeMap.insert( "kl-GL", i18n("Greenlandic (Greenland)") );
        languageCodeMap.insert( "km-KH", i18n("Khmer (Cambodia)") );
        languageCodeMap.insert( "kn-IN", i18n("Kannada (India)") );
        languageCodeMap.insert( "ko-KR", i18n("Korean (Korea)") );
        languageCodeMap.insert( "ky-KG", i18n("Kyrgyz (Kyrgyzstan)") );
        languageCodeMap.insert( "lb-LU", i18n("Luxembourgish (Luxembourg)") );
        languageCodeMap.insert( "lo-LA", i18n("Lao (Lao PDR)") );
        languageCodeMap.insert( "lt-LT", i18n("Lithuanian (Lithuania)") );
        languageCodeMap.insert( "lv-LV", i18n("Latvian (Latvia)") );
        languageCodeMap.insert( "mi-NZ", i18n("Maori (New Zealand)") );
        languageCodeMap.insert( "mk-MK", i18n("Macedonian (Macedonia)") );
        languageCodeMap.insert( "ml-IN", i18n("Malayalam (India)") );
        languageCodeMap.insert( "mn-CN", i18n("Mongolian (PRC)") );
        languageCodeMap.insert( "mn-MN", i18n("Mongolian (Mongolia)") );
        languageCodeMap.insert( "mr-IN", i18n("Marathi (India)") );
        languageCodeMap.insert( "ms-BN", i18n("Malay (Brunei Darussalam)") );
        languageCodeMap.insert( "ms-MY", i18n("Malay (Malaysia)") );
        languageCodeMap.insert( "mt-MT", i18n("Maltese (Malta)") );
        languageCodeMap.insert( "nb-NO", i18n("Norwegian Bokmål (Norway)") );
        languageCodeMap.insert( "ne-NP", i18n("Nepali (Nepal)") );
        languageCodeMap.insert( "nl-BE", i18n("Dutch (Belgium)") );
        languageCodeMap.insert( "nl-NL", i18n("Dutch (Netherlands)") );
        languageCodeMap.insert( "nn-NO", i18n("Norwegian Nynorsk (Norway)") );
        languageCodeMap.insert( "ns-ZA", i18n("Sesotho sa Leboa (South Africa)") );
        languageCodeMap.insert( "oc-FR", i18n("Occitan (France)") );
        languageCodeMap.insert( "or-IN", i18n("Oriya (India)") );
        languageCodeMap.insert( "pa-IN", i18n("Punjabi (India)") );
        languageCodeMap.insert( "pl-PL", i18n("Polish (Poland)") );
        languageCodeMap.insert( "ps-AF", i18n("Pashto (Afghanistan)") );
        languageCodeMap.insert( "pt-BR", i18n("Portuguese (Brazil)") );
        languageCodeMap.insert( "pt-PT", i18n("Portuguese (Portugal)") );
        languageCodeMap.insert( "rm-CH", i18n("Romansh (Switzerland)") );
        languageCodeMap.insert( "ro-RO", i18n("Romanian (Romania)") );
        languageCodeMap.insert( "ru-RU", i18n("Russian (Russia)") );
        languageCodeMap.insert( "rw-RW", i18n("Kinyarwanda (Rwanda)") );
        languageCodeMap.insert( "sa-IN", i18n("Sanskrit (India)") );
        languageCodeMap.insert( "se-FI", i18n("Sami (Northern, Finland)") );
        languageCodeMap.insert( "se-NO", i18n("Sami (Northern, Norway)") );
        languageCodeMap.insert( "se-SE", i18n("Sami (Northern, Sweden)") );
        languageCodeMap.insert( "si-LK", i18n("Sinhala (Sri Lanka)") );
        languageCodeMap.insert( "sk-SK", i18n("Slovak (Slovakia)") );
        languageCodeMap.insert( "sl-SI", i18n("Slovenian (Slovenia)") );
        languageCodeMap.insert( "sq-AL", i18n("Albanian (Albania)") );
        languageCodeMap.insert( "sv-FI", i18n("Swedish (Finland)") );
        languageCodeMap.insert( "sv-SE", i18n("Swedish (Sweden)") );
        languageCodeMap.insert( "sw-KE", i18n("Swahili (Kenya)") );
        languageCodeMap.insert( "ta-IN", i18n("Tamil (India)") );
        languageCodeMap.insert( "te-IN", i18n("Telugu (India)") );
        languageCodeMap.insert( "th-TH", i18n("Thai (Thailand)") );
        languageCodeMap.insert( "tk-TM", i18n("Turkmen (Turkmenistan)") );
        languageCodeMap.insert( "tn-ZA", i18n("Setswana Tswana (South Africa)") );
        languageCodeMap.insert( "tr-IN", i18n("Urdu (India)") );
        languageCodeMap.insert( "tr-TR", i18n("Turkish (Turkey)") );
        languageCodeMap.insert( "tt-RU", i18n("Tatar (Russia)") );
        languageCodeMap.insert( "ug-CN", i18n("Uighur (PRC)") );
        languageCodeMap.insert( "uk-UA", i18n("Ukrainian (Ukraine)") );
        languageCodeMap.insert( "ur-PK", i18n("Urdu (Pakistan)") );
        languageCodeMap.insert( "vi-VN", i18n("Vietnamese (Vietnam)") );
        languageCodeMap.insert( "wo-SN", i18n("Wolof (Senegal)") );
        languageCodeMap.insert( "xh-ZA", i18n("isiXhosa Xhosa (South Africa)") );
        languageCodeMap.insert( "yo-NG", i18n("Yoruba (Nigeria)") );
        languageCodeMap.insert( "zh-CN", i18n("Chinese (PRC)") );
        languageCodeMap.insert( "zh-HK", i18n("Chinese (Hong Kong SAR, PRC)") );
        languageCodeMap.insert( "zh-MO", i18n("Chinese (Macao SAR)") );
        languageCodeMap.insert( "zh-SG", i18n("Chinese (Singapore)") );
        languageCodeMap.insert( "zh-TW", i18n("Chinese (Taiwan)") );
        languageCodeMap.insert( "zu-ZA", i18n("isiZulu Zulu (South Africa)") );
    }

    typedef QMap<QString, QString>  LanguageCodeMap;

    LanguageCodeMap                 languageCodeMap;

    KExiv2::AltLangMap              values;

    QToolButton                    *addValueButton;
    QToolButton                    *delValueButton;

    KTextEdit                      *valueEdit;

    KComboBox                      *languageCB;
};

AltLangStrEdit::AltLangStrEdit(QWidget* parent, const QString& title)
              : QWidget(parent), d(new AltLangStrEditPriv)
{
    QGridLayout *grid = new QGridLayout(this);

    // --------------------------------------------------------

    QLabel *titleLabel = new QLabel(title, this);

    d->addValueButton  = new QToolButton(this);
    d->delValueButton  = new QToolButton(this);
    d->addValueButton->setIcon(SmallIcon("list-add"));
    d->delValueButton->setIcon(SmallIcon("list-remove"));
    d->delValueButton->setToolTip(i18n("Remove current caption"));
    d->addValueButton->setEnabled(false);
    d->delValueButton->setEnabled(false);

    d->languageCB = new KComboBox(this);
    d->languageCB->setWhatsThis(i18n("Select here the language for your caption."));

    d->valueEdit  = new KTextEdit(this);
    d->valueEdit->setCheckSpellingEnabled(true);

    // --------------------------------------------------------

    grid->setAlignment( Qt::AlignTop );
    grid->addWidget(titleLabel,        0, 0, 1, 1);
    grid->addWidget(d->languageCB,     0, 1, 1, 2);
    grid->addWidget(d->addValueButton, 0, 3, 1, 1);
    grid->addWidget(d->delValueButton, 0, 4, 1, 1);
    grid->addWidget(d->valueEdit,      1, 0, 1, 5);
    grid->setColumnStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->languageCB, SIGNAL(activated(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->addValueButton, SIGNAL(clicked()),
            this, SLOT(slotAddValue()));

    connect(d->addValueButton, SIGNAL(clicked()),
            this, SIGNAL(signalModified()));

    connect(d->delValueButton, SIGNAL(clicked()),
            this, SLOT(slotDeleteValue()));

    connect(d->valueEdit, SIGNAL(textChanged()),
            this, SLOT(slotTextChanged()));
}

AltLangStrEdit::~AltLangStrEdit()
{
    delete d;
}

void AltLangStrEdit::reset()
{
    setValues(KExiv2::AltLangMap());
}

void AltLangStrEdit::slotAddValue()
{
    QString lang = d->languageCB->currentText();
    QString text = d->valueEdit->toPlainText();
    if (text.isEmpty()) return;

    d->values.insert(lang, text);
    d->valueEdit->blockSignals(true);
    d->valueEdit->clear();
    d->valueEdit->blockSignals(false);
    loadLangAltListEntries(lang);
}

void AltLangStrEdit::slotDeleteValue()
{
    QString lang = d->languageCB->currentText();
    d->values.remove(lang);
    loadLangAltListEntries();
    emit signalModified();
}

void AltLangStrEdit::slotSelectionChanged(int index)
{
    QString lang = d->languageCB->currentText();
    d->valueEdit->blockSignals(true);

    if (!d->languageCB->itemIcon(index).isNull())
    {
        QString text = d->values[lang];
        d->valueEdit->setText(text);
        d->addValueButton->setEnabled(false);
        d->delValueButton->setEnabled(true);
        d->addValueButton->setIcon(SmallIcon("view-refresh"));
        d->addValueButton->setToolTip(i18n("Update current caption"));
    }
    else
    {
        d->valueEdit->clear();
        d->addValueButton->setEnabled(false);
        d->delValueButton->setEnabled(false);
        d->addValueButton->setIcon(SmallIcon("list-add"));
        d->addValueButton->setToolTip(i18n("Add new caption"));
    }
    d->valueEdit->blockSignals(false);
    d->languageCB->setToolTip(d->languageCodeMap[lang]);
}

void AltLangStrEdit::setValues(const KExiv2::AltLangMap& values)
{
    blockSignals(true);
    d->values = values;
    loadLangAltListEntries();
    blockSignals(false);
}

KExiv2::AltLangMap& AltLangStrEdit::values()
{
    return d->values;
}

void AltLangStrEdit::loadLangAltListEntries(const QString& currentLang)
{
    d->languageCB->clear();

    // In first we fill already assigned languages.

    QStringList list = d->values.keys();
    if (!list.isEmpty())
    {
        for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
        {
              d->languageCB->addItem(*it);
              d->languageCB->setItemIcon(d->languageCB->count()-1, SmallIcon("dialog-ok"));
        }
        d->languageCB->insertSeparator(d->languageCB->count());
    }

    // ...and now, all the rest...

    for (AltLangStrEditPriv::LanguageCodeMap::Iterator it = d->languageCodeMap.begin();
         it != d->languageCodeMap.end(); ++it)
    {
        if (!list.contains(it.key()))
            d->languageCB->addItem(it.key());
    }

    d->languageCB->setCurrentItem(currentLang.isEmpty() ? QString("x-default") : currentLang);
    slotSelectionChanged(d->languageCB->currentIndex());
}

QString AltLangStrEdit::defaultAltLang() const
{
    return d->values[QString("x-default")];
}

bool AltLangStrEdit::asDefaultAltLang() const
{
    return defaultAltLang().isNull() ? false : true;
}

void AltLangStrEdit::slotTextChanged()
{
    QString text = d->valueEdit->toPlainText();
    if (text.isEmpty())
    {
        d->addValueButton->setEnabled(false);
        return;
    }

    // we cannot trust that the text actually changed
    // (there are bogus signals caused by spell checking, see bug #141663)
    // so we have to check before marking the metadata as modified.

    bool dirty = (text != d->values[d->languageCB->currentText()]);
    d->addValueButton->setEnabled(dirty);

    if (dirty)
        emit signalModified();
}

void AltLangStrEdit::apply()
{
    slotAddValue();
}

}  // namespace Digikam
