/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "watermark.moc"

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

// KDE includes

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kfontcombobox.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kvbox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "blurfilter.h"
#include "imagedialog.h"

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

    QGroupBox*     textSettingsGroupBox;
    QGroupBox*     imageSettingsGroupBox;
    QRadioButton*  useImageRadioButton;
    QRadioButton*  useTextRadioButton;
    QCheckBox*     useBackgroundCheckBox;

    KUrlRequester* imageFileUrlRequester;
    KLineEdit*     textEdit;

    KComboBox*     comboBox;
    KFontComboBox* fontChooserWidget;

    KColorButton*  fontColorButton;
    KColorButton*  backgroundColorButton;

    KIntNumInput*  textOpacity;
    KIntNumInput*  backgroundOpacity;
    KIntNumInput*  xMarginInput;
    KIntNumInput*  yMarginInput;
    KIntNumInput*  waterMarkSizePercent;

    bool          changeSettings;
};

WaterMark::WaterMark(QObject* const parent)
    : BatchTool("WaterMark", DecorateTool, parent),
      d(new Private)
{
    setToolTitle(i18n("Add Watermark"));
    setToolDescription(i18n("Overlay an image or text as a visible watermark"));
    setToolIconName("insert-text");
}

WaterMark::~WaterMark()
{
    delete d;
}

void WaterMark::registerSettingsWidget()
{

    KVBox* vbox = new KVBox;
    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(0);

    KHBox* hbox = new KHBox(vbox);
    hbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox->setSpacing(10);

    QLabel* watermarkTypeLabel = new QLabel(hbox);
    watermarkTypeLabel->setText(i18n("Watermark type:"));

    d->useImageRadioButton = new QRadioButton(hbox);
    d->useImageRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* useImageLabel  = new QLabel(hbox);
    useImageLabel ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->useTextRadioButton  = new QRadioButton(hbox);
    d->useTextRadioButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel* useTextLabel   = new QLabel(hbox);
    useTextLabel ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useImageLabel->setText(i18n("Image"));
    useTextLabel->setText(i18n("Text"));

    useImageLabel->setAlignment(Qt::AlignLeft);
    useTextLabel->setAlignment(Qt::AlignLeft);
    d->useImageRadioButton->setChecked(true);

    d->imageSettingsGroupBox = new QGroupBox(vbox);
    d->imageSettingsGroupBox->setTitle(i18n("Image settings"));
    QVBoxLayout* imageSettingsGroupBoxLayout = new QVBoxLayout;
    imageSettingsGroupBoxLayout->setMargin(KDialog::spacingHint());
    imageSettingsGroupBoxLayout->addStretch(1);
    d->imageSettingsGroupBox->setLayout(imageSettingsGroupBoxLayout);

    QLabel* label = new QLabel();
    d->imageFileUrlRequester = new KUrlRequester();
    d->imageFileUrlRequester->setClickMessage(i18n("Click to select watermark image."));
    label->setText(i18n("Watermark image:"));
    imageSettingsGroupBoxLayout->addWidget(label);
    imageSettingsGroupBoxLayout->addWidget(d->imageFileUrlRequester);

    d->textSettingsGroupBox = new QGroupBox(vbox);
    d->textSettingsGroupBox->setTitle(i18n("Text settings"));
    QVBoxLayout* textSettingsGroupBoxLayout = new QVBoxLayout;
    textSettingsGroupBoxLayout->setMargin(KDialog::spacingHint());
    textSettingsGroupBoxLayout->addStretch(1);
    d->textSettingsGroupBox->setLayout(textSettingsGroupBoxLayout);

    QLabel* textEditLabel = new QLabel(vbox);
    d->textEdit   = new KLineEdit(vbox);
    d->textEdit->setClearButtonShown(true);
    d->textEdit->setClickMessage(i18n("Enter your watermark string here."));
    textEditLabel->setText(i18n("Watermark text:"));
    textSettingsGroupBoxLayout->addWidget(textEditLabel);
    textSettingsGroupBoxLayout->addWidget(d->textEdit);

    QLabel* label2       = new QLabel();
    d->fontChooserWidget = new KFontComboBox(vbox);
    d->fontChooserWidget->setWhatsThis(i18n("Here you can choose the font to be used."));
    label2->setText(i18n("Font:"));
    textSettingsGroupBoxLayout->addWidget(label2);
    textSettingsGroupBoxLayout->addWidget(d->fontChooserWidget);

    QLabel* label3     = new QLabel();
    d->fontColorButton = new KColorButton(Qt::black);
    d->fontColorButton->setWhatsThis(i18n("Set the font color to use here"));
    label3->setText(i18n("Font color:"));
    textSettingsGroupBoxLayout->addWidget(label3);
    textSettingsGroupBoxLayout->addWidget(d->fontColorButton);

    QLabel* textOpacityLabel = new QLabel();
    textOpacityLabel->setText(i18n("Text opacity:"));
    d->textOpacity = new KIntNumInput();
    d->textOpacity->setRange(0, 100);
    d->textOpacity->setValue(100);
    d->textOpacity->setSliderEnabled(true);
    d->textOpacity->setWhatsThis(i18n("Set the opacity of the watermark text. 100 is fully opaque, 0 is fully transparent."));
    textSettingsGroupBoxLayout->addWidget(textOpacityLabel);
    textSettingsGroupBoxLayout->addWidget(d->textOpacity);

    KHBox* useBackgroundHBox = new KHBox();
    useBackgroundHBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    useBackgroundHBox->setSpacing(5);
    d->useBackgroundCheckBox = new QCheckBox(useBackgroundHBox);
    d->useBackgroundCheckBox->setWhatsThis(i18n("Check this if you want a background fill behind the text"));
    QLabel* useBackgroundLabel = new QLabel(useBackgroundHBox);
    useBackgroundLabel->setText(i18n("Use background"));
    textSettingsGroupBoxLayout->addWidget(useBackgroundHBox);

    QLabel* backgroundColorLabel = new QLabel();
    d->backgroundColorButton = new KColorButton(QColor(0xCC, 0xCC, 0xCC));
    d->backgroundColorButton->setWhatsThis(i18n("Choose the color of the watermark background"));
    backgroundColorLabel ->setText(i18n("Background color:"));
    textSettingsGroupBoxLayout->addWidget(backgroundColorLabel);
    textSettingsGroupBoxLayout->addWidget(d->backgroundColorButton);

    QLabel* backgroundOpacityLabel = new QLabel();
    backgroundOpacityLabel->setText(i18n("Background opacity:"));
    d->backgroundOpacity = new KIntNumInput();
    d->backgroundOpacity->setRange(0, 100);
    d->backgroundOpacity->setValue(0xCC);
    d->backgroundOpacity->setSliderEnabled(true);
    d->backgroundOpacity->setWhatsThis(i18n("Set the opacity of the watermark background. 100 is fully opaque, 0 is fully transparent."));
    textSettingsGroupBoxLayout->addWidget(backgroundOpacityLabel);
    textSettingsGroupBoxLayout->addWidget(d->backgroundOpacity);

    d->imageSettingsGroupBox->setVisible(true);
    d->textSettingsGroupBox->setVisible(false);

    QLabel* label4 = new QLabel(vbox);
    d->comboBox    = new KComboBox(vbox);
    d->comboBox->insertItem(Private::TopLeft,     i18n("Top left"));
    d->comboBox->insertItem(Private::TopRight,    i18n("Top right"));
    d->comboBox->insertItem(Private::BottomLeft,  i18n("Bottom left"));
    d->comboBox->insertItem(Private::BottomRight, i18n("Bottom right"));
    d->comboBox->insertItem(Private::Center,      i18n("Center"));
    label4->setText(i18n("Placement:"));

    QLabel* label5  = new QLabel(vbox);
    d->waterMarkSizePercent = new KIntNumInput(vbox);
    d->waterMarkSizePercent->setRange(0, 100);
    d->waterMarkSizePercent->setValue(30);
    d->waterMarkSizePercent->setSliderEnabled(true);
    d->waterMarkSizePercent->setWhatsThis(i18n("Size of watermark, as a percentage of the marked image."));
    label5->setText(i18n("Size (%):"));

    QLabel* label6  = new QLabel(vbox);
    d->xMarginInput = new KIntNumInput(vbox);
    d->xMarginInput->setRange(0, 100);
    d->xMarginInput->setValue(2);
    d->xMarginInput->setSliderEnabled(true);
    d->xMarginInput->setWhatsThis(i18n("Margin from edge in X direction, as a percentage of the marked image"));
    label6->setText(i18n("X margin (%):"));

    QLabel* label7  = new QLabel(vbox);
    d->yMarginInput = new KIntNumInput(vbox);
    d->yMarginInput->setRange(0, 100);
    d->yMarginInput->setValue(2);
    d->yMarginInput->setSliderEnabled(true);
    d->yMarginInput->setWhatsThis(i18n("Margin from edge in Y direction, as a percentage of the marked image"));
    label7->setText(i18n("Y margin (%):"));

    QLabel* space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget = vbox;

    // ------------------------------------------------------------------------------------------------------

    connect(d->useImageRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useTextRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->imageFileUrlRequester, SIGNAL(textChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontChooserWidget, SIGNAL(currentFontChanged(QFont)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontColorButton, SIGNAL(changed(QColor)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->useBackgroundCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->backgroundColorButton, SIGNAL(changed(QColor)),
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

    settings.insert("Use image",          true);
    settings.insert("Watermark image",    QString());
    settings.insert("Text",               QString());
    settings.insert("Font",               QFont());
    settings.insert("Color",              Qt::black);
    settings.insert("Text opacity",       100);
    settings.insert("Use background",     true);
    settings.insert("Background color",   QColor(0xCC, 0xCC, 0xCC));
    settings.insert("Background opacity", 0xCC);
    settings.insert("Placement",          Private::BottomRight);
    settings.insert("Watermark size",     25);
    settings.insert("X margin",           2);
    settings.insert("Y margin",           2);

    return settings;
}

void WaterMark::slotAssignSettings2Widget()
{
    d->changeSettings = false;
    d->useImageRadioButton->setChecked(settings()["Use image"].toBool());
    d->useTextRadioButton->setChecked(!settings()["Use image"].toBool());
    d->imageFileUrlRequester->setText(settings()["Watermark image"].toString());
    d->textEdit->setText(settings()["Text"].toString());
    d->fontChooserWidget->setFont(settings()["Font"].toString());
    d->fontColorButton->setColor(settings()["Color"].toString());
    d->textOpacity->setValue(settings()["Text opacity"].toInt());
    d->useBackgroundCheckBox->setChecked(settings()["Use background"].toBool());
    d->backgroundColorButton->setColor(settings()["Background color"].toString());
    d->backgroundOpacity->setValue(settings()["Background opacity"].toInt());
    d->comboBox->setCurrentIndex(settings()["Placement"].toInt());
    d->waterMarkSizePercent->setValue(settings()["Watermark size"].toInt());
    d->xMarginInput->setValue(settings()["X margin"].toInt());
    d->yMarginInput->setValue(settings()["Y margin"].toInt());
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
        settings.insert("Use image",          d->useImageRadioButton->isChecked());
        settings.insert("Watermark image",    d->imageFileUrlRequester->text());
        settings.insert("Text",               d->textEdit->text());
        settings.insert("Font",               d->fontChooserWidget->currentFont());
        settings.insert("Color",              d->fontColorButton->color());
        settings.insert("Text opacity",       d->textOpacity->value());
        settings.insert("Use background",     d->useBackgroundCheckBox->isChecked());
        settings.insert("Background color",   d->backgroundColorButton->color());
        settings.insert("Background opacity", d->backgroundOpacity->value());
        settings.insert("Placement",          (int)d->comboBox->currentIndex());
        settings.insert("Watermark size",     (int)d->waterMarkSizePercent->value());
        settings.insert("X margin",           (int)d->xMarginInput->value());
        settings.insert("Y margin",           (int)d->yMarginInput->value());
        BatchTool::slotSettingsChanged(settings);
    }

}

bool WaterMark::toolOperations()
{

    if (!loadToDImg())
    {
        return false;
    }

    QString fileName        = settings()["Watermark image"].toString();
    int placement           = settings()["Placement"].toInt();
    int size                = settings()["Watermark size"].toInt();
    int xMargin             = settings()["X margin"].toInt();
    int yMargin             = settings()["Y margin"].toInt();
    bool useImage           = settings()["Use image"].toBool();

    QString text            = settings()["Text"].toString();
    QFont font              = settings()["Font"].toString();
    QColor fontColor        = settings()["Color"].toString();
    int textOpacity         = settings()["Text opacity"].toInt();
    bool useBackground      = settings()["Use background"].toBool();
    QColor backgroundColor  = settings()["Background color"].toString();
    int backgroundOpacity   = settings()["Background opacity"].toInt();


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
