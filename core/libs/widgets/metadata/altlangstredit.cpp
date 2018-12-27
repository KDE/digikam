/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : multi-languages string editor
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

    explicit Private()
    {
        valueEdit       = 0;
        titleLabel      = 0;
        delValueButton  = 0;
        languageCB      = 0;
        linesVisible    = 0;
        currentLanguage = QLatin1String("x-default");

        // We cannot use KLocale::allLanguagesList() here because KDE only
        // support 2 characters country codes. XMP require 2+2 characters language+country
        // following ISO 3066 (http://babelwiki.babelzilla.org/index.php?title=Language_codes)

        // The first one from the list is the Default Language code specified by XMP paper
        languageCodeMap.insert( QLatin1String("x-default"), i18n("Default Language") );

        // Standard ISO 3066 country codes.

        languageCodeMap.insert( QLatin1String("af-ZA"), i18n("Afrikaans (South Africa)") );
        languageCodeMap.insert( QLatin1String("am-ET"), i18n("Amharic (Ethiopia)") );
        languageCodeMap.insert( QLatin1String("ar-AE"), i18n("Arabic (UAE)") );
        languageCodeMap.insert( QLatin1String("ar-BH"), i18n("Arabic (Bahrain)") );
        languageCodeMap.insert( QLatin1String("ar-DZ"), i18n("Arabic (Algeria)") );
        languageCodeMap.insert( QLatin1String("ar-EG"), i18n("Arabic (Egypt)") );
        languageCodeMap.insert( QLatin1String("ar-IQ"), i18n("Arabic (Iraq)") );
        languageCodeMap.insert( QLatin1String("ar-JO"), i18n("Arabic (Jordan)") );
        languageCodeMap.insert( QLatin1String("ar-KW"), i18n("Arabic (Kuwait)") );
        languageCodeMap.insert( QLatin1String("ar-LB"), i18n("Arabic (Lebanon)") );
        languageCodeMap.insert( QLatin1String("ar-LY"), i18n("Arabic (Libya)") );
        languageCodeMap.insert( QLatin1String("ar-MA"), i18n("Arabic (Morocco)") );
        languageCodeMap.insert( QLatin1String("ar-OM"), i18n("Arabic (Oman)") );
        languageCodeMap.insert( QLatin1String("ar-QA"), i18n("Arabic (Qatar)") );
        languageCodeMap.insert( QLatin1String("ar-SA"), i18n("Arabic (Saudi Arabia)") );
        languageCodeMap.insert( QLatin1String("ar-SY"), i18n("Arabic (Syria)") );
        languageCodeMap.insert( QLatin1String("ar-TN"), i18n("Arabic (Tunisia)") );
        languageCodeMap.insert( QLatin1String("ar-YE"), i18n("Arabic (Yemen)") );
        languageCodeMap.insert( QLatin1String("as-IN"), i18n("Assamese (India)") );
        languageCodeMap.insert( QLatin1String("ba-RU"), i18n("Bashkir (Russia)") );
        languageCodeMap.insert( QLatin1String("be-BY"), i18n("Belarusian (Belarus)") );
        languageCodeMap.insert( QLatin1String("bg-BG"), i18n("Bulgarian (Bulgaria)") );
        languageCodeMap.insert( QLatin1String("bn-IN"), i18n("Bengali (India)") );
        languageCodeMap.insert( QLatin1String("bo-BT"), i18n("Tibetan (Bhutan)") );
        languageCodeMap.insert( QLatin1String("bo-CN"), i18n("Tibetan (PRC)") );
        languageCodeMap.insert( QLatin1String("br-FR"), i18n("Breton (France)") );
        languageCodeMap.insert( QLatin1String("ca-AD"), i18n("Catalan (Andorra)") );
        languageCodeMap.insert( QLatin1String("ca-ES"), i18n("Catalan (Spain)") );
        languageCodeMap.insert( QLatin1String("ca-FR"), i18n("Catalan (France)") );
        languageCodeMap.insert( QLatin1String("co-FR"), i18n("Corsican (France)") );
        languageCodeMap.insert( QLatin1String("cs-CZ"), i18n("Czech (Czechia)") );
        languageCodeMap.insert( QLatin1String("cy-GB"), i18n("Welsh (United Kingdom)") );
        languageCodeMap.insert( QLatin1String("da-DK"), i18n("Danish (Denmark)") );
        languageCodeMap.insert( QLatin1String("de-AT"), i18n("German (Austria)") );
        languageCodeMap.insert( QLatin1String("de-CH"), i18n("German (Switzerland)") );
        languageCodeMap.insert( QLatin1String("de-DE"), i18n("German (Germany)") );
        languageCodeMap.insert( QLatin1String("de-LI"), i18n("German (Liechtenstein)") );
        languageCodeMap.insert( QLatin1String("de-LU"), i18n("German (Luxembourg)") );
        languageCodeMap.insert( QLatin1String("el-GR"), i18n("Greek (Greece)") );
        languageCodeMap.insert( QLatin1String("en-AU"), i18n("English (Australia)") );
        languageCodeMap.insert( QLatin1String("en-BZ"), i18n("English (Belize)") );
        languageCodeMap.insert( QLatin1String("en-CA"), i18n("English (Canada)") );
        languageCodeMap.insert( QLatin1String("en-CB"), i18n("English (Caribbean)") );
        languageCodeMap.insert( QLatin1String("en-GB"), i18n("English (United Kingdom)") );
        languageCodeMap.insert( QLatin1String("en-IE"), i18n("English (Ireland)") );
        languageCodeMap.insert( QLatin1String("en-IN"), i18n("English (India)") );
        languageCodeMap.insert( QLatin1String("en-JA"), i18n("English (Jamaica)") );
        languageCodeMap.insert( QLatin1String("en-MY"), i18n("English (Malaysia)") );
        languageCodeMap.insert( QLatin1String("en-NZ"), i18n("English (New Zealand)") );
        languageCodeMap.insert( QLatin1String("en-PH"), i18n("English (Philippines)") );
        languageCodeMap.insert( QLatin1String("en-SG"), i18n("English (Singapore)") );
        languageCodeMap.insert( QLatin1String("en-TT"), i18n("English (Trinidad)") );
        languageCodeMap.insert( QLatin1String("en-US"), i18n("English (United States)") );
        languageCodeMap.insert( QLatin1String("en-ZA"), i18n("English (South Africa)") );
        languageCodeMap.insert( QLatin1String("en-ZW"), i18n("English (Zimbabwe)") );
        languageCodeMap.insert( QLatin1String("es-AR"), i18n("Spanish (Argentina)") );
        languageCodeMap.insert( QLatin1String("es-BO"), i18n("Spanish (Bolivia)") );
        languageCodeMap.insert( QLatin1String("es-CL"), i18n("Spanish (Chile)") );
        languageCodeMap.insert( QLatin1String("es-CO"), i18n("Spanish (Colombia)") );
        languageCodeMap.insert( QLatin1String("es-CR"), i18n("Spanish (Costa Rica)") );
        languageCodeMap.insert( QLatin1String("es-DO"), i18n("Spanish (Dominican Republic)") );
        languageCodeMap.insert( QLatin1String("es-EC"), i18n("Spanish (Ecuador)") );
        languageCodeMap.insert( QLatin1String("es-ES"), i18n("Spanish (Spain)") );
        languageCodeMap.insert( QLatin1String("es-GT"), i18n("Spanish (Guatemala)") );
        languageCodeMap.insert( QLatin1String("es-HN"), i18n("Spanish (Honduras)") );
        languageCodeMap.insert( QLatin1String("es-MX"), i18n("Spanish (Mexico)") );
        languageCodeMap.insert( QLatin1String("es-NI"), i18n("Spanish (Nicaragua)") );
        languageCodeMap.insert( QLatin1String("es-PA"), i18n("Spanish (Panama)") );
        languageCodeMap.insert( QLatin1String("es-PE"), i18n("Spanish (Peru)") );
        languageCodeMap.insert( QLatin1String("es-PR"), i18n("Spanish (Puerto Rico)") );
        languageCodeMap.insert( QLatin1String("es-PY"), i18n("Spanish (Paraguay)") );
        languageCodeMap.insert( QLatin1String("es-SV"), i18n("Spanish (El Salvador)") );
        languageCodeMap.insert( QLatin1String("es-UR"), i18n("Spanish (Uruguay)") );
        languageCodeMap.insert( QLatin1String("es-US"), i18n("Spanish (United States)") );
        languageCodeMap.insert( QLatin1String("es-VE"), i18n("Spanish (Venezuela)") );
        languageCodeMap.insert( QLatin1String("et-EE"), i18n("Estonian (Estonia)") );
        languageCodeMap.insert( QLatin1String("eu-ES"), i18n("Basque (Basque Country)") );
        languageCodeMap.insert( QLatin1String("fa-IR"), i18n("Persian (Iran)") );
        languageCodeMap.insert( QLatin1String("fi-FI"), i18n("Finnish (Finland)") );
        languageCodeMap.insert( QLatin1String("fo-FO"), i18n("Faeroese (Faero Islands)") );
        languageCodeMap.insert( QLatin1String("fr-BE"), i18n("French (Belgium)") );
        languageCodeMap.insert( QLatin1String("fr-CA"), i18n("French (Canada)") );
        languageCodeMap.insert( QLatin1String("fr-CH"), i18n("French (Switzerland)") );
        languageCodeMap.insert( QLatin1String("fr-FR"), i18n("French (France)") );
        languageCodeMap.insert( QLatin1String("fr-LU"), i18n("French (Luxembourg)") );
        languageCodeMap.insert( QLatin1String("fr-MC"), i18n("French (Monaco)") );
        languageCodeMap.insert( QLatin1String("fy-NL"), i18n("Frisian (Netherlands)") );
        languageCodeMap.insert( QLatin1String("ga-IE"), i18n("Irish (Ireland)") );
        languageCodeMap.insert( QLatin1String("gl-ES"), i18n("Galician (Galicia)") );
        languageCodeMap.insert( QLatin1String("gu-IN"), i18n("Gujarati (India)") );
        languageCodeMap.insert( QLatin1String("he-IL"), i18n("Hebrew (Israel)") );
        languageCodeMap.insert( QLatin1String("hi-IN"), i18n("Hindi (India)") );
        languageCodeMap.insert( QLatin1String("hr-BA"), i18n("Croatian (Bosnia and Herzegovina, Latin)") );
        languageCodeMap.insert( QLatin1String("hr-HR"), i18n("Croatian (Croatia)") );
        languageCodeMap.insert( QLatin1String("hu-HU"), i18n("Hungarian (Hungary)") );
        languageCodeMap.insert( QLatin1String("hy-AM"), i18n("Armenian (Armenia)") );
        languageCodeMap.insert( QLatin1String("id-ID"), i18n("(Indonesian)") );
        languageCodeMap.insert( QLatin1String("ii-CN"), i18n("Yi (PRC)") );
        languageCodeMap.insert( QLatin1String("is-IS"), i18n("Icelandic (Iceland)") );
        languageCodeMap.insert( QLatin1String("it-CH"), i18n("Italian (Switzerland)") );
        languageCodeMap.insert( QLatin1String("it-IT"), i18n("Italian (Italy)") );
        languageCodeMap.insert( QLatin1String("ja-JP"), i18n("Japanese (Japan)") );
        languageCodeMap.insert( QLatin1String("ka-GE"), i18n("Georgian (Georgia)") );
        languageCodeMap.insert( QLatin1String("kk-KZ"), i18n("Kazakh (Kazakhstan)") );
        languageCodeMap.insert( QLatin1String("kl-GL"), i18n("Greenlandic (Greenland)") );
        languageCodeMap.insert( QLatin1String("km-KH"), i18n("Khmer (Cambodia)") );
        languageCodeMap.insert( QLatin1String("kn-IN"), i18n("Kannada (India)") );
        languageCodeMap.insert( QLatin1String("ko-KR"), i18n("Korean (South Korea)") );
        languageCodeMap.insert( QLatin1String("ky-KG"), i18n("Kyrgyz (Kyrgyzstan)") );
        languageCodeMap.insert( QLatin1String("lb-LU"), i18n("Luxembourgish (Luxembourg)") );
        languageCodeMap.insert( QLatin1String("lo-LA"), i18n("Lao (Lao PDR)") );
        languageCodeMap.insert( QLatin1String("lt-LT"), i18n("Lithuanian (Lithuania)") );
        languageCodeMap.insert( QLatin1String("lv-LV"), i18n("Latvian (Latvia)") );
        languageCodeMap.insert( QLatin1String("mi-NZ"), i18n("Maori (New Zealand)") );
        languageCodeMap.insert( QLatin1String("mk-MK"), i18n("Macedonian (Macedonia)") );
        languageCodeMap.insert( QLatin1String("ml-IN"), i18n("Malayalam (India)") );
        languageCodeMap.insert( QLatin1String("mn-CN"), i18n("Mongolian (PRC)") );
        languageCodeMap.insert( QLatin1String("mn-MN"), i18n("Mongolian (Mongolia)") );
        languageCodeMap.insert( QLatin1String("mr-IN"), i18n("Marathi (India)") );
        languageCodeMap.insert( QLatin1String("ms-BN"), i18n("Malay (Brunei Darussalam)") );
        languageCodeMap.insert( QLatin1String("ms-MY"), i18n("Malay (Malaysia)") );
        languageCodeMap.insert( QLatin1String("mt-MT"), i18n("Maltese (Malta)") );
        languageCodeMap.insert( QLatin1String("nb-NO"), i18n("Norwegian Bokmål (Norway)") );
        languageCodeMap.insert( QLatin1String("ne-NP"), i18n("Nepali (Nepal)") );
        languageCodeMap.insert( QLatin1String("nl-BE"), i18n("Dutch (Belgium)") );
        languageCodeMap.insert( QLatin1String("nl-NL"), i18n("Dutch (Netherlands)") );
        languageCodeMap.insert( QLatin1String("nn-NO"), i18n("Norwegian Nynorsk (Norway)") );
        languageCodeMap.insert( QLatin1String("ns-ZA"), i18n("Sesotho sa Leboa (South Africa)") );
        languageCodeMap.insert( QLatin1String("oc-FR"), i18n("Occitan (France)") );
        languageCodeMap.insert( QLatin1String("or-IN"), i18n("Oriya (India)") );
        languageCodeMap.insert( QLatin1String("pa-IN"), i18n("Punjabi (India)") );
        languageCodeMap.insert( QLatin1String("pl-PL"), i18n("Polish (Poland)") );
        languageCodeMap.insert( QLatin1String("ps-AF"), i18n("Pashto (Afghanistan)") );
        languageCodeMap.insert( QLatin1String("pt-BR"), i18n("Portuguese (Brazil)") );
        languageCodeMap.insert( QLatin1String("pt-PT"), i18n("Portuguese (Portugal)") );
        languageCodeMap.insert( QLatin1String("rm-CH"), i18n("Romansh (Switzerland)") );
        languageCodeMap.insert( QLatin1String("ro-RO"), i18n("Romanian (Romania)") );
        languageCodeMap.insert( QLatin1String("ru-RU"), i18n("Russian (Russia)") );
        languageCodeMap.insert( QLatin1String("rw-RW"), i18n("Kinyarwanda (Rwanda)") );
        languageCodeMap.insert( QLatin1String("sa-IN"), i18n("Sanskrit (India)") );
        languageCodeMap.insert( QLatin1String("se-FI"), i18n("Sami (Northern, Finland)") );
        languageCodeMap.insert( QLatin1String("se-NO"), i18n("Sami (Northern, Norway)") );
        languageCodeMap.insert( QLatin1String("se-SE"), i18n("Sami (Northern, Sweden)") );
        languageCodeMap.insert( QLatin1String("si-LK"), i18n("Sinhala (Sri Lanka)") );
        languageCodeMap.insert( QLatin1String("sk-SK"), i18n("Slovak (Slovakia)") );
        languageCodeMap.insert( QLatin1String("sl-SI"), i18n("Slovenian (Slovenia)") );
        languageCodeMap.insert( QLatin1String("sq-AL"), i18n("Albanian (Albania)") );
        languageCodeMap.insert( QLatin1String("sv-FI"), i18n("Swedish (Finland)") );
        languageCodeMap.insert( QLatin1String("sv-SE"), i18n("Swedish (Sweden)") );
        languageCodeMap.insert( QLatin1String("sw-KE"), i18n("Swahili (Kenya)") );
        languageCodeMap.insert( QLatin1String("ta-IN"), i18n("Tamil (India)") );
        languageCodeMap.insert( QLatin1String("te-IN"), i18n("Telugu (India)") );
        languageCodeMap.insert( QLatin1String("th-TH"), i18n("Thai (Thailand)") );
        languageCodeMap.insert( QLatin1String("tk-TM"), i18n("Turkmen (Turkmenistan)") );
        languageCodeMap.insert( QLatin1String("tn-ZA"), i18n("Setswana Tswana (South Africa)") );
        languageCodeMap.insert( QLatin1String("tr-IN"), i18n("Urdu (India)") );
        languageCodeMap.insert( QLatin1String("tr-TR"), i18n("Turkish (Turkey)") );
        languageCodeMap.insert( QLatin1String("tt-RU"), i18n("Tatar (Russia)") );
        languageCodeMap.insert( QLatin1String("ug-CN"), i18n("Uighur (PRC)") );
        languageCodeMap.insert( QLatin1String("uk-UA"), i18n("Ukrainian (Ukraine)") );
        languageCodeMap.insert( QLatin1String("ur-PK"), i18n("Urdu (Pakistan)") );
        languageCodeMap.insert( QLatin1String("vi-VN"), i18n("Vietnamese (Vietnam)") );
        languageCodeMap.insert( QLatin1String("wo-SN"), i18n("Wolof (Senegal)") );
        languageCodeMap.insert( QLatin1String("xh-ZA"), i18n("isiXhosa Xhosa (South Africa)") );
        languageCodeMap.insert( QLatin1String("yo-NG"), i18n("Yoruba (Nigeria)") );
        languageCodeMap.insert( QLatin1String("zh-CN"), i18n("Chinese (PRC)") );
        languageCodeMap.insert( QLatin1String("zh-HK"), i18n("Chinese (Hong Kong SAR, PRC)") );
        languageCodeMap.insert( QLatin1String("zh-MO"), i18n("Chinese (Macao SAR)") );
        languageCodeMap.insert( QLatin1String("zh-SG"), i18n("Chinese (Singapore)") );
        languageCodeMap.insert( QLatin1String("zh-TW"), i18n("Chinese (Taiwan)") );
        languageCodeMap.insert( QLatin1String("zu-ZA"), i18n("isiZulu Zulu (South Africa)") );
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
    d->delValueButton->setIcon(QIcon::fromTheme(QLatin1String("edit-clear")));
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
        d->currentLanguage = QLatin1String("x-default");
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
              d->languageCB->setItemIcon(d->languageCB->count()-1, QIcon::fromTheme(QLatin1String("dialog-ok-apply")).pixmap(16, 16));
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
    return d->values.value(QLatin1String("x-default"));
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

} // namespace Digikam
