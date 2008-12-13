/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <qbrush.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kcursor.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <ktextedit.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "fontchooserwidget.h"
#include "imageiface.h"
#include "inserttextwidget.h"
#include "inserttexttool.h"
#include "inserttexttool.moc"

using namespace Digikam;

namespace DigikamInsertTextImagesPlugin
{

InsertTextTool::InsertTextTool(QObject* parent)
              : EditorTool(parent)
{
    setName("inserttext");
    setToolName(i18n("Insert Text"));
    setToolIcon(SmallIcon("inserttext"));

    // -------------------------------------------------------------

    QFrame *frame   = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new InsertTextWidget(480, 320, frame);
    l->addWidget(m_previewWidget);
    QWhatsThis::add(m_previewWidget, i18n("<p>This previews the text inserted in the image. "
                                          "You can use the mouse to move the text to the right location."));
    setToolView(frame);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);
    QGridLayout *grid = new QGridLayout(m_gboxSettings->plainPage(), 9, 1);

    m_textEdit = new KTextEdit(m_gboxSettings->plainPage());
    m_textEdit->setCheckSpellingEnabled(true);
    m_textEdit->setWordWrap(QTextEdit::NoWrap);
    QWhatsThis::add(m_textEdit, i18n("<p>Here, enter the text you want to insert in your image."));

    // -------------------------------------------------------------

    m_fontChooserWidget = new FontChooserWidget(m_gboxSettings->plainPage());
    QWhatsThis::add( m_textEdit, i18n("<p>Here you can choose the font to be used."));

    // -------------------------------------------------------------

    KIconLoader icon;
    m_alignButtonGroup = new QHButtonGroup(m_gboxSettings->plainPage());

    QPushButton *alignLeft = new QPushButton(m_alignButtonGroup);
    m_alignButtonGroup->insert(alignLeft, ALIGN_LEFT);
    alignLeft->setPixmap(icon.loadIcon("text_left", (KIcon::Group) KIcon::Small));
    alignLeft->setToggleButton(true);
    QToolTip::add(alignLeft, i18n("Align text to the left"));

    QPushButton *alignRight = new QPushButton(m_alignButtonGroup);
    m_alignButtonGroup->insert(alignRight, ALIGN_RIGHT);
    alignRight->setPixmap(icon.loadIcon("text_right", (KIcon::Group) KIcon::Small));
    alignRight->setToggleButton(true);
    QToolTip::add(alignRight, i18n("Align text to the right"));

    QPushButton *alignCenter = new QPushButton(m_alignButtonGroup);
    m_alignButtonGroup->insert(alignCenter, ALIGN_CENTER);
    alignCenter->setPixmap(icon.loadIcon("text_center", (KIcon::Group) KIcon::Small));
    alignCenter->setToggleButton(true);
    QToolTip::add(alignCenter, i18n("Align text to center"));

    QPushButton *alignBlock = new QPushButton(m_alignButtonGroup);
    m_alignButtonGroup->insert(alignBlock, ALIGN_BLOCK);
    alignBlock->setPixmap(icon.loadIcon("text_block", (KIcon::Group) KIcon::Small));
    alignBlock->setToggleButton(true);
    QToolTip::add(alignBlock, i18n("Align text to a block"));

    m_alignButtonGroup->setExclusive(true);
    m_alignButtonGroup->setFrameShape(QFrame::NoFrame);

    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Rotation:"), m_gboxSettings->plainPage());
    m_textRotation = new QComboBox(false, m_gboxSettings->plainPage());
    m_textRotation->insertItem(i18n("None"));
    m_textRotation->insertItem(i18n("90 Degrees"));
    m_textRotation->insertItem(i18n("180 Degrees"));
    m_textRotation->insertItem(i18n("270 Degrees"));
    QWhatsThis::add( m_textRotation, i18n("<p>Select the text rotation to use."));

    // -------------------------------------------------------------

    QLabel *label2    = new QLabel(i18n("Color:"), m_gboxSettings->plainPage());
    m_fontColorButton = new KColorButton( Qt::black, m_gboxSettings->plainPage() );
    QWhatsThis::add( m_fontColorButton, i18n("<p>Select the font color to use."));

    // -------------------------------------------------------------

    m_borderText = new QCheckBox(i18n("Add border"), m_gboxSettings->plainPage());
    QToolTip::add(m_borderText, i18n("Add a solid border around text using current text color"));

    m_transparentText = new QCheckBox(i18n("Semi-transparent"), m_gboxSettings->plainPage());
    QToolTip::add(m_transparentText, i18n("Use semi-transparent text background under image"));

    grid->addMultiCellWidget(m_textEdit,          0, 2, 0, 1);
    grid->addMultiCellWidget(m_fontChooserWidget, 3, 3, 0, 1);
    grid->addMultiCellWidget(m_alignButtonGroup,  4, 4, 0, 1);
    grid->addMultiCellWidget(label1,              5, 5, 0, 0);
    grid->addMultiCellWidget(m_textRotation,      5, 5, 1, 1);
    grid->addMultiCellWidget(label2,              6, 6, 0, 0);
    grid->addMultiCellWidget(m_fontColorButton,   6, 6, 1, 1);
    grid->addMultiCellWidget(m_borderText,        7, 7, 0, 1);
    grid->addMultiCellWidget(m_transparentText,   8, 8, 0, 1);
    grid->setMargin(0);
    grid->setSpacing(m_gboxSettings->spacingHint());
    grid->setRowStretch(9, 10);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_fontChooserWidget, SIGNAL(fontSelected (const QFont&)),
            this, SLOT(slotFontPropertiesChanged(const QFont&)));

    connect(m_fontColorButton, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdatePreview()));

    connect(m_textEdit, SIGNAL(textChanged()),
            this, SLOT(slotUpdatePreview()));

    connect(m_alignButtonGroup, SIGNAL(released(int)),
            this, SLOT(slotAlignModeChanged(int)));

    connect(m_borderText, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdatePreview()));

    connect(m_transparentText, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdatePreview()));

    connect(m_textRotation, SIGNAL(activated(int)),
            this, SLOT(slotUpdatePreview()));

    connect(this, SIGNAL(signalUpdatePreview()),
            this, SLOT(slotUpdatePreview()));

    // -------------------------------------------------------------

    slotUpdatePreview();
}

InsertTextTool::~InsertTextTool()
{
}

void InsertTextTool::readSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("inserttext Tool");
    QColor black(0, 0, 0);
    QFont  defaultFont;

    int orgW = m_previewWidget->imageIface()->originalWidth();
    int orgH = m_previewWidget->imageIface()->originalHeight();

    if (orgW > orgH) m_defaultSizeFont = (int)(orgH / 8.0);
    else m_defaultSizeFont = (int)(orgW / 8.0);

    defaultFont.setPointSize(m_defaultSizeFont);
    m_textRotation->setCurrentItem(config->readNumEntry("Text Rotation", 0));
    m_fontColorButton->setColor(config->readColorEntry("Font Color", &black));
    m_textEdit->setText(config->readEntry("Text String", i18n("Enter your text here!")));
    m_textFont = config->readFontEntry("Font Properties", &defaultFont);
    m_fontChooserWidget->setFont(m_textFont);
    m_alignTextMode = config->readNumEntry("Text Alignment", ALIGN_LEFT);
    m_borderText->setChecked(config->readBoolEntry("Border Text", false));
    m_transparentText->setChecked(config->readBoolEntry("Transparent Text", false));
    m_previewWidget->setPositionHint(config->readRectEntry("Position Hint"));

    static_cast<QPushButton*>(m_alignButtonGroup->find(m_alignTextMode))->setOn(true);
    slotAlignModeChanged(m_alignTextMode);
}

void InsertTextTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("inserttext Tool");

    config->writeEntry("Text Rotation", m_textRotation->currentItem());
    config->writeEntry("Font Color", m_fontColorButton->color());
    config->writeEntry("Text String", m_textEdit->text());
    config->writeEntry("Font Properties", m_textFont);
    config->writeEntry("Text Alignment", m_alignTextMode);
    config->writeEntry("Border Text", m_borderText->isChecked());
    config->writeEntry("Transparent Text", m_transparentText->isChecked());
    config->writeEntry("Position Hint", m_previewWidget->getPositionHint());

    config->sync();
}

void InsertTextTool::slotResetSettings()
{
    m_fontColorButton->blockSignals(true);
    m_alignButtonGroup->blockSignals(true);
    m_fontChooserWidget->blockSignals(true);

    m_textRotation->setCurrentItem(0); // No rotation.
    m_fontColorButton->setColor(Qt::black);
    QFont defaultFont;
    m_textFont = defaultFont; // Reset to default KDE font.
    m_textFont.setPointSize(m_defaultSizeFont);
    m_fontChooserWidget->setFont(m_textFont);
    m_borderText->setChecked(false);
    m_transparentText->setChecked(false);
    m_previewWidget->resetEdit();
    static_cast<QPushButton*> (m_alignButtonGroup->find(ALIGN_LEFT))->setOn(true);

    m_fontChooserWidget->blockSignals(false);
    m_fontColorButton->blockSignals(false);
    m_alignButtonGroup->blockSignals(false);
    slotAlignModeChanged(ALIGN_LEFT);
}

void InsertTextTool::slotAlignModeChanged(int mode)
{
    m_alignTextMode = mode;
    m_textEdit->selectAll(true);

    switch (m_alignTextMode)
        {
        case ALIGN_LEFT:
           m_textEdit->setAlignment( Qt::AlignLeft );
           break;

        case ALIGN_RIGHT:
           m_textEdit->setAlignment( Qt::AlignRight );
           break;

        case ALIGN_CENTER:
           m_textEdit->setAlignment( Qt::AlignHCenter );
           break;

        case ALIGN_BLOCK:
           m_textEdit->setAlignment( Qt::AlignJustify );
           break;
        }

    m_textEdit->selectAll(false);
    emit signalUpdatePreview();
}

void InsertTextTool::slotFontPropertiesChanged(const QFont& font)
{
    m_textFont = font;
    emit signalUpdatePreview();
}

void InsertTextTool::slotUpdatePreview()
{
    m_previewWidget->setText(m_textEdit->text(), m_textFont, m_fontColorButton->color(), m_alignTextMode,
                             m_borderText->isChecked(), m_transparentText->isChecked(),
                             m_textRotation->currentItem());
}

void InsertTextTool::finalRendering()
{
    kapp->setOverrideCursor(KCursor::waitCursor());

    ImageIface iface(0, 0);
    DImg dest = m_previewWidget->makeInsertText();
    iface.putOriginalImage(i18n("Insert Text"), dest.bits(), dest.width(), dest.height());

    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamInsertTextImagesPlugin
