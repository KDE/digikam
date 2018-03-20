/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages string editor
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QEvent>
#include <QMap>
#include <QStyle>
#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QApplication>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN AltLangStrEdit::Private
{
public:

    Private()
    {
        valueEdit       = 0;
        titleLabel      = 0;
        delValueButton  = 0;
        languageCB      = 0;
        linesVisible    = 0;
        currentLanguage = QString::fromLatin1("x-default");

        // We cannot use KLocale::allLanguagesList() here because KDE only
        // support 2 characters country codes. XMP require 2+2 characters language+country
        // following ISO 3066 (http://babelwiki.babelzilla.org/index.php?title=Language_codes)

        // The first one from the list is the Default Language code specified by XMP paper
        languageCodeMap.insert( QString::fromLatin1("x-default"), i18n("Default Language") );

        // Standard ISO 3066 country codes.

        languageCodeMap.insert( QString::fromLatin1("af-ZA"), i18n("Afrikaans (South Africa)") );
        languageCodeMap.insert( QString::fromLatin1("am-ET"), i18n("Amharic (Ethiopia)") );
        languageCodeMap.insert( QString::fromLatin1("ar-AE"), i18n("Arabic (UAE)") );
        languageCodeMap.insert( QString::fromLatin1("ar-BH"), i18n("Arabic (Bahrain)") );
        languageCodeMap.insert( QString::fromLatin1("ar-DZ"), i18n("Arabic (Algeria)") );
        languageCodeMap.insert( QString::fromLatin1("ar-EG"), i18n("Arabic (Egypt)") );
        languageCodeMap.insert( QString::fromLatin1("ar-IQ"), i18n("Arabic (Iraq)") );
        languageCodeMap.insert( QString::fromLatin1("ar-JO"), i18n("Arabic (Jordan)") );
        languageCodeMap.insert( QString::fromLatin1("ar-KW"), i18n("Arabic (Kuwait)") );
        languageCodeMap.insert( QString::fromLatin1("ar-LB"), i18n("Arabic (Lebanon)") );
        languageCodeMap.insert( QString::fromLatin1("ar-LY"), i18n("Arabic (Libya)") );
        languageCodeMap.insert( QString::fromLatin1("ar-MA"), i18n("Arabic (Morocco)") );
        languageCodeMap.insert( QString::fromLatin1("ar-OM"), i18n("Arabic (Oman)") );
        languageCodeMap.insert( QString::fromLatin1("ar-QA"), i18n("Arabic (Qatar)") );
        languageCodeMap.insert( QString::fromLatin1("ar-SA"), i18n("Arabic (Saudi Arabia)") );
        languageCodeMap.insert( QString::fromLatin1("ar-SY"), i18n("Arabic (Syria)") );
        languageCodeMap.insert( QString::fromLatin1("ar-TN"), i18n("Arabic (Tunisia)") );
        languageCodeMap.insert( QString::fromLatin1("ar-YE"), i18n("Arabic (Yemen)") );
        languageCodeMap.insert( QString::fromLatin1("as-IN"), i18n("Assamese (India)") );
        languageCodeMap.insert( QString::fromLatin1("ba-RU"), i18n("Bashkir (Russia)") );
        languageCodeMap.insert( QString::fromLatin1("be-BY"), i18n("Belarusian (Belarus)") );
        languageCodeMap.insert( QString::fromLatin1("bg-BG"), i18n("Bulgarian (Bulgaria)") );
        languageCodeMap.insert( QString::fromLatin1("bn-IN"), i18n("Bengali (India)") );
        languageCodeMap.insert( QString::fromLatin1("bo-BT"), i18n("Tibetan (Bhutan)") );
        languageCodeMap.insert( QString::fromLatin1("bo-CN"), i18n("Tibetan (PRC)") );
        languageCodeMap.insert( QString::fromLatin1("br-FR"), i18n("Breton (France)") );
        languageCodeMap.insert( QString::fromLatin1("ca-AD"), i18n("Catalan (Andorra)") );
        languageCodeMap.insert( QString::fromLatin1("ca-ES"), i18n("Catalan (Spain)") );
        languageCodeMap.insert( QString::fromLatin1("ca-FR"), i18n("Catalan (France)") );
        languageCodeMap.insert( QString::fromLatin1("co-FR"), i18n("Corsican (France)") );
        languageCodeMap.insert( QString::fromLatin1("cs-CZ"), i18n("Czech (Czechia)") );
        languageCodeMap.insert( QString::fromLatin1("cy-GB"), i18n("Welsh (United Kingdom)") );
        languageCodeMap.insert( QString::fromLatin1("da-DK"), i18n("Danish (Denmark)") );
        languageCodeMap.insert( QString::fromLatin1("de-AT"), i18n("German (Austria)") );
        languageCodeMap.insert( QString::fromLatin1("de-CH"), i18n("German (Switzerland)") );
        languageCodeMap.insert( QString::fromLatin1("de-DE"), i18n("German (Germany)") );
        languageCodeMap.insert( QString::fromLatin1("de-LI"), i18n("German (Liechtenstein)") );
        languageCodeMap.insert( QString::fromLatin1("de-LU"), i18n("German (Luxembourg)") );
        languageCodeMap.insert( QString::fromLatin1("el-GR"), i18n("Greek (Greece)") );
        languageCodeMap.insert( QString::fromLatin1("en-AU"), i18n("English (Australia)") );
        languageCodeMap.insert( QString::fromLatin1("en-BZ"), i18n("English (Belize)") );
        languageCodeMap.insert( QString::fromLatin1("en-CA"), i18n("English (Canada)") );
        languageCodeMap.insert( QString::fromLatin1("en-CB"), i18n("English (Caribbean)") );
        languageCodeMap.insert( QString::fromLatin1("en-GB"), i18n("English (United Kingdom)") );
        languageCodeMap.insert( QString::fromLatin1("en-IE"), i18n("English (Ireland)") );
        languageCodeMap.insert( QString::fromLatin1("en-IN"), i18n("English (India)") );
        languageCodeMap.insert( QString::fromLatin1("en-JA"), i18n("English (Jamaica)") );
        languageCodeMap.insert( QString::fromLatin1("en-MY"), i18n("English (Malaysia)") );
        languageCodeMap.insert( QString::fromLatin1("en-NZ"), i18n("English (New Zealand)") );
        languageCodeMap.insert( QString::fromLatin1("en-PH"), i18n("English (Philippines)") );
        languageCodeMap.insert( QString::fromLatin1("en-SG"), i18n("English (Singapore)") );
        languageCodeMap.insert( QString::fromLatin1("en-TT"), i18n("English (Trinidad)") );
        languageCodeMap.insert( QString::fromLatin1("en-US"), i18n("English (United States)") );
        languageCodeMap.insert( QString::fromLatin1("en-ZA"), i18n("English (South Africa)") );
        languageCodeMap.insert( QString::fromLatin1("en-ZW"), i18n("English (Zimbabwe)") );
        languageCodeMap.insert( QString::fromLatin1("es-AR"), i18n("Spanish (Argentina)") );
        languageCodeMap.insert( QString::fromLatin1("es-BO"), i18n("Spanish (Bolivia)") );
        languageCodeMap.insert( QString::fromLatin1("es-CL"), i18n("Spanish (Chile)") );
        languageCodeMap.insert( QString::fromLatin1("es-CO"), i18n("Spanish (Colombia)") );
        languageCodeMap.insert( QString::fromLatin1("es-CR"), i18n("Spanish (Costa Rica)") );
        languageCodeMap.insert( QString::fromLatin1("es-DO"), i18n("Spanish (Dominican Republic)") );
        languageCodeMap.insert( QString::fromLatin1("es-EC"), i18n("Spanish (Ecuador)") );
        languageCodeMap.insert( QString::fromLatin1("es-ES"), i18n("Spanish (Spain)") );
        languageCodeMap.insert( QString::fromLatin1("es-GT"), i18n("Spanish (Guatemala)") );
        languageCodeMap.insert( QString::fromLatin1("es-HN"), i18n("Spanish (Honduras)") );
        languageCodeMap.insert( QString::fromLatin1("es-MX"), i18n("Spanish (Mexico)") );
        languageCodeMap.insert( QString::fromLatin1("es-NI"), i18n("Spanish (Nicaragua)") );
        languageCodeMap.insert( QString::fromLatin1("es-PA"), i18n("Spanish (Panama)") );
        languageCodeMap.insert( QString::fromLatin1("es-PE"), i18n("Spanish (Peru)") );
        languageCodeMap.insert( QString::fromLatin1("es-PR"), i18n("Spanish (Puerto Rico)") );
        languageCodeMap.insert( QString::fromLatin1("es-PY"), i18n("Spanish (Paraguay)") );
        languageCodeMap.insert( QString::fromLatin1("es-SV"), i18n("Spanish (El Salvador)") );
        languageCodeMap.insert( QString::fromLatin1("es-UR"), i18n("Spanish (Uruguay)") );
        languageCodeMap.insert( QString::fromLatin1("es-US"), i18n("Spanish (United States)") );
        languageCodeMap.insert( QString::fromLatin1("es-VE"), i18n("Spanish (Venezuela)") );
        languageCodeMap.insert( QString::fromLatin1("et-EE"), i18n("Estonian (Estonia)") );
        languageCodeMap.insert( QString::fromLatin1("eu-ES"), i18n("Basque (Basque Country)") );
        languageCodeMap.insert( QString::fromLatin1("fa-IR"), i18n("Persian (Iran)") );
        languageCodeMap.insert( QString::fromLatin1("fi-FI"), i18n("Finnish (Finland)") );
        languageCodeMap.insert( QString::fromLatin1("fo-FO"), i18n("Faeroese (Faero Islands)") );
        languageCodeMap.insert( QString::fromLatin1("fr-BE"), i18n("French (Belgium)") );
        languageCodeMap.insert( QString::fromLatin1("fr-CA"), i18n("French (Canada)") );
        languageCodeMap.insert( QString::fromLatin1("fr-CH"), i18n("French (Switzerland)") );
        languageCodeMap.insert( QString::fromLatin1("fr-FR"), i18n("French (France)") );
        languageCodeMap.insert( QString::fromLatin1("fr-LU"), i18n("French (Luxembourg)") );
        languageCodeMap.insert( QString::fromLatin1("fr-MC"), i18n("French (Monaco)") );
        languageCodeMap.insert( QString::fromLatin1("fy-NL"), i18n("Frisian (Netherlands)") );
        languageCodeMap.insert( QString::fromLatin1("ga-IE"), i18n("Irish (Ireland)") );
        languageCodeMap.insert( QString::fromLatin1("gl-ES"), i18n("Galician (Galicia)") );
        languageCodeMap.insert( QString::fromLatin1("gu-IN"), i18n("Gujarati (India)") );
        languageCodeMap.insert( QString::fromLatin1("he-IL"), i18n("Hebrew (Israel)") );
        languageCodeMap.insert( QString::fromLatin1("hi-IN"), i18n("Hindi (India)") );
        languageCodeMap.insert( QString::fromLatin1("hr-BA"), i18n("Croatian (Bosnia and Herzegovina, Latin)") );
        languageCodeMap.insert( QString::fromLatin1("hr-HR"), i18n("Croatian (Croatia)") );
        languageCodeMap.insert( QString::fromLatin1("hu-HU"), i18n("Hungarian (Hungary)") );
        languageCodeMap.insert( QString::fromLatin1("hy-AM"), i18n("Armenian (Armenia)") );
        languageCodeMap.insert( QString::fromLatin1("id-ID"), i18n("(Indonesian)") );
        languageCodeMap.insert( QString::fromLatin1("ii-CN"), i18n("Yi (PRC)") );
        languageCodeMap.insert( QString::fromLatin1("is-IS"), i18n("Icelandic (Iceland)") );
        languageCodeMap.insert( QString::fromLatin1("it-CH"), i18n("Italian (Switzerland)") );
        languageCodeMap.insert( QString::fromLatin1("it-IT"), i18n("Italian (Italy)") );
        languageCodeMap.insert( QString::fromLatin1("ja-JP"), i18n("Japanese (Japan)") );
        languageCodeMap.insert( QString::fromLatin1("ka-GE"), i18n("Georgian (Georgia)") );
        languageCodeMap.insert( QString::fromLatin1("kk-KZ"), i18n("Kazakh (Kazakhstan)") );
        languageCodeMap.insert( QString::fromLatin1("kl-GL"), i18n("Greenlandic (Greenland)") );
        languageCodeMap.insert( QString::fromLatin1("km-KH"), i18n("Khmer (Cambodia)") );
        languageCodeMap.insert( QString::fromLatin1("kn-IN"), i18n("Kannada (India)") );
        languageCodeMap.insert( QString::fromLatin1("ko-KR"), i18n("Korean (South Korea)") );
        languageCodeMap.insert( QString::fromLatin1("ky-KG"), i18n("Kyrgyz (Kyrgyzstan)") );
        languageCodeMap.insert( QString::fromLatin1("lb-LU"), i18n("Luxembourgish (Luxembourg)") );
        languageCodeMap.insert( QString::fromLatin1("lo-LA"), i18n("Lao (Lao PDR)") );
        languageCodeMap.insert( QString::fromLatin1("lt-LT"), i18n("Lithuanian (Lithuania)") );
        languageCodeMap.insert( QString::fromLatin1("lv-LV"), i18n("Latvian (Latvia)") );
        languageCodeMap.insert( QString::fromLatin1("mi-NZ"), i18n("Maori (New Zealand)") );
        languageCodeMap.insert( QString::fromLatin1("mk-MK"), i18n("Macedonian (Macedonia)") );
        languageCodeMap.insert( QString::fromLatin1("ml-IN"), i18n("Malayalam (India)") );
        languageCodeMap.insert( QString::fromLatin1("mn-CN"), i18n("Mongolian (PRC)") );
        languageCodeMap.insert( QString::fromLatin1("mn-MN"), i18n("Mongolian (Mongolia)") );
        languageCodeMap.insert( QString::fromLatin1("mr-IN"), i18n("Marathi (India)") );
        languageCodeMap.insert( QString::fromLatin1("ms-BN"), i18n("Malay (Brunei Darussalam)") );
        languageCodeMap.insert( QString::fromLatin1("ms-MY"), i18n("Malay (Malaysia)") );
        languageCodeMap.insert( QString::fromLatin1("mt-MT"), i18n("Maltese (Malta)") );
        languageCodeMap.insert( QString::fromLatin1("nb-NO"), i18n("Norwegian Bokmål (Norway)") );
        languageCodeMap.insert( QString::fromLatin1("ne-NP"), i18n("Nepali (Nepal)") );
        languageCodeMap.insert( QString::fromLatin1("nl-BE"), i18n("Dutch (Belgium)") );
        languageCodeMap.insert( QString::fromLatin1("nl-NL"), i18n("Dutch (Netherlands)") );
        languageCodeMap.insert( QString::fromLatin1("nn-NO"), i18n("Norwegian Nynorsk (Norway)") );
        languageCodeMap.insert( QString::fromLatin1("ns-ZA"), i18n("Sesotho sa Leboa (South Africa)") );
        languageCodeMap.insert( QString::fromLatin1("oc-FR"), i18n("Occitan (France)") );
        languageCodeMap.insert( QString::fromLatin1("or-IN"), i18n("Oriya (India)") );
        languageCodeMap.insert( QString::fromLatin1("pa-IN"), i18n("Punjabi (India)") );
        languageCodeMap.insert( QString::fromLatin1("pl-PL"), i18n("Polish (Poland)") );
        languageCodeMap.insert( QString::fromLatin1("ps-AF"), i18n("Pashto (Afghanistan)") );
        languageCodeMap.insert( QString::fromLatin1("pt-BR"), i18n("Portuguese (Brazil)") );
        languageCodeMap.insert( QString::fromLatin1("pt-PT"), i18n("Portuguese (Portugal)") );
        languageCodeMap.insert( QString::fromLatin1("rm-CH"), i18n("Romansh (Switzerland)") );
        languageCodeMap.insert( QString::fromLatin1("ro-RO"), i18n("Romanian (Romania)") );
        languageCodeMap.insert( QString::fromLatin1("ru-RU"), i18n("Russian (Russia)") );
        languageCodeMap.insert( QString::fromLatin1("rw-RW"), i18n("Kinyarwanda (Rwanda)") );
        languageCodeMap.insert( QString::fromLatin1("sa-IN"), i18n("Sanskrit (India)") );
        languageCodeMap.insert( QString::fromLatin1("se-FI"), i18n("Sami (Northern, Finland)") );
        languageCodeMap.insert( QString::fromLatin1("se-NO"), i18n("Sami (Northern, Norway)") );
        languageCodeMap.insert( QString::fromLatin1("se-SE"), i18n("Sami (Northern, Sweden)") );
        languageCodeMap.insert( QString::fromLatin1("si-LK"), i18n("Sinhala (Sri Lanka)") );
        languageCodeMap.insert( QString::fromLatin1("sk-SK"), i18n("Slovak (Slovakia)") );
        languageCodeMap.insert( QString::fromLatin1("sl-SI"), i18n("Slovenian (Slovenia)") );
        languageCodeMap.insert( QString::fromLatin1("sq-AL"), i18n("Albanian (Albania)") );
        languageCodeMap.insert( QString::fromLatin1("sv-FI"), i18n("Swedish (Finland)") );
        languageCodeMap.insert( QString::fromLatin1("sv-SE"), i18n("Swedish (Sweden)") );
        languageCodeMap.insert( QString::fromLatin1("sw-KE"), i18n("Swahili (Kenya)") );
        languageCodeMap.insert( QString::fromLatin1("ta-IN"), i18n("Tamil (India)") );
        languageCodeMap.insert( QString::fromLatin1("te-IN"), i18n("Telugu (India)") );
        languageCodeMap.insert( QString::fromLatin1("th-TH"), i18n("Thai (Thailand)") );
        languageCodeMap.insert( QString::fromLatin1("tk-TM"), i18n("Turkmen (Turkmenistan)") );
        languageCodeMap.insert( QString::fromLatin1("tn-ZA"), i18n("Setswana Tswana (South Africa)") );
        languageCodeMap.insert( QString::fromLatin1("tr-IN"), i18n("Urdu (India)") );
        languageCodeMap.insert( QString::fromLatin1("tr-TR"), i18n("Turkish (Turkey)") );
        languageCodeMap.insert( QString::fromLatin1("tt-RU"), i18n("Tatar (Russia)") );
        languageCodeMap.insert( QString::fromLatin1("ug-CN"), i18n("Uighur (PRC)") );
        languageCodeMap.insert( QString::fromLatin1("uk-UA"), i18n("Ukrainian (Ukraine)") );
        languageCodeMap.insert( QString::fromLatin1("ur-PK"), i18n("Urdu (Pakistan)") );
        languageCodeMap.insert( QString::fromLatin1("vi-VN"), i18n("Vietnamese (Vietnam)") );
        languageCodeMap.insert( QString::fromLatin1("wo-SN"), i18n("Wolof (Senegal)") );
        languageCodeMap.insert( QString::fromLatin1("xh-ZA"), i18n("isiXhosa Xhosa (South Africa)") );
        languageCodeMap.insert( QString::fromLatin1("yo-NG"), i18n("Yoruba (Nigeria)") );
        languageCodeMap.insert( QString::fromLatin1("zh-CN"), i18n("Chinese (PRC)") );
        languageCodeMap.insert( QString::fromLatin1("zh-HK"), i18n("Chinese (Hong Kong SAR, PRC)") );
        languageCodeMap.insert( QString::fromLatin1("zh-MO"), i18n("Chinese (Macao SAR)") );
        languageCodeMap.insert( QString::fromLatin1("zh-SG"), i18n("Chinese (Singapore)") );
        languageCodeMap.insert( QString::fromLatin1("zh-TW"), i18n("Chinese (Taiwan)") );
        languageCodeMap.insert( QString::fromLatin1("zu-ZA"), i18n("isiZulu Zulu (South Africa)") );
    }

    ~Private()
    {
        languageCodeMap.clear();
    }

public:

    typedef QMap<QString, QString> LanguageCodeMap;

    LanguageCodeMap                languageCodeMap;

    QString                        currentLanguage;

    uint                           linesVisible;

    QLabel*                        titleLabel;

    QToolButton*                   delValueButton;

    QTextEdit*                     valueEdit;

    QComboBox*                     languageCB;

    MetaEngine::AltLangMap         values;
};

AltLangStrEdit::AltLangStrEdit(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QGridLayout* const grid = new QGridLayout(this);
    d->titleLabel           = new QLabel(this);
    d->delValueButton       = new QToolButton(this);
    d->delValueButton->setIcon(QIcon::fromTheme(QString::fromLatin1("edit-clear")));
    d->delValueButton->setToolTip(i18n("Remove entry for this language"));
    d->delValueButton->setEnabled(false);

    d->languageCB = new QComboBox(this);
    d->languageCB->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    d->languageCB->setWhatsThis(i18n("Select item language here."));

    d->valueEdit  = new QTextEdit(this);
    d->valueEdit->setAcceptRichText(false);

    // --------------------------------------------------------

    grid->setAlignment( Qt::AlignTop );
    grid->addWidget(d->titleLabel,     0, 0, 1, 1);
    grid->addWidget(d->languageCB,     0, 2, 1, 1);
    grid->addWidget(d->delValueButton, 0, 3, 1, 1);
    grid->addWidget(d->valueEdit,      1, 0, 1,-1);
    grid->setColumnStretch(1, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    loadLangAltListEntries();

    // --------------------------------------------------------

    connect(d->languageCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AltLangStrEdit::slotSelectionChanged);

    connect(d->delValueButton, &QToolButton::clicked,
            this, &AltLangStrEdit::slotDeleteValue);

    connect(d->valueEdit, &QTextEdit::textChanged,
            this, &AltLangStrEdit::slotTextChanged);
}

AltLangStrEdit::~AltLangStrEdit()
{
    delete d;
}

QString AltLangStrEdit::currentLanguageCode() const
{
    return d->currentLanguage;
}

void AltLangStrEdit::setCurrentLanguageCode(const QString& lang)
{
    if (d->currentLanguage.isEmpty())
    {
        d->currentLanguage = QString::fromLatin1("x-default");
    }
    else
    {
        d->currentLanguage = lang;
    }
}

QString AltLangStrEdit::languageCode(int index) const
{
    return d->languageCB->itemText(index);
}

void AltLangStrEdit::setTitle(const QString& title)
{
    d->titleLabel->setText(title);
}

void AltLangStrEdit::setPlaceholderText(const QString& msg)
{
    d->valueEdit->setPlaceholderText(msg);
}

void AltLangStrEdit::reset()
{
    setValues(MetaEngine::AltLangMap());
}

void AltLangStrEdit::slotDeleteValue()
{
    d->values.remove(d->currentLanguage);
    setValues(d->values);
    emit signalValueDeleted(d->currentLanguage);
}

void AltLangStrEdit::slotSelectionChanged()
{
    d->currentLanguage = d->languageCB->currentText();

    // There are bogus signals caused by spell checking, see bug #141663.
    // so we must block signals here.

    d->valueEdit->blockSignals(true);

    QString text = d->values.value(d->currentLanguage);
    d->valueEdit->setPlainText(text);
    d->delValueButton->setEnabled(!text.isNull());

    d->valueEdit->blockSignals(false);

    d->languageCB->setToolTip(d->languageCodeMap.value(d->currentLanguage));

    emit signalSelectionChanged(d->currentLanguage);
}

void AltLangStrEdit::setValues(const MetaEngine::AltLangMap& values)
{
    d->values    = values;
    loadLangAltListEntries();

    d->valueEdit->blockSignals(true);

    QString text = d->values.value(d->currentLanguage);
    d->valueEdit->setPlainText(text);
    d->delValueButton->setEnabled(!text.isNull());

    d->valueEdit->blockSignals(false);
}

MetaEngine::AltLangMap& AltLangStrEdit::values() const
{
    return d->values;
}

void AltLangStrEdit::loadLangAltListEntries()
{
    d->languageCB->blockSignals(true);

    d->languageCB->clear();

    // In first we fill already assigned languages.

    QStringList list = d->values.keys();

    if (!list.isEmpty())
    {
        foreach(const QString& item, list)
        {
              d->languageCB->addItem(item);
              d->languageCB->setItemIcon(d->languageCB->count()-1, QIcon::fromTheme(QString::fromLatin1("dialog-ok-apply")).pixmap(16, 16));
        }

        d->languageCB->insertSeparator(d->languageCB->count());
    }

    // ...and now, all the rest...

    for (Private::LanguageCodeMap::Iterator it = d->languageCodeMap.begin();
         it != d->languageCodeMap.end(); ++it)
    {
        if (!list.contains(it.key()))
        {
            d->languageCB->addItem(it.key());
        }
    }

    d->languageCB->setCurrentIndex(d->languageCB->findText(d->currentLanguage));

    d->languageCB->blockSignals(false);
}

QString AltLangStrEdit::defaultAltLang() const
{
    return d->values.value(QString::fromLatin1("x-default"));
}

bool AltLangStrEdit::asDefaultAltLang() const
{
    return !defaultAltLang().isNull();
}

void AltLangStrEdit::slotTextChanged()
{
    QString editedText   = d->valueEdit->toPlainText();
    QString previousText = d->values.value(d->currentLanguage);

    if (editedText.isEmpty())
    {
        slotDeleteValue();
    }
    else if (previousText.isNull())
    {
        addCurrent();
    }
    else if (editedText != previousText)
    {
        // we cannot trust that the text actually changed
        // (there are bogus signals caused by spell checking, see bug #141663)
        // so we have to check before marking the metadata as modified.
        d->values.insert(d->currentLanguage, editedText);
        emit signalModified(d->currentLanguage, editedText);
    }
}

void AltLangStrEdit::addCurrent()
{
    QString text = d->valueEdit->toPlainText();

    d->values.insert(d->currentLanguage, text);
    loadLangAltListEntries();
    d->delValueButton->setEnabled(true);
    emit signalValueAdded(d->currentLanguage, text);
}

void AltLangStrEdit::setLinesVisible(uint lines)
{
    d->linesVisible = lines;

    if (d->linesVisible == 0)
    {
        d->valueEdit->setFixedHeight(QWIDGETSIZE_MAX); // reset
    }
    else
    {
        d->valueEdit->setFixedHeight(d->valueEdit->fontMetrics().lineSpacing() * d->linesVisible         +
                                     d->valueEdit->contentsMargins().top()                               +
                                     d->valueEdit->contentsMargins().bottom()                            +
                                     1                                                                   +
                                     2*(d->valueEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) +
                                        d->valueEdit->style()->pixelMetric(QStyle::PM_FocusFrameVMargin))
                                    );
    }

    // It's not possible to display scrollbar properly if size is too small
    if (d->linesVisible < 3)
    {
        d->valueEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

uint AltLangStrEdit::linesVisible() const
{
    return d->linesVisible;
}

void AltLangStrEdit::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::FontChange)
    {
        setLinesVisible(linesVisible());
    }

    QWidget::changeEvent(e);
}

QTextEdit* AltLangStrEdit::textEdit() const
{
    return d->valueEdit;
}

}  // namespace Digikam
