/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
 *
 * Copyright (C) 2009-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Mikkel Baekhoej Christensen <mbc at baekhoej dot dk>
 * Copyright (C) 2017      by Ahmed Fathi <ahmed dot fathi dot abdelmageed at gmail dot com>
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

#include "watermark.h"

// Qt includes

#include <QFontMetrics>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QRect>
#include <QWidget>
#include <QGridLayout>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QApplication>
#include <QStyle>
#include <QFontComboBox>
#include <QComboBox>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dfontproperties.h"
#include "dwidgetutils.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "dimg.h"
#include "blurfilter.h"
#include "loadsavethread.h"
#include "metaengine.h"

namespace Digikam
{

class WaterMark::Private
{

public:

    enum WaterMarkPositon
    {
        TopLeft = 0,
        TopRight,
        BottomLeft,
        BottomRight,
        Center
    };

public:

    Private() :
        textSettingsGroupBox(0),
        imageSettingsGroupBox(0),
        useAbsoluteImageSizeGroupBox(0),
        useImageRadioButton(0),
        ignoreWatermarkAspectCheckBox(0),
        useAbsoluteSizeCheckBox(0),
        useTextRadioButton(0),
        useBackgroundCheckBox(0),
        imageFileUrlRequester(0),
        textEdit(0),
        comboBox(0),
        rotationComboBox(0),
        extendedFontChooserWidget(0),
        fontColorButton(0),
        backgroundColorButton(0),
        textOpacity(0),
        backgroundOpacity(0),
        xMarginInput(0),
        yMarginInput(0),
        waterMarkSizePercent(0),
        changeSettings(true)
    {
    }

    QGroupBox*       textSettingsGroupBox;
    QGroupBox*       imageSettingsGroupBox;
    QGroupBox*       useAbsoluteImageSizeGroupBox ;
    QRadioButton*    useImageRadioButton;
    QCheckBox*       ignoreWatermarkAspectCheckBox;
    QCheckBox*       useAbsoluteSizeCheckBox;
    QRadioButton*    useTextRadioButton;
    QCheckBox*       useBackgroundCheckBox;

    DFileSelector*   imageFileUrlRequester;
    QLineEdit*       textEdit;

    QComboBox*       comboBox;
    QComboBox*       rotationComboBox;
    DFontProperties* extendedFontChooserWidget;

    DColorSelector*  fontColorButton;
    DColorSelector*  backgroundColorButton;

    DIntNumInput*    textOpacity;
    DIntNumInput*    backgroundOpacity;
    DIntNumInput*    xMarginInput;
    DIntNumInput*    yMarginInput;
    DIntNumInput*    waterMarkSizePercent;

    bool             changeSettings;
};

WaterMark::WaterMark(QObject* const parent)
    : BatchTool(QLatin1String("WaterMark"), DecorateTool, parent),
      d(new Private)
{
    setToolTitle(i18n("Add Watermark"));
    setToolDescription(i18n("Overlay an image or text as a visible watermark"));
    setToolIconName(QLatin1String("insert-text"));
}

WaterMark::~WaterMark()
{
    delete d;
}

void WaterMark::registerSettingsWidget()
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    DVBox* const vbox = new DVBox;
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(spacing);

    DHBox* const hbox = new DHBox(vbox);
    hbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox->setSpacing(10);

    d->useAbsoluteImageSizeGroupBox                       = new QGroupBox(vbox);
    QVBoxLayout* const useAbsoluteImageSizeGroupBoxLayout = new QVBoxLayout;
    useAbsoluteImageSizeGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    useAbsoluteImageSizeGroupBoxLayout->addStretch(1);
    d->useAbsoluteImageSizeGroupBox->setLayout(useAbsoluteImageSizeGroupBoxLayout);


    DHBox* const useAbsoluteSizeHBox   = new DHBox();
    useAbsoluteSizeHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useAbsoluteSizeHBox->setSpacing(10);
    d->useAbsoluteSizeCheckBox         = new QCheckBox(useAbsoluteSizeHBox);
    d->useAbsoluteSizeCheckBox->setWhatsThis(i18n("Check this if you want the watermark to use the given size of the font or the image "
                                                  "without any adjustment to the actual image"));
    QLabel* const useAbsoluteSizeLabel = new QLabel(useAbsoluteSizeHBox);
    useAbsoluteSizeLabel->setText(i18n("Use Absolute Size"));
    d->useAbsoluteSizeCheckBox->setChecked(false);

    useAbsoluteImageSizeGroupBoxLayout->addWidget(useAbsoluteSizeHBox);

    QLabel* const watermarkTypeLabel   = new QLabel(hbox);
    watermarkTypeLabel->setText(i18n("Watermark type:"));

    d->useImageRadioButton      = new QRadioButton(hbox);
    d->useImageRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* const useImageLabel = new QLabel(hbox);
    useImageLabel ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->useTextRadioButton       = new QRadioButton(hbox);
    d->useTextRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* const useTextLabel  = new QLabel(hbox);
    useTextLabel ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useImageLabel->setText(i18n("Image"));
    useTextLabel->setText(i18n("Text"));

    useImageLabel->setAlignment(Qt::AlignLeft);
    useTextLabel->setAlignment(Qt::AlignLeft);
    d->useImageRadioButton->setChecked(true);

    d->imageSettingsGroupBox = new QGroupBox(vbox);
    d->imageSettingsGroupBox->setTitle(i18n("Image settings"));
    QVBoxLayout* const imageSettingsGroupBoxLayout = new QVBoxLayout;
    imageSettingsGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    imageSettingsGroupBoxLayout->addStretch(1);
    d->imageSettingsGroupBox->setLayout(imageSettingsGroupBoxLayout);

    QLabel* const label      = new QLabel();
    d->imageFileUrlRequester = new DFileSelector();
    d->imageFileUrlRequester->lineEdit()->setPlaceholderText(i18n("Click to select watermark image."));
    label->setText(i18n("Watermark image:"));
    imageSettingsGroupBoxLayout->addWidget(label);
    imageSettingsGroupBoxLayout->addWidget(d->imageFileUrlRequester);

    DHBox* const ignoreWatermarkAspectRatioHBox   = new DHBox();
    ignoreWatermarkAspectRatioHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ignoreWatermarkAspectRatioHBox->setSpacing(5);

    d->ignoreWatermarkAspectCheckBox              = new QCheckBox(ignoreWatermarkAspectRatioHBox);
    d->ignoreWatermarkAspectCheckBox->setWhatsThis(i18n("Check this if you want the watermark to ignore "
                                                        "its own aspect ratio and use the image's aspect ratio instead"));

    QLabel* const ignoreWatermarkAspectRatioLabel = new QLabel(ignoreWatermarkAspectRatioHBox);
    ignoreWatermarkAspectRatioLabel->setText(i18n("Ignore Watermark aspect Ratio"));
    d->ignoreWatermarkAspectCheckBox->setChecked(false);
    imageSettingsGroupBoxLayout->addWidget(ignoreWatermarkAspectRatioHBox);

    d->textSettingsGroupBox                       = new QGroupBox(vbox);
    d->textSettingsGroupBox->setTitle(i18n("Text settings"));
    QVBoxLayout* const textSettingsGroupBoxLayout = new QVBoxLayout;
    textSettingsGroupBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    textSettingsGroupBoxLayout->addStretch(1);
    d->textSettingsGroupBox->setLayout(textSettingsGroupBoxLayout);

    QLabel* const textEditLabel = new QLabel(vbox);
    d->textEdit                 = new QLineEdit(vbox);
    d->textEdit->setClearButtonEnabled(true);
    d->textEdit->setPlaceholderText(i18n("Enter your watermark string here."));
    textEditLabel->setText(i18n("Watermark text:"));
    textSettingsGroupBoxLayout->addWidget(textEditLabel);
    textSettingsGroupBoxLayout->addWidget(d->textEdit);

    d->extendedFontChooserWidget = new DFontProperties(0, DFontProperties::NoDisplayFlags);
    d->extendedFontChooserWidget->setSampleBoxVisible(true);
    d->extendedFontChooserWidget->enableColumn(0x04,false);
    d->extendedFontChooserWidget->setWhatsThis(i18n("choose the font type and style. size is auto calculated"));
    textSettingsGroupBoxLayout->addWidget(d->extendedFontChooserWidget);

    QLabel* const label3 = new QLabel();
    d->fontColorButton   = new DColorSelector();
    d->fontColorButton->setColor(Qt::black);
    d->fontColorButton->setWhatsThis(i18n("Set the font color to use here"));
    label3->setText(i18n("Font color:"));
    textSettingsGroupBoxLayout->addWidget(label3);
    textSettingsGroupBoxLayout->addWidget(d->fontColorButton);

    QLabel* const textOpacityLabel = new QLabel();
    textOpacityLabel->setText(i18n("Text opacity:"));
    d->textOpacity                 = new DIntNumInput();
    d->textOpacity->setRange(0, 100, 1);
    d->textOpacity->setDefaultValue(100);
    d->textOpacity->setWhatsThis(i18n("Set the opacity of the watermark text. 100 is fully opaque, 0 is fully transparent."));
    textSettingsGroupBoxLayout->addWidget(textOpacityLabel);
    textSettingsGroupBoxLayout->addWidget(d->textOpacity);

    DHBox* const useBackgroundHBox   = new DHBox();
    useBackgroundHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useBackgroundHBox->setSpacing(5);

    d->useBackgroundCheckBox         = new QCheckBox(useBackgroundHBox);
    d->useBackgroundCheckBox->setWhatsThis(i18n("Check this if you want a background fill behind the text"));
    QLabel* const useBackgroundLabel = new QLabel(useBackgroundHBox);
    useBackgroundLabel->setText(i18n("Use background"));
    textSettingsGroupBoxLayout->addWidget(useBackgroundHBox);

    QLabel* const backgroundColorLabel = new QLabel();
    d->backgroundColorButton           = new DColorSelector();
    d->backgroundColorButton->setColor(QColor(0xCC, 0xCC, 0xCC));
    d->backgroundColorButton->setWhatsThis(i18n("Choose the color of the watermark background"));
    backgroundColorLabel ->setText(i18n("Background color:"));
    textSettingsGroupBoxLayout->addWidget(backgroundColorLabel);
    textSettingsGroupBoxLayout->addWidget(d->backgroundColorButton);

    QLabel* const backgroundOpacityLabel = new QLabel();
    backgroundOpacityLabel->setText(i18n("Background opacity:"));
    d->backgroundOpacity                 = new DIntNumInput();
    d->backgroundOpacity->setRange(0, 100, 1);
    d->backgroundOpacity->setDefaultValue(100);
    d->backgroundOpacity->setWhatsThis(i18n("Set the opacity of the watermark background. 100 is fully opaque, 0 is fully transparent."));
    textSettingsGroupBoxLayout->addWidget(backgroundOpacityLabel);
    textSettingsGroupBoxLayout->addWidget(d->backgroundOpacity);

    d->imageSettingsGroupBox->setVisible(true);
    d->textSettingsGroupBox->setVisible(false);

    QLabel* const label4 = new QLabel(vbox);
    d->comboBox          = new QComboBox(vbox);
    d->comboBox->insertItem(Private::TopLeft,     i18n("Top left"));
    d->comboBox->insertItem(Private::TopRight,    i18n("Top right"));
    d->comboBox->insertItem(Private::BottomLeft,  i18n("Bottom left"));
    d->comboBox->insertItem(Private::BottomRight, i18n("Bottom right"));
    d->comboBox->insertItem(Private::Center,      i18n("Center"));
    label4->setText(i18n("Placement:"));

    QLabel* const labelRotation  = new QLabel(vbox);
    d->rotationComboBox          = new QComboBox(vbox);
    d->rotationComboBox->insertItem(0,     i18n("0 degrees"));
    d->rotationComboBox->insertItem(1,     i18n("90 degrees CW"));
    d->rotationComboBox->insertItem(2,     i18n("180 degrees"));
    d->rotationComboBox->insertItem(3,     i18n("270 degrees CW"));
    labelRotation->setText(i18n("Rotation:"));

    QLabel* const label5    = new QLabel(vbox);
    d->waterMarkSizePercent = new DIntNumInput(vbox);
    d->waterMarkSizePercent->setRange(0, 100, 1);
    d->waterMarkSizePercent->setDefaultValue(30);
    d->waterMarkSizePercent->setWhatsThis(i18n("Size of watermark, as a percentage of the marked image."));
    label5->setText(i18n("Size (%):"));

    QLabel* const label6 = new QLabel(vbox);
    d->xMarginInput      = new DIntNumInput(vbox);
    d->xMarginInput->setRange(0, 100, 1);
    d->xMarginInput->setDefaultValue(2);
    d->xMarginInput->setWhatsThis(i18n("Margin from edge in X direction, as a percentage of the marked image"));
    label6->setText(i18n("X margin (%):"));

    QLabel* const label7 = new QLabel(vbox);
    d->yMarginInput      = new DIntNumInput(vbox);
    d->yMarginInput->setRange(0, 100, 1);
    d->yMarginInput->setDefaultValue(2);
    d->yMarginInput->setWhatsThis(i18n("Margin from edge in Y direction, as a percentage of the marked image"));
    label7->setText(i18n("Y margin (%):"));

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget    = vbox;

    // ------------------------------------------------------------------------------------------------------

    connect(d->useImageRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useTextRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->imageFileUrlRequester->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->extendedFontChooserWidget, SIGNAL(fontSelected(QFont)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useBackgroundCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->ignoreWatermarkAspectCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useAbsoluteSizeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->backgroundColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotSettingsChanged()));

    connect(d->backgroundOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));
    connect(d->rotationComboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->waterMarkSizePercent, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->yMarginInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->xMarginInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings WaterMark::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("Use image"),          true);
    settings.insert(QLatin1String("Watermark image"),    QString());
    settings.insert(QLatin1String("Text"),               QString());
    settings.insert(QLatin1String("Font"),               QFont());
    settings.insert(QLatin1String("Color"),              QColor(Qt::black));
    settings.insert(QLatin1String("Text opacity"),       100);
    settings.insert(QLatin1String("Use background"),     true);
    settings.insert(QLatin1String("Background color"),   QColor(0xCC, 0xCC, 0xCC));
    settings.insert(QLatin1String("Background opacity"), 0xCC);
    settings.insert(QLatin1String("Placement"),          Private::BottomRight);
    settings.insert(QLatin1String("Rotation"),           0);
    settings.insert(QLatin1String("Watermark size"),     25);
    settings.insert(QLatin1String("X margin"),           2);
    settings.insert(QLatin1String("Y margin"),           2);

    return settings;
}

void WaterMark::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->useImageRadioButton->setChecked(settings()[QLatin1String("Use image")].toBool());
    d->useTextRadioButton->setChecked(!settings()[QLatin1String("Use image")].toBool());
    d->imageFileUrlRequester->setFileDlgPath(settings()[QLatin1String("Watermark image")].toString());
    d->textEdit->setText(settings()[QLatin1String("Text")].toString());
    d->fontColorButton->setColor(settings()[QLatin1String("Color")].toString());
    d->textOpacity->setValue(settings()[QLatin1String("Text opacity")].toInt());
    d->useBackgroundCheckBox->setChecked(settings()[QLatin1String("Use background")].toBool());
    d->backgroundColorButton->setColor(settings()[QLatin1String("Background color")].toString());
    d->backgroundOpacity->setValue(settings()[QLatin1String("Background opacity")].toInt());
    d->comboBox->setCurrentIndex(settings()[QLatin1String("Placement")].toInt());
    d->rotationComboBox->setCurrentIndex(settings()[QLatin1String("Rotation")].toInt());
    d->waterMarkSizePercent->setValue(settings()[QLatin1String("Watermark size")].toInt());
    d->xMarginInput->setValue(settings()[QLatin1String("X margin")].toInt());
    d->yMarginInput->setValue(settings()[QLatin1String("Y margin")].toInt());
    d->changeSettings = true;
}

void WaterMark::slotSettingsChanged()
{
    if (d->useImageRadioButton->isChecked())
    {
        d->textSettingsGroupBox->setVisible(false);
        d->imageSettingsGroupBox->setVisible(true);
    }
    else if (d->useTextRadioButton->isChecked())
    {
        d->imageSettingsGroupBox->setVisible(false);
        d->textSettingsGroupBox->setVisible(true);
    }

    d->waterMarkSizePercent->setEnabled(!d->useAbsoluteSizeCheckBox->isChecked());
    d->extendedFontChooserWidget->enableColumn(0x04,d->useAbsoluteSizeCheckBox->isChecked());

    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("Use image"),                     d->useImageRadioButton->isChecked());
        settings.insert(QLatin1String("Watermark image"),               d->imageFileUrlRequester->fileDlgPath());
        settings.insert(QLatin1String("Text"),                          d->textEdit->text());
        settings.insert(QLatin1String("Font"),                          d->extendedFontChooserWidget->font());
        settings.insert(QLatin1String("Color"),                         d->fontColorButton->color());
        settings.insert(QLatin1String("Text opacity"),                  d->textOpacity->value());
        settings.insert(QLatin1String("Use background"),                d->useBackgroundCheckBox->isChecked());
        settings.insert(QLatin1String("Ignore Watermark Aspect Ratio"), d->ignoreWatermarkAspectCheckBox->isChecked());
        settings.insert(QLatin1String("Use Absolute Size"),             d->useAbsoluteSizeCheckBox->isChecked());
        settings.insert(QLatin1String("Background color"),              d->backgroundColorButton->color());
        settings.insert(QLatin1String("Background opacity"),            d->backgroundOpacity->value());
        settings.insert(QLatin1String("Placement"),                     (int)d->comboBox->currentIndex());
        settings.insert(QLatin1String("Rotation"),                      (int)d->rotationComboBox->currentIndex());
        settings.insert(QLatin1String("Watermark size"),                (int)d->waterMarkSizePercent->value());
        settings.insert(QLatin1String("X margin"),                      (int)d->xMarginInput->value());
        settings.insert(QLatin1String("Y margin"),                      (int)d->yMarginInput->value());
        BatchTool::slotSettingsChanged(settings);
    }
}

bool WaterMark::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    QString fileName                             = settings()[QLatin1String("Watermark image")].toString();
    int placement                                = settings()[QLatin1String("Placement")].toInt();
    int size                                     = settings()[QLatin1String("Watermark size")].toInt();
    int xMargin                                  = settings()[QLatin1String("X margin")].toInt();
    int yMargin                                  = settings()[QLatin1String("Y margin")].toInt();
    bool useImage                                = settings()[QLatin1String("Use image")].toBool();
    QString text                                 = settings()[QLatin1String("Text")].toString();
    QFont font                                   = qvariant_cast<QFont>(settings()[QLatin1String("Font")]);;

    QColor fontColor                             = settings()[QLatin1String("Color")].toString();
    int textOpacity                              = settings()[QLatin1String("Text opacity")].toInt();
    bool useBackground                           = settings()[QLatin1String("Use background")].toBool();
    QColor backgroundColor                       = settings()[QLatin1String("Background color")].toString();
    int backgroundOpacity                        = settings()[QLatin1String("Background opacity")].toInt();
    Qt::AspectRatioMode watermarkAspectRatioMode = settings()[QLatin1String("Ignore Watermark Aspect Ratio")].toBool() ?
                                                              Qt::IgnoreAspectRatio : Qt::KeepAspectRatio;

    bool useAbsoluteSize                         = settings()[QLatin1String("Use Absolute Size")].toBool();

    DImg watermarkImage;
    DColorComposer* const composer               = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
    int marginW                                  = lround(image().width()  * (xMargin / 100.0));
    int marginH                                  = lround(image().height() * (yMargin / 100.0));
    int rotationIndex = settings()[QLatin1String("Rotation")].toInt();

    DImg::ANGLE rotationAngle = (rotationIndex == 1) ? DImg::ANGLE::ROT90 :
                                (rotationIndex == 2) ? DImg::ANGLE::ROT180 :
                                (rotationIndex == 3) ? DImg::ANGLE::ROT270 :
                                                       DImg::ANGLE::ROTNONE;

    // rotate and/or flip the image depending on the exif information to allow for the expected watermark placement.
    //note that this operation is reversed after proper watermark generation to leave everything as it was.
    LoadSaveThread::exifRotate(image(), inputUrl().toLocalFile());

    float ratio = (float)image().height()/image().width();

    if (rotationAngle == DImg::ANGLE::ROT90 || rotationAngle == DImg::ANGLE::ROT270)
    {
        size    = size * ratio;
    }
    else
    {
        // For Images whose height are much larger than their width, this helps keep
        // the watermark size reasonable
        if (ratio > 1.5)
        {
            int tempSize  = size * ratio;

            if(tempSize < 35)
                tempSize *= 1.5;

            size          = (tempSize < 100) ? tempSize : 100;
        }
    }

    if (useImage)
    {
        watermarkImage = DImg(fileName);

        if (watermarkImage.isNull())
        {
            return false;
        }

        if (!useAbsoluteSize)
        {
            DImg tempImage = watermarkImage.smoothScale(image().width()  * size / 100,
                                                        image().height() * size / 100,
                                                        watermarkAspectRatioMode);
            watermarkImage = tempImage;
        }
    }
    else
    {
        int alignMode;
        const int radius = 10;

        if (text.isEmpty())
        {
            return false;
        }

        int fontSize = queryFontSize(text, font, size);

        if (fontSize == 0)
        {
            return false;
        }

        switch (placement)
        {
            case Private::TopLeft:
                alignMode = Qt::AlignLeft;
                break;

            case Private::TopRight:
                alignMode = Qt::AlignRight;
                break;

            case Private::BottomLeft:
                alignMode = Qt::AlignLeft;
                break;

            case Private::Center:
                alignMode = Qt::AlignCenter;
                break;

            default :    // BottomRight
                alignMode = Qt::AlignRight;
                break;
        }

        if (!useAbsoluteSize)
        {
            font.setPointSizeF(fontSize);
        }

        QFontMetrics fontMt(font);
        QRect fontRect = fontMt.boundingRect(radius, radius, image().width(), image().height(), 0, text);

        // Add a transparent layer.
        QRect backgroundRect(fontRect.x() - radius, fontRect.y() - radius,
                             fontRect.width() + 2 * radius, fontRect.height() + 2 * radius);
        DImg backgroundLayer(backgroundRect.width(), backgroundRect.height(), image().sixteenBit(), true);
        DColor transparent(QColor(0, 0, 0));
        transparent.setAlpha(0);

        if (image().sixteenBit())
        {
            transparent.convertToSixteenBit();
        }

        backgroundLayer.fill(transparent);

        DImg grayTransLayer(fontRect.width(), fontRect.height(), image().sixteenBit(), true);

        if (useBackground)
        {
            DColor grayTrans(backgroundColor);
            grayTrans.setAlpha(backgroundOpacity * 255 / 100);

            if (image().sixteenBit())
            {
                grayTrans.convertToSixteenBit();
            }

            grayTransLayer.fill(grayTrans);
            backgroundLayer.bitBlendImage(composer, &grayTransLayer, 0, 0,
                                          grayTransLayer.width(), grayTransLayer.height(),
                                          radius, radius);
        }

        BlurFilter blur(&backgroundLayer, 0L, radius);
        blur.startFilterDirectly();
        backgroundLayer.putImageData(blur.getTargetImage().bits());

        // Draw text
        QImage img = backgroundLayer.copyQImage(fontRect);
        QPainter p(&img);
        fontColor.setAlpha(textOpacity * 255 / 100);
        p.setPen(QPen(fontColor, 1));
        p.setFont(font);
        p.save();
        p.drawText(0, 0, fontRect.width(), fontRect.height(), alignMode, text);
        p.restore();
        p.end();

        watermarkImage = DImg(img);
    }

    watermarkImage.convertToDepthOfImage(&image());

    QRect watermarkRect(0, 0, watermarkImage.width(), watermarkImage.height());

    int xAdditionalValue = 0;
    int yAdditionalValue = 0;

    watermarkImage.rotate(rotationAngle);

    switch (placement)
    {
        case Private::TopLeft:
            watermarkRect.moveTopLeft(QPoint(marginW, marginH));
            break;

        case Private::TopRight:

            if (rotationAngle == DImg::ANGLE::ROT270 || rotationAngle == DImg::ANGLE::ROT90)
            {
                xAdditionalValue += watermarkRect.width() - watermarkRect.height();
            }

            watermarkRect.moveTopRight(QPoint(image().width() + xAdditionalValue -1 - marginW, marginH));
            break;

        case Private::BottomLeft:

            if (rotationAngle == DImg::ANGLE::ROT90 || rotationAngle == DImg::ANGLE::ROT270)
            {
                yAdditionalValue += watermarkRect.height() - watermarkRect.width();
            }

            watermarkRect.moveBottomLeft(QPoint(marginW, image().height() + yAdditionalValue - 1 - marginH));
            break;

        case Private::Center:

            if (rotationAngle == DImg::ANGLE::ROT90 || rotationAngle == DImg::ANGLE::ROT270)
            {
                xAdditionalValue += (watermarkRect.width() - watermarkRect.height())/2;
                yAdditionalValue += (watermarkRect.height() - watermarkRect.width())/2;
            }

            watermarkRect.moveCenter(QPoint((int)(image().width()  / 2 + xAdditionalValue),
                                            (int)(image().height() / 2 + yAdditionalValue)));
            break;

        default :    // BottomRight

            if (rotationAngle == DImg::ANGLE::ROT90 || rotationAngle == DImg::ANGLE::ROT270)
            {
                xAdditionalValue += watermarkRect.width() - watermarkRect.height();
                yAdditionalValue += watermarkRect.height() - watermarkRect.width();
            }

            watermarkRect.moveBottomRight(QPoint(image().width()  + xAdditionalValue - 1 - marginW,
                                                 image().height() + yAdditionalValue -1 - marginH));
            break;
    }

    // TODO: Create watermark filter, move code there, implement FilterAction

    image().bitBlendImage(composer, &watermarkImage, 0, 0, watermarkImage.width(), watermarkImage.height(),
                          watermarkRect.left(), watermarkRect.top());

    delete composer;
    LoadSaveThread::reverseExifRotate(image(), inputUrl().toLocalFile());

    return (savefromDImg());
}

int WaterMark::queryFontSize(const QString& text, const QFont& font, int length) const
{
    // Find font size using relative length compared to image width.
    QFont fnt = font;
    QRect fontRect;

    for (int i = 1 ; i <= 1000 ; ++i)
    {
        fnt.setPointSizeF(i);
        QFontMetrics fontMt(fnt);
        fontRect = fontMt.boundingRect(0, 0, image().width(), image().height(), 0, text);

        if (fontRect.width() > lround((image().width() * length) / 100.0))
        {
            return (i - 1);
        }
    }

    return 0;
}

} // namespace Digikam
