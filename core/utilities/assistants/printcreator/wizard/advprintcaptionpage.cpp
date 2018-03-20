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

    explicit Private(QWizard* const dialog)
      : settings(0),
        iface(0)
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
    QMap<AdvPrintSettings::CaptionType, QString> map                = AdvPrintSettings::captionTypeNames();
    QMap<AdvPrintSettings::CaptionType, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        d->captionUi->m_captionType->addItem(it.value(), (int)it.key());
        ++it;
    }

    // ----------------------------------------------------------------------

    connect(d->captionUi->m_captionType, SIGNAL(activated(int)),
            this, SLOT(slotCaptionChanged(int)));

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
}

AdvPrintCaptionPage::~AdvPrintCaptionPage()
{
    delete d;
}

DImagesList* AdvPrintCaptionPage::imagesList() const
{
    return d->captionUi->mPrintList;
}

void AdvPrintCaptionPage::initializePage()
{
    d->captionUi->m_captionType->setCurrentIndex(d->settings->captionType);
    enableCaptionGroup(d->captionUi->m_captionType->currentIndex());
    d->captionUi->m_font_color->setColor(d->settings->captionColor);
    d->captionUi->m_font_name->setCurrentFont(d->settings->captionFont.family());
    d->captionUi->m_font_size->setValue(d->settings->captionSize);
    d->captionUi->m_FreeCaptionFormat->setText(d->settings->captionTxt);
    slotCaptionChanged(d->captionUi->m_captionType->currentIndex());
    slotUpdateImagesList();
    slotUpdateCaptions();
}

bool AdvPrintCaptionPage::validatePage()
{
    d->settings->captionType  = (AdvPrintSettings::CaptionType)d->captionUi->m_captionType->currentIndex();
    d->settings->captionColor = d->captionUi->m_font_color->color();
    d->settings->captionFont  = QFont(d->captionUi->m_font_name->currentFont());
    d->settings->captionSize  = d->captionUi->m_font_size->value();
    d->settings->captionTxt   = d->captionUi->m_FreeCaptionFormat->text();

    return true;
}

void AdvPrintCaptionPage::slotUpdateImagesList()
{
    d->captionUi->mPrintList->listView()->clear();
    d->captionUi->mPrintList->slotAddImages(d->wizard->itemsList());
}

void AdvPrintCaptionPage::blockCaptionButtons(bool block)
{
    d->captionUi->m_captionType->blockSignals(block);
    d->captionUi->m_free_label->blockSignals(block);
    d->captionUi->m_font_name->blockSignals(block);
    d->captionUi->m_font_size->blockSignals(block);
    d->captionUi->m_font_color->blockSignals(block);
}

void AdvPrintCaptionPage::enableCaptionGroup(int index)
{
    bool fontSettingsEnabled;

    if (index == AdvPrintSettings::NONE)
    {
        fontSettingsEnabled = false;
        d->captionUi->m_customCaptionGB->setEnabled(false);
    }
    else if (index == AdvPrintSettings::CUSTOM)
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

void AdvPrintCaptionPage::slotCaptionChanged(int index)
{
    enableCaptionGroup(index);
    slotUpdateCaptions();
}

void AdvPrintCaptionPage::updateCaption(AdvPrintPhoto* const pPhoto)
{
    if (pPhoto)
    {
        if (!pPhoto->m_pAdvPrintCaptionInfo &&
            d->captionUi->m_captionType->currentIndex() != AdvPrintSettings::NONE)
        {
            pPhoto->m_pAdvPrintCaptionInfo = new AdvPrintCaptionInfo();
        }
        else if (pPhoto->m_pAdvPrintCaptionInfo &&
                 d->captionUi->m_captionType->currentIndex() == AdvPrintSettings::NONE)
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
                (AdvPrintSettings::CaptionType)d->captionUi->m_captionType->currentIndex();
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
                        AdvPrintSettings::NONE)
                        cap = captionFormatter(pPhoto);

                    qCDebug(DIGIKAM_GENERAL_LOG) << cap;

                    lvItem->setText(DImagesListView::User1, cap);
                }
            }
        }
    }

    // create our photo sizes list
    d->wizard->previewPhotos();
}

QString AdvPrintCaptionPage::captionFormatter(AdvPrintPhoto* const photo)
{
    if (!photo->m_pAdvPrintCaptionInfo)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Internal caption info container is NULL for"
                                       << photo->m_url;
        return QString();
    }

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
        case AdvPrintSettings::FILENAME:
            format = QLatin1String("%f");
            break;
        case AdvPrintSettings::DATETIME:
            format = QLatin1String("%d");
            break;
        case AdvPrintSettings::COMMENT:
            format = QLatin1String("%c");
            break;
        case AdvPrintSettings::CUSTOM:
            format = photo->m_pAdvPrintCaptionInfo->m_captionText;
            break;
        default:
            qCWarning(DIGIKAM_GENERAL_LOG) << "UNKNOWN caption type "
                                           << photo->m_pAdvPrintCaptionInfo->m_captionType;
            break;
    }

    format.replace(QLatin1String("\\n"), QLatin1String("\n"));

    if (photo->m_iface)
    {
        DItemInfo info(photo->m_iface->itemInfo(photo->m_url));
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

    qCDebug(DIGIKAM_GENERAL_LOG) << "Caption for"
                                 << photo->m_url
                                 << ":" << format;
    return format;
}

} // namespace Digikam
