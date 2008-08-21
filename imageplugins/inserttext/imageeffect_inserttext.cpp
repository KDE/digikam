/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a plugin to insert a text over an image.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QBrush>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfontchooser.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <ktextedit.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "inserttextwidget.h"
#include "imageeffect_inserttext.h"
#include "imageeffect_inserttext.moc"

namespace DigikamInsertTextImagesPlugin
{

ImageEffect_InsertText::ImageEffect_InsertText(QWidget* parent)
                      : Digikam::ImageDlgBase(parent, i18n("Insert Text on Photograph"),
                                              "inserttext", false, false)
{
    QString whatsThis;

    // About data and help button.

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Insert Text"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A digiKam image plugin for inserting text on a photograph."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2006, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QFrame *frame   = new QFrame(mainWidget());
    QVBoxLayout* l  = new QVBoxLayout(frame);
    m_previewWidget = new InsertTextWidget(480, 320, frame);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    l->addWidget(m_previewWidget);
    l->setMargin(5);
    l->setSpacing(0);
    m_previewWidget->setWhatsThis( i18n("<p>This previews the text inserted in the image. "
                                           "You can use the mouse to move the text to the right location."));
    setPreviewAreaWidget(frame);

    // -------------------------------------------------------------

    QWidget *gbox2        = new QWidget(mainWidget());
    QGridLayout *gridBox2 = new QGridLayout( gbox2 );

    m_textEdit = new KTextEdit(gbox2);
    m_textEdit->setCheckSpellingEnabled(true);
    m_textEdit->setWordWrapMode(QTextOption::NoWrap);
    m_textEdit->setWhatsThis( i18n("<p>Here, enter the text you want to insert in your image."));

    // -------------------------------------------------------------

    m_fontChooserWidget = new KFontChooser(gbox2, KFontChooser::NoDisplayFlags);
    m_fontChooserWidget->setSampleBoxVisible(false);
    m_fontChooserWidget->setWhatsThis( i18n("<p>Here you can choose the font to be used."));

    // -------------------------------------------------------------

    KIconLoader icon;
    QWidget *alignBox  = new QWidget(gbox2);
    QHBoxLayout *hlay  = new QHBoxLayout(alignBox);
    m_alignButtonGroup = new QButtonGroup(alignBox);
    m_alignButtonGroup->setExclusive(true);

    QToolButton *alignLeft = new QToolButton( alignBox );
    m_alignButtonGroup->addButton(alignLeft, ALIGN_LEFT);
    alignLeft->setIcon(SmallIcon("format-justify-left"));
    alignLeft->setCheckable(true);
    alignLeft->setToolTip( i18n( "Align text to the left" ) );

    QToolButton *alignRight = new QToolButton( alignBox );
    m_alignButtonGroup->addButton(alignRight, ALIGN_RIGHT);
    alignRight->setIcon(SmallIcon("format-justify-right"));
    alignRight->setCheckable(true);
    alignRight->setToolTip( i18n( "Align text to the right" ) );

    QToolButton *alignCenter = new QToolButton( alignBox );
    m_alignButtonGroup->addButton(alignCenter, ALIGN_CENTER);
    alignCenter->setIcon(SmallIcon("format-justify-center"));
    alignCenter->setCheckable(true);
    alignCenter->setToolTip( i18n( "Align text to center" ) );

    QToolButton *alignBlock = new QToolButton( alignBox );
    m_alignButtonGroup->addButton(alignBlock, ALIGN_BLOCK);
    alignBlock->setIcon(SmallIcon("format-justify-fill"));
    alignBlock->setCheckable(true);
    alignBlock->setToolTip( i18n( "Align text to a block" ) );

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(alignLeft);
    hlay->addWidget(alignRight);
    hlay->addWidget(alignCenter);
    hlay->addWidget(alignBlock);

    // -------------------------------------------------------------

    QLabel *label1 = new QLabel(i18n("Rotation:"), gbox2);
    m_textRotation = new QComboBox( gbox2 );
    m_textRotation->addItem( i18n("None") );
    m_textRotation->addItem( i18n("90 Degrees") );
    m_textRotation->addItem( i18n("180 Degrees") );
    m_textRotation->addItem( i18n("270 Degrees") );
    m_textRotation->setWhatsThis( i18n("<p>Select the text rotation to use here."));

    // -------------------------------------------------------------

    QLabel *label2    = new QLabel(i18n("Color:"), gbox2);
    m_fontColorButton = new KColorButton( Qt::black, gbox2 );
    m_fontColorButton->setWhatsThis( i18n("<p>Set here the font color to use."));

    // -------------------------------------------------------------

    m_borderText = new QCheckBox( i18n( "Add border"), gbox2 );
    m_borderText->setToolTip( i18n( "Add a solid border around text using current text color" ) );

    m_transparentText = new QCheckBox( i18n( "Semi-transparent"), gbox2 );
    m_transparentText->setToolTip( i18n( "Use semi-transparent text background under image" ) );

    // -------------------------------------------------------------

    gridBox2->addWidget(m_textEdit, 0, 0, 3, 2 );
    gridBox2->addWidget(m_fontChooserWidget, 3, 0, 1, 2 );
    gridBox2->addWidget(alignBox, 4, 0, 1, 2 );
    gridBox2->addWidget(label1, 5, 0, 1, 1);
    gridBox2->addWidget(m_textRotation, 5, 1, 1, 1);
    gridBox2->addWidget(label2, 6, 0, 1, 1);
    gridBox2->addWidget(m_fontColorButton, 6, 1, 1, 1);
    gridBox2->addWidget(m_borderText, 7, 0, 1, 2 );
    gridBox2->addWidget(m_transparentText, 8, 0, 1, 2 );
    gridBox2->setRowStretch(9, 10);
    gridBox2->setMargin(spacingHint());
    gridBox2->setSpacing(spacingHint());

    setUserAreaWidget(gbox2);

    // -------------------------------------------------------------

    connect(m_fontChooserWidget, SIGNAL(fontSelected(const QFont&)),
            this, SLOT(slotFontPropertiesChanged(const QFont&)));

    connect(m_fontColorButton, SIGNAL(changed(const QColor&)),
            this, SLOT(slotUpdatePreview()));

    connect(m_textEdit, SIGNAL(textChanged()),
            this, SLOT(slotUpdatePreview()));

    connect(m_alignButtonGroup, SIGNAL(buttonReleased(int)),
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

ImageEffect_InsertText::~ImageEffect_InsertText()
{
}

void ImageEffect_InsertText::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group =  config->group("inserttext Tool Dialog");
    QColor black(0, 0, 0);
    QFont  defaultFont;

    int orgW = m_previewWidget->imageIface()->originalWidth();
    int orgH = m_previewWidget->imageIface()->originalHeight();

    if ( orgW > orgH ) m_defaultSizeFont = (int)(orgH / 8.0);
    else m_defaultSizeFont = (int)(orgW / 8.0);

    defaultFont.setPointSize(m_defaultSizeFont);
    m_textRotation->setCurrentIndex( group.readEntry("Text Rotation", 0) );
    m_fontColorButton->setColor(group.readEntry("Font Color", black));
    m_textEdit->setText(group.readEntry("Text String", i18n("Enter your text here!")));
    m_textFont = group.readEntry("Font Properties", defaultFont);
    m_fontChooserWidget->setFont(m_textFont);
    m_alignTextMode = group.readEntry("Text Alignment", (int)ALIGN_LEFT);
    m_borderText->setChecked( group.readEntry("Border Text", false) );
    m_transparentText->setChecked( group.readEntry("Transparent Text", false) );
    m_previewWidget->setPositionHint( group.readEntry("Position Hint",QRect()) );

    m_alignButtonGroup->button(m_alignTextMode)->setChecked(true);
    slotAlignModeChanged(m_alignTextMode);
}

void ImageEffect_InsertText::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("inserttext Tool Dialog");

    group.writeEntry( "Text Rotation", m_textRotation->currentIndex() );
    group.writeEntry( "Font Color", m_fontColorButton->color() );
    group.writeEntry( "Text String", m_textEdit->document()->toPlainText() );
    group.writeEntry( "Font Properties", m_textFont );
    group.writeEntry( "Text Alignment", m_alignTextMode );
    group.writeEntry( "Border Text", m_borderText->isChecked() );
    group.writeEntry( "Transparent Text", m_transparentText->isChecked() );
    group.writeEntry( "Position Hint", m_previewWidget->getPositionHint() );

    config->sync();
}

void ImageEffect_InsertText::resetValues()
{
    m_fontColorButton->blockSignals(true);
    m_alignButtonGroup->blockSignals(true);
    m_fontChooserWidget->blockSignals(true);

    m_textRotation->setCurrentIndex(0);    // No rotation.
    m_fontColorButton->setColor(Qt::black);
    QFont defaultFont;
    m_textFont = defaultFont; // Reset to default KDE font.
    m_textFont.setPointSize( m_defaultSizeFont );
    m_fontChooserWidget->setFont(m_textFont);
    m_borderText->setChecked( false );
    m_transparentText->setChecked( false );
    m_previewWidget->resetEdit();
    m_alignButtonGroup->button(ALIGN_LEFT)->setChecked(true);

    m_fontChooserWidget->blockSignals(false);
    m_fontColorButton->blockSignals(false);
    m_alignButtonGroup->blockSignals(false);
    slotAlignModeChanged(ALIGN_LEFT);
}

void ImageEffect_InsertText::slotAlignModeChanged(int mode)
{
    m_alignTextMode = mode;
    m_textEdit->selectAll();

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

    m_textEdit->textCursor().clearSelection();
    emit signalUpdatePreview();
}

void ImageEffect_InsertText::slotFontPropertiesChanged(const QFont &font)
{
    m_textFont = font;
    emit signalUpdatePreview();
}

void ImageEffect_InsertText::slotUpdatePreview()
{
    m_previewWidget->setText(m_textEdit->document()->toPlainText(), m_textFont, m_fontColorButton->color(), m_alignTextMode,
                             m_borderText->isChecked(), m_transparentText->isChecked(),
                             m_textRotation->currentIndex());
}

void ImageEffect_InsertText::finalRendering()
{
    accept();
    kapp->setOverrideCursor( Qt::WaitCursor );

    Digikam::ImageIface iface(0, 0);
    Digikam::DImg dest = m_previewWidget->makeInsertText();
    iface.putOriginalImage(i18n("Insert Text"), dest.bits(), dest.width(), dest.height());

    kapp->restoreOverrideCursor();
}

}  // NameSpace DigikamInsertTextImagesPlugin
