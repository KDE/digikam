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

// Qt includes.

#include <QWidget>
#include <QLabel>
#include <QFontMetrics>
#include <QFont>
#include <QPoint>
#include <QRect>
#include <QPen>
#include <QPainter>

// KDE includes.

#include <khbox.h>
#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kfontchooser.h>
#include <kcolorbutton.h>
#include <klineedit.h>
#include <kdialog.h>

// Local includes.

#include "dimg.h"

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

    KHBox *hbox    = new KHBox(vbox);
    QLabel *label  = new QLabel(hbox);
    m_textEdit     = new KLineEdit(hbox);
    m_textEdit->setClearButtonShown(true);
    m_textEdit->setWhatsThis(i18n("Here, enter your watermark."));
    label->setText(i18n("Text:"));

    m_fontChooserWidget = new KFontChooser(vbox, KFontChooser::NoDisplayFlags);
    m_fontChooserWidget->setSampleBoxVisible(true);
    m_fontChooserWidget->setWhatsThis( i18n("Here you can choose the font to be used."));

    KHBox *hbox2      = new KHBox(vbox);
    QLabel *label2    = new QLabel(hbox2);
    m_fontColorButton = new KColorButton(Qt::red, hbox2);
    m_fontColorButton->setWhatsThis(i18n("Set here the font color to use."));
    label2->setText(i18n("Font Color:"));

    KHBox *hbox3   = new KHBox(vbox);
    QLabel *label3 = new QLabel(hbox3);
    m_comboBox     = new KComboBox(hbox3);
    m_comboBox->insertItem(TopLeft,     i18n("Top left"));
    m_comboBox->insertItem(TopRight,    i18n("Top right"));
    m_comboBox->insertItem(BottomLeft,  i18n("Bottom left"));
    m_comboBox->insertItem(BottomRight, i18n("Bottom right"));
    label3->setText(i18n("Corner:"));

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(m_fontChooserWidget, SIGNAL(fontSelected(const QFont&)),
            this, SLOT(slotSettingsChanged()));

    connect(m_fontColorButton, SIGNAL(changed(const QColor&)),
            this, SLOT(slotSettingsChanged()));

    connect(m_textEdit, SIGNAL(textChanged()),
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
    settings.insert("Color",  Qt::red);
    settings.insert("Corner", BottomRight);
    return settings;
}

void WaterMark::assignSettings2Widget()
{
    m_textEdit->setText(settings()["Text"].toString());
    m_fontChooserWidget->setFont(settings()["Font"].toString());
    m_fontColorButton->setColor(settings()["Color"].toString());
    m_comboBox->setCurrentIndex(settings()["Corner"].toInt());
}

void WaterMark::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Text",   m_textEdit->text());
    settings.insert("Font",   m_fontChooserWidget->font());
    settings.insert("Color",  m_fontColorButton->color());
    settings.insert("Corner", (int)m_comboBox->currentIndex());
    setSettings(settings);
}

bool WaterMark::toolOperations()
{
    if (!loadToDImg()) return false;

    QString text = settings()["Text"].toString();
    QFont font   = settings()["Font"].toString();
    QColor color = settings()["Color"].toString();
    int corner   = settings()["Corner"].toInt();
    int alignMode;

    font.setPointSizeF(font.pointSizeF());
    QFontMetrics fontMt(font);
    QRect fontRect = fontMt.boundingRect(0, 0, image().width(), image().height(), 0, text);

    switch(corner)
    {
        case TopLeft:
            fontRect.moveTopLeft(QPoint(50, 50));
            alignMode = Qt::AlignLeft;
            break;
        case TopRight:
            fontRect.moveTopRight(QPoint(image().width()-50, 50));
            alignMode = Qt::AlignRight;
            break;
        case BottomLeft:
            fontRect.moveBottomLeft(QPoint(50, image().height()-50));
            alignMode = Qt::AlignLeft;
            break;
        default :    // BottomRight
            fontRect.moveBottomRight(QPoint(image().width()-50, image().height()-50));
            alignMode = Qt::AlignRight;
            break;
    }

    QImage img = image().copyQImage(fontRect);
    QPainter p(&img);
    p.setPen(QPen(color, 1));
    p.setFont(font);
    p.save();
    p.drawText(0, 0, fontRect.width(), fontRect.height(), alignMode, text);
    p.restore();
    p.end();

    DColorComposer *composer = DColorComposer::getComposer(DColorComposer::PorterDuffNone);

    DImg transparentLayer(fontRect.width(), fontRect.height(), image().sixteenBit(), true);
    DColor transparent(QColor(0xCC, 0xCC, 0xCC));
    transparent.setAlpha(210);
    if (image().sixteenBit()) transparent.convertToSixteenBit();
    transparentLayer.fill(transparent);
    image().bitBlendImage(composer, &transparentLayer, 0, 0, fontRect.width(), fontRect.height(),
                          fontRect.x(), fontRect.y());

    DImg textDrawn(img);

    // convert to 16 bit if needed
    textDrawn.convertToDepthOfImage(&image());

    // now compose to original: only pixels affected by drawing text and border are changed, not whole area
    image().bitBlendImage(composer, &textDrawn, 0, 0, textDrawn.width(), textDrawn.height(), 
                          fontRect.x(), fontRect.y());

    delete composer;

    return (savefromDImg());
}

}  // namespace Digikam
