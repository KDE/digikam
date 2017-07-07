/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprintcaptionpage.h"

// Qt includes

#include <QIcon>
#include <QPrinter>
#include <QPrinterInfo>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "advprintwizard.h"
#include "advprintphoto.h"

namespace Digikam
{

class AdvPrintCaptionPage::Private
{
public:

    template <class Ui_Class>

    class WizardUI : public QWidget, public Ui_Class
    {
    public:

        WizardUI(QWidget* const parent)
            : QWidget(parent)
        {
            this->setupUi(this);
        }
    };

    typedef WizardUI<Ui_AdvPrintCaptionPage> CaptionUI;

public:

    Private(QWizard* const dialog)
    {
        wizard    = dynamic_cast<AdvPrintWizard*>(dialog);
        captionUi = new CaptionUI(dialog);
    }

    CaptionUI*      captionUi;
    AdvPrintWizard* wizard;
};

AdvPrintCaptionPage::AdvPrintCaptionPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d(new Private(wizard))
{
    connect(this, SIGNAL(signalInfoPageUpdateCaptions()),
            wizard, SLOT(slotInfoPageUpdateCaptions()));

    connect(d->captionUi->m_captions, SIGNAL(activated(QString)),
            this, SLOT(slotCaptionChanged(QString)));

    connect(d->captionUi->m_FreeCaptionFormat, SIGNAL(editingFinished()),
            wizard, SLOT(slotInfoPageUpdateCaptions()));

    connect(d->captionUi->m_font_name, SIGNAL(currentFontChanged(QFont)),
            wizard, SLOT(slotInfoPageUpdateCaptions()));

    connect(d->captionUi->m_font_size, SIGNAL(valueChanged(int)),
            wizard, SLOT(slotInfoPageUpdateCaptions()));

    connect(d->captionUi->m_font_color, SIGNAL(signalColorSelected(QColor)),
            wizard, SLOT(slotInfoPageUpdateCaptions()));

    // -----------------------------------

    d->captionUi->mPrintList->setAllowDuplicate(true);
    d->captionUi->mPrintList->setControlButtonsPlacement(DImagesList::NoControlButtons);
    d->captionUi->mPrintList->listView()->setColumn(DImagesListView::User1,
                                        i18nc("@title:column", "Caption"),
                                        true);

    // -----------------------------------

    setPageWidget(d->captionUi);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("imagecomment")));

    readCaptionSettings();
}

AdvPrintCaptionPage::~AdvPrintCaptionPage()
{
    delete d;
}

Ui_AdvPrintCaptionPage* AdvPrintCaptionPage::ui() const
{
    return d->captionUi;
}

void AdvPrintCaptionPage::updateUi()
{
    d->captionUi->update();
}

DImagesList* AdvPrintCaptionPage::imagesList() const
{
    return d->captionUi->mPrintList;
}

void AdvPrintCaptionPage::initializePage()
{
    d->captionUi->mPrintList->setIface(d->wizard->iface());
    slotUpdateImagesList();
}

bool AdvPrintCaptionPage::validatePage()
{
    // Save the current settings
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));
    group.writeEntry(QLatin1String("Captions"),
                     d->captionUi->m_captions->currentIndex());
    group.writeEntry(QLatin1String("CaptionColor"),
                     d->captionUi->m_font_color->color());
    group.writeEntry(QLatin1String("CaptionFont"),
                     QFont(d->captionUi->m_font_name->currentFont()));
    group.writeEntry(QLatin1String("CaptionSize"),
                     d->captionUi->m_font_size->value());
    group.writeEntry(QLatin1String("CustomCaption"),
                     d->captionUi->m_FreeCaptionFormat->text());

    return true;
}

void AdvPrintCaptionPage::slotUpdateImagesList()
{
    d->captionUi->mPrintList->listView()->clear();
    d->captionUi->mPrintList->slotAddImages(d->wizard->itemsList());
}

void AdvPrintCaptionPage::blockCaptionButtons(bool block)
{
    d->captionUi->m_captions->blockSignals(block);
    d->captionUi->m_free_label->blockSignals(block);
    d->captionUi->m_font_name->blockSignals(block);
    d->captionUi->m_font_size->blockSignals(block);
    d->captionUi->m_font_color->blockSignals(block);
}

void AdvPrintCaptionPage::readCaptionSettings()
{
    KConfig config;
    KConfigGroup group = config.group(QLatin1String("PrintCreator"));

    // image captions
    d->captionUi->m_captions->setCurrentIndex(group.readEntry(QLatin1String("Captions"), 0));
    enableCaptionGroup(d->captionUi->m_captions->currentText());

    // caption color
    QColor defColor(Qt::yellow);
    QColor color = group.readEntry(QLatin1String("CaptionColor"), defColor);
    d->captionUi->m_font_color->setColor(color);

    // caption font
    QFont defFont(QLatin1String("Sans Serif"));
    QFont font = group.readEntry(QLatin1String("CaptionFont"), defFont);
    d->captionUi->m_font_name->setCurrentFont(font.family());

    // caption size
    int fontSize = group.readEntry(QLatin1String("CaptionSize"), 4);
    d->captionUi->m_font_size->setValue(fontSize);

    // free caption
    QString captionTxt = group.readEntry(QLatin1String("CustomCaption"));
    d->captionUi->m_FreeCaptionFormat->setText(captionTxt);
}

void AdvPrintCaptionPage::enableCaptionGroup(const QString& text)
{
    bool fontSettingsEnabled;

    if (text == i18n("No caption"))
    {
        fontSettingsEnabled = false;
        d->captionUi->m_customCaptionGB->setEnabled(false);
    }
    else if (text == i18n("Custom format"))
    {
        fontSettingsEnabled = true;
        d->captionUi->m_customCaptionGB->setEnabled(true);
    }
    else
    {
        fontSettingsEnabled = true;
        d->captionUi->m_customCaptionGB->setEnabled(false);
    }

    d->captionUi->m_font_name->setEnabled(fontSettingsEnabled);
    d->captionUi->m_font_size->setEnabled(fontSettingsEnabled);
    d->captionUi->m_font_color->setEnabled(fontSettingsEnabled);
}

void AdvPrintCaptionPage::slotCaptionChanged(const QString& text)
{
    enableCaptionGroup(text);
    emit signalInfoPageUpdateCaptions();
}

} // namespace Digikam
