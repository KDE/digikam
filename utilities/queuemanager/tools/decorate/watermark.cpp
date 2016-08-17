/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Mikkel Baekhoej Christensen <mbc at baekhoej dot dk>
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

#include "dwidgetutils.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "dimg.h"
#include "blurfilter.h"



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
        useImageRadioButton(0),
        useTextRadioButton(0),
        useBackgroundCheckBox(0),
        imageFileUrlRequester(0),
        textEdit(0),
        comboBox(0),
        fontChooserWidget(0),
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

    QGroupBox*      textSettingsGroupBox;
    QGroupBox*      imageSettingsGroupBox;
    QRadioButton*   useImageRadioButton;
    QRadioButton*   useTextRadioButton;
    QCheckBox*      useBackgroundCheckBox;

    DFileSelector*  imageFileUrlRequester;
    QLineEdit*      textEdit;

    QComboBox*      comboBox;
    QFontComboBox*  fontChooserWidget;

    DColorSelector* fontColorButton;
    DColorSelector* backgroundColorButton;

    DIntNumInput*   textOpacity;
    DIntNumInput*   backgroundOpacity;
    DIntNumInput*   xMarginInput;
    DIntNumInput*   yMarginInput;
    DIntNumInput*   waterMarkSizePercent;

    bool            changeSettings;
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

    QLabel* const watermarkTypeLabel = new QLabel(hbox);
    watermarkTypeLabel->setText(i18n("Watermark type:"));

    d->useImageRadioButton       = new QRadioButton(hbox);
    d->useImageRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* const useImageLabel  = new QLabel(hbox);
    useImageLabel ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->useTextRadioButton        = new QRadioButton(hbox);
    d->useTextRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* const useTextLabel   = new QLabel(hbox);
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

    d->textSettingsGroupBox = new QGroupBox(vbox);
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

    QLabel* const label2 = new QLabel();
    d->fontChooserWidget = new QFontComboBox(vbox);
    d->fontChooserWidget->setWhatsThis(i18n("Here you can choose the font to be used."));
    label2->setText(i18n("Font:"));
    textSettingsGroupBoxLayout->addWidget(label2);
    textSettingsGroupBoxLayout->addWidget(d->fontChooserWidget);

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

    m_settingsWidget = vbox;

    // ------------------------------------------------------------------------------------------------------

    connect(d->useImageRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useTextRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->imageFileUrlRequester->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontChooserWidget, SIGNAL(currentFontChanged(QFont)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useBackgroundCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->backgroundColorButton, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotSettingsChanged()));

    connect(d->backgroundOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->comboBox, SIGNAL(activated(int)),
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
    d->imageFileUrlRequester->lineEdit()->setText(settings()[QLatin1String("Watermark image")].toString());
    d->textEdit->setText(settings()[QLatin1String("Text")].toString());
    d->fontChooserWidget->setFont(settings()[QLatin1String("Font")].toString());
    d->fontColorButton->setColor(settings()[QLatin1String("Color")].toString());
    d->textOpacity->setValue(settings()[QLatin1String("Text opacity")].toInt());
    d->useBackgroundCheckBox->setChecked(settings()[QLatin1String("Use background")].toBool());
    d->backgroundColorButton->setColor(settings()[QLatin1String("Background color")].toString());
    d->backgroundOpacity->setValue(settings()[QLatin1String("Background opacity")].toInt());
    d->comboBox->setCurrentIndex(settings()[QLatin1String("Placement")].toInt());
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

    if (d->changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("Use image"),          d->useImageRadioButton->isChecked());
        settings.insert(QLatin1String("Watermark image"),    d->imageFileUrlRequester->lineEdit()->text());
        settings.insert(QLatin1String("Text"),               d->textEdit->text());
        settings.insert(QLatin1String("Font"),               d->fontChooserWidget->currentFont());
        settings.insert(QLatin1String("Color"),              d->fontColorButton->color());
        settings.insert(QLatin1String("Text opacity"),       d->textOpacity->value());
        settings.insert(QLatin1String("Use background"),     d->useBackgroundCheckBox->isChecked());
        settings.insert(QLatin1String("Background color"),   d->backgroundColorButton->color());
        settings.insert(QLatin1String("Background opacity"), d->backgroundOpacity->value());
        settings.insert(QLatin1String("Placement"),          (int)d->comboBox->currentIndex());
        settings.insert(QLatin1String("Watermark size"),     (int)d->waterMarkSizePercent->value());
        settings.insert(QLatin1String("X margin"),           (int)d->xMarginInput->value());
        settings.insert(QLatin1String("Y margin"),           (int)d->yMarginInput->value());
        BatchTool::slotSettingsChanged(settings);
    }
}

bool WaterMark::toolOperations()
{

    if (!loadToDImg())
    {
        return false;
    }

    QString fileName        = settings()[QLatin1String("Watermark image")].toString();
    int placement           = settings()[QLatin1String("Placement")].toInt();
    int size                = settings()[QLatin1String("Watermark size")].toInt();
    int xMargin             = settings()[QLatin1String("X margin")].toInt();
    int yMargin             = settings()[QLatin1String("Y margin")].toInt();
    bool useImage           = settings()[QLatin1String("Use image")].toBool();

    QString text            = settings()[QLatin1String("Text")].toString();
    QFont font              = settings()[QLatin1String("Font")].toString();
    QColor fontColor        = settings()[QLatin1String("Color")].toString();
    int textOpacity         = settings()[QLatin1String("Text opacity")].toInt();
    bool useBackground      = settings()[QLatin1String("Use background")].toBool();
    QColor backgroundColor  = settings()[QLatin1String("Background color")].toString();
    int backgroundOpacity   = settings()[QLatin1String("Background opacity")].toInt();


    DImg watermarkImage;
    DColorComposer* composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);
    int marginW              = lround(image().width()  * (xMargin / 100.0));
    int marginH              = lround(image().height() * (yMargin / 100.0));

    if (useImage)
    {
        watermarkImage = DImg(fileName);

        if (watermarkImage.isNull())
        {
            return false;
        }

        DImg tempImage = watermarkImage.smoothScale(image().width() * size / 100, image().height() * size / 100, Qt::KeepAspectRatio);
        watermarkImage = tempImage;
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

        font.setPointSizeF(fontSize);
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

    switch (placement)
    {
        case Private::TopLeft:
            watermarkRect.moveTopLeft(QPoint(marginW, marginH));
            break;

        case Private::TopRight:
            watermarkRect.moveTopRight(QPoint(image().width() - marginW, marginH));
            break;

        case Private::BottomLeft:
            watermarkRect.moveBottomLeft(QPoint(marginW, image().height() - marginH));
            break;

        case Private::Center:
            watermarkRect.moveCenter(QPoint((int)(image().width() / 2), (int)(image().height() / 2)));
            break;

        default :    // BottomRight
            watermarkRect.moveBottomRight(QPoint(image().width() - marginW, image().height() - marginH));
            break;
    }

    // TODO: Create watermark filter, move code there, implement FilterAction

    image().bitBlendImage(composer, &watermarkImage, 0, 0, watermarkImage.width(), watermarkImage.height(),
                          watermarkRect.left(), watermarkRect.top());

    delete composer;

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

}  // namespace Digikam
