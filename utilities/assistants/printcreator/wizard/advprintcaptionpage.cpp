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
#include <QFileInfo>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"
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
        captionUi = new CaptionUI(dialog);
        wizard    = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

    CaptionUI*        captionUi;
    AdvPrintWizard*   wizard;
    AdvPrintSettings* settings;
    DInfoInterface*   iface;
};

AdvPrintCaptionPage::AdvPrintCaptionPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d(new Private(wizard))
{
    connect(d->captionUi->m_captions, SIGNAL(activated(QString)),
            this, SLOT(slotCaptionChanged(QString)));

    connect(d->captionUi->m_FreeCaptionFormat, SIGNAL(editingFinished()),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->m_font_name, SIGNAL(currentFontChanged(QFont)),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->m_font_size, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->m_font_color, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotUpdateCaptions()));

    connect(d->captionUi->mPrintList, SIGNAL(signalImageListChanged()),
            this, SLOT(slotUpdateCaptions()));

    // -----------------------------------

    d->captionUi->mPrintList->setIface(d->iface);
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

DImagesList* AdvPrintCaptionPage::imagesList() const
{
    return d->captionUi->mPrintList;
}

void AdvPrintCaptionPage::initializePage()
{
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
    slotUpdateCaptions();
}

void AdvPrintCaptionPage::updateCaption(AdvPrintPhoto* const pPhoto)
{
    if (pPhoto)
    {
        if (!pPhoto->m_pAdvPrintCaptionInfo &&
            d->captionUi->m_captions->currentIndex() != AdvPrintCaptionInfo::NoCaptions)
        {
            pPhoto->m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo();
        }
        else if (pPhoto->m_pAdvPrintCaptionInfo &&
                 d->captionUi->m_captions->currentIndex() == AdvPrintCaptionInfo::NoCaptions)
        {
            delete pPhoto->m_pAdvPrintCaptionInfo;
            pPhoto->m_pAdvPrintCaptionInfo = NULL;
        }

        if (pPhoto->m_pAdvPrintCaptionInfo)
        {
            pPhoto->m_pAdvPrintCaptionInfo->m_captionColor =
                d->captionUi->m_font_color->color();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionSize  =
                d->captionUi->m_font_size->value();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionFont  =
                d->captionUi->m_font_name->currentFont();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionType  =
                (AdvPrintCaptionInfo::AvailableCaptions)d->captionUi->m_captions->currentIndex();
            pPhoto->m_pAdvPrintCaptionInfo->m_captionText  =
                d->captionUi->m_FreeCaptionFormat->text();

            qCDebug(DIGIKAM_GENERAL_LOG) << "Update caption properties for"
                                         << pPhoto->m_url;
        }
    }
}

void AdvPrintCaptionPage::slotUpdateCaptions()
{
    if (d->settings->photos.size())
    {
        foreach(AdvPrintPhoto* const pPhoto, d->settings->photos)
        {
            updateCaption(pPhoto);

            if (pPhoto && pPhoto->m_pAdvPrintCaptionInfo)
            {
                DImagesListViewItem* const lvItem = d->captionUi->mPrintList->listView()->findItem(pPhoto->m_url);

                if (lvItem)
                {
                    QString cap;

                    if (pPhoto->m_pAdvPrintCaptionInfo->m_captionType !=
                        AdvPrintCaptionInfo::NoCaptions)
                        cap = captionFormatter(pPhoto);

                    lvItem->setText(DImagesListView::User1, cap);
                }
            }
        }
    }

    // create our photo sizes list
    d->wizard->previewPhotos();
}

QString AdvPrintCaptionPage::captionFormatter(AdvPrintPhoto* const photo) const
{
    if (!photo->m_pAdvPrintCaptionInfo)
        return QString();

    QString resolution;
    QSize   imageSize;
    QString format;

    // %f filename
    // %c comment
    // %d date-time
    // %t exposure time
    // %i iso
    // %r resolution
    // %a aperture
    // %l focal length

    switch (photo->m_pAdvPrintCaptionInfo->m_captionType)
    {
        case AdvPrintCaptionInfo::FileNames:
            format = QLatin1String("%f");
            break;
        case AdvPrintCaptionInfo::ExifDateTime:
            format = QLatin1String("%d");
            break;
        case AdvPrintCaptionInfo::Comment:
            format = QLatin1String("%c");
            break;
        case AdvPrintCaptionInfo::Custom:
            format =  photo->m_pAdvPrintCaptionInfo->m_captionText;
            break;
        default:
            qCWarning(DIGIKAM_GENERAL_LOG) << "UNKNOWN caption type "
                                           << photo->m_pAdvPrintCaptionInfo->m_captionType;
            break;
    }

    format.replace(QLatin1String("\\n"), QLatin1String("\n"));

    if (d->iface)
    {
        DItemInfo info(d->iface->itemInfo(photo->m_url));
        imageSize = info.dimensions();

        format.replace(QString::fromUtf8("%c"), info.comment());
        format.replace(QString::fromUtf8("%d"), QLocale().toString(info.dateTime(),
                                                QLocale::ShortFormat));
        format.replace(QString::fromUtf8("%f"), info.name());
        format.replace(QString::fromUtf8("%t"), info.exposureTime());
        format.replace(QString::fromUtf8("%i"), info.sensitivity());
        format.replace(QString::fromUtf8("%a"), info.aperture());
        format.replace(QString::fromUtf8("%l"), info.focalLength());
    }
    else
    {
        QFileInfo fi(photo->m_url.toLocalFile());
        DMetadata meta(photo->m_url.toLocalFile());
        imageSize = meta.getImageDimensions();

        format.replace(QString::fromUtf8("%c"),
            meta.getImageComments()[QLatin1String("x-default")].caption);
        format.replace(QString::fromUtf8("%d"),
            QLocale().toString(meta.getImageDateTime(), QLocale::ShortFormat));
        format.replace(QString::fromUtf8("%f"), fi.fileName());

        PhotoInfoContainer photoInfo = meta.getPhotographInformation();
        format.replace(QString::fromUtf8("%t"), photoInfo.exposureTime);
        format.replace(QString::fromUtf8("%i"), photoInfo.sensitivity);
        format.replace(QString::fromUtf8("%a"), photoInfo.aperture);
        format.replace(QString::fromUtf8("%l"), photoInfo.focalLength);
    }

    if (imageSize.isValid())
    {
        resolution = QString::fromUtf8("%1x%2").arg(imageSize.width()).arg(imageSize.height());
    }

    format.replace(QString::fromUtf8("%r"), resolution);

    return format;
}

} // namespace Digikam
