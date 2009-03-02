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

// C++ includes.

#include <cmath>

// Qt includes.

#include <QWidget>
#include <QLabel>
#include <QFontMetrics>
#include <QPoint>
#include <QRect>
#include <QPen>
#include <QPainter>

// KDE includes.

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

// Local includes.

#include "dimg.h"
#include "dimgimagefilters.h"

namespace Digikam
{

WaterMark::WaterMark(QObject* parent)
         : BatchTool("WaterMark", BaseTool, parent)
{
    setToolTitle(i18n("Add Watermark"));
    setToolDescription(i18n("A tool to add a visible watermark"));
    setToolIcon(KIcon(SmallIcon("insert-text")));

    KVBox *vbox = new KVBox;
    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(0);

    QLabel *label  = new QLabel(vbox);
    m_textEdit     = new KLineEdit(vbox);
    m_textEdit->setClearButtonShown(true);
    m_textEdit->setClickMessage(i18n("Enter here your watermark string."));
    label->setText(i18n("Text:"));

    QLabel *label2      = new QLabel(vbox);
    m_fontChooserWidget = new KFontComboBox(vbox);
    m_fontChooserWidget->setWhatsThis( i18n("Here you can choose the font to be used."));
    label2->setText(i18n("Font:"));

    QLabel *label3    = new QLabel(vbox);
    m_fontColorButton = new KColorButton(Qt::black, vbox);
    m_fontColorButton->setWhatsThis(i18n("Set here the font color to use."));
    label3->setText(i18n("Font Color:"));

    QLabel *label4 = new QLabel(vbox);
    m_comboBox     = new KComboBox(vbox);
    m_comboBox->insertItem(TopLeft,     i18n("Top left"));
    m_comboBox->insertItem(TopRight,    i18n("Top right"));
    m_comboBox->insertItem(BottomLeft,  i18n("Bottom left"));
    m_comboBox->insertItem(BottomRight, i18n("Bottom right"));
    label4->setText(i18n("Corner:"));

    QLabel *label5 = new QLabel(vbox);
    m_stringLength = new KIntNumInput(vbox);
    m_stringLength->setRange(10, 90);
    m_stringLength->setValue(25);
    m_stringLength->setSliderEnabled(true);
    m_stringLength->setWhatsThis(i18n("Set here the string lenght in percent relative of image width."));
    label5->setText(i18n("Lenght (%):"));

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(m_fontChooserWidget, SIGNAL(currentFontChanged(const QFont&)),
            this, SLOT(slotSettingsChanged()));

    connect(m_fontColorButton, SIGNAL(changed(const QColor&)),
            this, SLOT(slotSettingsChanged()));

    connect(m_textEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotSettingsChanged()));

    connect(m_stringLength, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));
}

WaterMark::~WaterMark()
{
}

BatchToolSettings WaterMark::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("Text",   QString());
    settings.insert("Font",   QFont());
    settings.insert("Color",  Qt::black);
    settings.insert("Corner", BottomRight);
    settings.insert("Lenght", 25);
    return settings;
}

void WaterMark::assignSettings2Widget()
{
    m_textEdit->setText(settings()["Text"].toString());
    m_fontChooserWidget->setFont(settings()["Font"].toString());
    m_fontColorButton->setColor(settings()["Color"].toString());
    m_comboBox->setCurrentIndex(settings()["Corner"].toInt());
    m_stringLength->setValue(settings()["Lenght"].toInt());
}

void WaterMark::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Text",   m_textEdit->text());
    settings.insert("Font",   m_fontChooserWidget->font());
    settings.insert("Color",  m_fontColorButton->color());
    settings.insert("Corner", (int)m_comboBox->currentIndex());
    settings.insert("Lenght", (int)m_stringLength->value());
    setSettings(settings);
}

bool WaterMark::toolOperations()
{
    if (!loadToDImg()) return false;

    const int radius = 10;
    const int margin = 50;
    QString text     = settings()["Text"].toString();
    QFont font       = settings()["Font"].toString();
    QColor color     = settings()["Color"].toString();
    int corner       = settings()["Corner"].toInt();
    int lenght       = settings()["Lenght"].toInt();
    int alignMode;

    int size = queryFontSize(text, font, lenght);
    if (size == 0) return false;

    font.setPointSizeF(size);
    QFontMetrics fontMt(font);
    QRect fontRect = fontMt.boundingRect(0, 0, image().width(), image().height(), 0, text);

    switch(corner)
    {
        case TopLeft:
            fontRect.moveTopLeft(QPoint(margin, margin));
            alignMode = Qt::AlignLeft;
            break;
        case TopRight:
            fontRect.moveTopRight(QPoint(image().width()-margin, margin));
            alignMode = Qt::AlignRight;
            break;
        case BottomLeft:
            fontRect.moveBottomLeft(QPoint(margin, image().height()-margin));
            alignMode = Qt::AlignLeft;
            break;
        default :    // BottomRight
            fontRect.moveBottomRight(QPoint(image().width()-margin, image().height()-margin));
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

int WaterMark::queryFontSize(const QString& text, const QFont& font, int lenght)
{
    // Find font size using relative lenght compared to image width.
    QFont fnt = font;
    QRect fontRect;
    for (int i = 1 ; i <= 1000 ; i++)
    {
        fnt.setPointSizeF(i);
        QFontMetrics fontMt(fnt);
        fontRect = fontMt.boundingRect(0, 0, image().width(), image().height(), 0, text);
        if (fontRect.width() > lround((image().width() * lenght)/100.0))
            return (i-1);
    }
    return 0;
}

}  // namespace Digikam
