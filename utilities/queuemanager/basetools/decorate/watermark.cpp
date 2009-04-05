/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
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

#include "watermark.h"
#include "watermark.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QWidget>
#include <QLabel>
#include <QFontMetrics>
#include <QPoint>
#include <QRect>
#include <QPen>
#include <QPainter>

// KDE includes

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kfontcombobox.h>
#include <kcolorbutton.h>
#include <klineedit.h>
#include <kdialog.h>
#include <knuminput.h>

// Local includes

#include "dimg.h"
#include "dimgimagefilters.h"

namespace Digikam
{

class WaterMarkPriv
{

public:

    enum WaterMarkPositon
    {
        TopLeft=0,
        TopRight,
        BottomLeft,
        BottomRight
    };

public:

    WaterMarkPriv()
    {
        textEdit          = 0;
        stringLength      = 0;
        fontChooserWidget = 0;
        fontColorButton   = 0;
        comboBox          = 0;
    }

    KLineEdit     *textEdit;

    KIntNumInput  *stringLength;

    KFontComboBox *fontChooserWidget;

    KColorButton  *fontColorButton;

    KComboBox     *comboBox;
};

WaterMark::WaterMark(QObject* parent)
         : BatchTool("WaterMark", BaseTool, parent), d(new WaterMarkPriv)
{
    setToolTitle(i18n("Add Watermark"));
    setToolDescription(i18n("A tool to add a visible watermark"));
    setToolIcon(KIcon(SmallIcon("insert-text")));

    KVBox *vbox = new KVBox;
    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(0);

    QLabel *label = new QLabel(vbox);
    d->textEdit   = new KLineEdit(vbox);
    d->textEdit->setClearButtonShown(true);
    d->textEdit->setClickMessage(i18n("Enter here your watermark string."));
    label->setText(i18n("Text:"));

    QLabel *label2       = new QLabel(vbox);
    d->fontChooserWidget = new KFontComboBox(vbox);
    d->fontChooserWidget->setWhatsThis( i18n("Here you can choose the font to be used."));
    label2->setText(i18n("Font:"));

    QLabel *label3     = new QLabel(vbox);
    d->fontColorButton = new KColorButton(Qt::black, vbox);
    d->fontColorButton->setWhatsThis(i18n("Set here the font color to use."));
    label3->setText(i18n("Font Color:"));

    QLabel *label4 = new QLabel(vbox);
    d->comboBox    = new KComboBox(vbox);
    d->comboBox->insertItem(WaterMarkPriv::TopLeft,     i18n("Top left"));
    d->comboBox->insertItem(WaterMarkPriv::TopRight,    i18n("Top right"));
    d->comboBox->insertItem(WaterMarkPriv::BottomLeft,  i18n("Bottom left"));
    d->comboBox->insertItem(WaterMarkPriv::BottomRight, i18n("Bottom right"));
    label4->setText(i18n("Corner:"));

    QLabel *label5  = new QLabel(vbox);
    d->stringLength = new KIntNumInput(vbox);
    d->stringLength->setRange(10, 90);
    d->stringLength->setValue(25);
    d->stringLength->setSliderEnabled(true);
    d->stringLength->setWhatsThis(i18n("Enter here the string length as a percent of the image width."));
    label5->setText(i18n("length (%):"));

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(d->comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontChooserWidget, SIGNAL(currentFontChanged(const QFont&)),
            this, SLOT(slotSettingsChanged()));

    connect(d->fontColorButton, SIGNAL(changed(const QColor&)),
            this, SLOT(slotSettingsChanged()));

    connect(d->textEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotSettingsChanged()));

    connect(d->stringLength, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));
}

WaterMark::~WaterMark()
{
    delete d;
}

BatchToolSettings WaterMark::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("Text",   QString());
    settings.insert("Font",   QFont());
    settings.insert("Color",  Qt::black);
    settings.insert("Corner", WaterMarkPriv::BottomRight);
    settings.insert("Length", 25);
    return settings;
}

void WaterMark::assignSettings2Widget()
{
    d->textEdit->setText(settings()["Text"].toString());
    d->fontChooserWidget->setFont(settings()["Font"].toString());
    d->fontColorButton->setColor(settings()["Color"].toString());
    d->comboBox->setCurrentIndex(settings()["Corner"].toInt());
    d->stringLength->setValue(settings()["Length"].toInt());
}

void WaterMark::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Text",   d->textEdit->text());
    settings.insert("Font",   d->fontChooserWidget->currentFont());
    settings.insert("Color",  d->fontColorButton->color());
    settings.insert("Corner", (int)d->comboBox->currentIndex());
    settings.insert("Length", (int)d->stringLength->value());
    setSettings(settings);
}

bool WaterMark::toolOperations()
{
    if (!loadToDImg()) return false;

    const int radius = 10;
    const int margin = 5;   // Relative of image dimenssions (percents)
    QString text     = settings()["Text"].toString();
    QFont font       = settings()["Font"].toString();
    QColor color     = settings()["Color"].toString();
    int corner       = settings()["Corner"].toInt();
    int length       = settings()["Length"].toInt();
    int alignMode;

    if (text.isEmpty()) return false;

    int size = queryFontSize(text, font, length);
    if (size == 0) return false;

    font.setPointSizeF(size);
    QFontMetrics fontMt(font);
    QRect fontRect = fontMt.boundingRect(0, 0, image().width(), image().height(), 0, text);
    int mrgW       = lround(image().width()  * (margin / 100.0));
    int mrgH       = lround(image().height() * (margin / 100.0));

    switch(corner)
    {
        case WaterMarkPriv::TopLeft:
            fontRect.moveTopLeft(QPoint(mrgW, mrgH));
            alignMode = Qt::AlignLeft;
            break;
        case WaterMarkPriv::TopRight:
            fontRect.moveTopRight(QPoint(image().width()-mrgW, mrgH));
            alignMode = Qt::AlignRight;
            break;
        case WaterMarkPriv::BottomLeft:
            fontRect.moveBottomLeft(QPoint(mrgW, image().height()-mrgH));
            alignMode = Qt::AlignLeft;
            break;
        default :    // BottomRight
            fontRect.moveBottomRight(QPoint(image().width()-mrgW, image().height()-mrgH));
            alignMode = Qt::AlignRight;
            break;
    }

    DColorComposer *composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);

    // Add a transparent layer.
    QRect backgroundRect(fontRect.x()-radius, fontRect.y()-radius,
                         fontRect.width()+2*radius, fontRect.height()+2*radius);
    DImg backgroundLayer(backgroundRect.width(), backgroundRect.height(), image().sixteenBit(), true);
    DColor transparent(QColor(0, 0, 0));
    transparent.setAlpha(0);
    if (image().sixteenBit()) transparent.convertToSixteenBit();
    backgroundLayer.fill(transparent);

    DImg grayTransLayer(fontRect.width(), fontRect.height(), image().sixteenBit(), true);
    DColor grayTrans(QColor(0xCC, 0xCC, 0xCC));
    grayTrans.setAlpha(0xCC);
    if (image().sixteenBit()) grayTrans.convertToSixteenBit();
    grayTransLayer.fill(grayTrans);

    backgroundLayer.bitBlendImage(composer, &grayTransLayer, 0, 0,
                                  grayTransLayer.width(), grayTransLayer.height(),
                                  radius, radius);

    DImgImageFilters filters;
    filters.gaussianBlurImage(backgroundLayer.bits(), backgroundLayer.width(),
                              backgroundLayer.height(), backgroundLayer.sixteenBit(), radius);

    image().bitBlendImage(composer, &backgroundLayer, 0, 0,
                          backgroundLayer.width(), backgroundLayer.height(),
                          backgroundRect.x(), backgroundRect.y());

    // Draw text
    QImage img = image().copyQImage(fontRect);
    QPainter p(&img);
    p.setPen(QPen(color, 1));
    p.setFont(font);
    p.save();
    p.drawText(0, 0, fontRect.width(), fontRect.height(), alignMode, text);
    p.restore();
    p.end();

    DImg textDrawn(img);

    // convert to 16 bit if needed
    textDrawn.convertToDepthOfImage(&image());

    // now compose to original: only pixels affected by drawing text and border are changed, not whole area
    image().bitBlendImage(composer, &textDrawn, 0, 0, textDrawn.width(), textDrawn.height(),
                          fontRect.x(), fontRect.y());

    delete composer;

    return (savefromDImg());
}

int WaterMark::queryFontSize(const QString& text, const QFont& font, int length)
{
    // Find font size using relative length compared to image width.
    QFont fnt = font;
    QRect fontRect;
    for (int i = 1 ; i <= 1000 ; i++)
    {
        fnt.setPointSizeF(i);
        QFontMetrics fontMt(fnt);
        fontRect = fontMt.boundingRect(0, 0, image().width(), image().height(), 0, text);
        if (fontRect.width() > lround((image().width() * length)/100.0))
            return (i-1);
    }
    return 0;
}

}  // namespace Digikam
