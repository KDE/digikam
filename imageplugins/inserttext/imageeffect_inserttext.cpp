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
 
#include <q3vgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <qlayout.h>
#include <q3frame.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qfont.h>
#include <qtimer.h> 
#include <Q3HButtonGroup> 
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3VBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kcolorbutton.h>
#include <ktextedit.h> 
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "inserttextwidget.h"
#include "fontchooserwidget.h"
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
                                       digikam_version,
                                       ki18n("A digiKam image plugin for inserting text on a photograph."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2006, Gilles Caulier\n"
                                       "(c) 2006-2007, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       "http://www.digikam.org");
                                       
    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    Q3VBoxLayout* l  = new Q3VBoxLayout(frame, 5, 0);
    m_previewWidget = new InsertTextWidget(480, 320, frame);
    l->addWidget(m_previewWidget);
    m_previewWidget->setWhatsThis( i18n("<p>This is the preview of the text inserted to the image. "
                                           "You can use the mouse to move the text to the right location."));
    setPreviewAreaWidget(frame);                                           
    
    // -------------------------------------------------------------

    QWidget *gbox2        = new QWidget(plainPage());
    Q3GridLayout *gridBox2 = new Q3GridLayout( gbox2, 9, 1, spacingHint());
    
    m_textEdit = new KTextEdit(gbox2);
    m_textEdit->setCheckSpellingEnabled(true);
    m_textEdit->setWordWrap(Q3TextEdit::NoWrap);
    m_textEdit->setWhatsThis( i18n("<p>Here, enter the text you want to insert in your image."));
    gridBox2->addMultiCellWidget(m_textEdit, 0, 2, 0, 1);
    
    // -------------------------------------------------------------
    
    m_fontChooserWidget = new FontChooserWidget(gbox2);
    m_textEdit->setWhatsThis( i18n("<p>Here you can choose the font to be used."));
    gridBox2->addMultiCellWidget(m_fontChooserWidget, 3, 3, 0, 1);
    
    // -------------------------------------------------------------
    
    KIconLoader icon;
    m_alignButtonGroup = new Q3HButtonGroup(gbox2);
    
    QPushButton *alignLeft = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignLeft, ALIGN_LEFT);
    alignLeft->setPixmap( icon.loadIcon( "text_left", (KIcon::Group)KIcon::Small ) );
    alignLeft->setToggleButton(true);
    alignLeft->setToolTip( i18n( "Align text to the left" ) );
    
    QPushButton *alignRight = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignRight, ALIGN_RIGHT);
    alignRight->setPixmap( icon.loadIcon( "text_right", (KIcon::Group)KIcon::Small ) );
    alignRight->setToggleButton(true);
    alignRight->setToolTip( i18n( "Align text to the right" ) );
    
    QPushButton *alignCenter = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignCenter, ALIGN_CENTER);
    alignCenter->setPixmap( icon.loadIcon( "text_center", (KIcon::Group)KIcon::Small ) );
    alignCenter->setToggleButton(true);
    alignCenter->setToolTip( i18n( "Align text to center" ) );
    
    QPushButton *alignBlock = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignBlock, ALIGN_BLOCK);
    alignBlock->setPixmap( icon.loadIcon( "text_block", (KIcon::Group)KIcon::Small ) );
    alignBlock->setToggleButton(true);
    alignBlock->setToolTip( i18n( "Align text to a block" ) );
    
    m_alignButtonGroup->setExclusive(true);
    m_alignButtonGroup->setFrameShape(QFrame::NoFrame);
    gridBox2->addMultiCellWidget(m_alignButtonGroup, 4, 4, 0, 1);
    
    // -------------------------------------------------------------
    
    QLabel *label1 = new QLabel(i18n("Rotation:"), gbox2);
    m_textRotation = new QComboBox( false, gbox2 );
    m_textRotation->insertItem( i18n("None") );
    m_textRotation->insertItem( i18n("90 Degrees") );
    m_textRotation->insertItem( i18n("180 Degrees") );
    m_textRotation->insertItem( i18n("270 Degrees") );
    m_textRotation->setWhatsThis( i18n("<p>Select here the text rotation to use."));
    gridBox2->addMultiCellWidget(label1, 5, 5, 0, 0);
    gridBox2->addMultiCellWidget(m_textRotation, 5, 5, 1, 1);

    // -------------------------------------------------------------
        
    QLabel *label2    = new QLabel(i18n("Color:"), gbox2);
    m_fontColorButton = new KColorButton( Qt::black, gbox2 );
    m_fontColorButton->setWhatsThis( i18n("<p>Set here the font color to use."));
    gridBox2->addMultiCellWidget(label2, 6, 6, 0, 0);
    gridBox2->addMultiCellWidget(m_fontColorButton, 6, 6, 1, 1);

    // -------------------------------------------------------------
        
    m_borderText = new QCheckBox( i18n( "Add border"), gbox2 );
    m_borderText->setToolTip( i18n( "Add a solid border around text using current text color" ) );

    m_transparentText = new QCheckBox( i18n( "Semi-transparent"), gbox2 );
    m_transparentText->setToolTip( i18n( "Use semi-transparent text background under image" ) );

    gridBox2->addMultiCellWidget(m_borderText, 7, 7, 0, 1);                            
    gridBox2->addMultiCellWidget(m_transparentText, 8, 8, 0, 1);                            
    gridBox2->setRowStretch(9, 10);    
    
    setUserAreaWidget(gbox2);

    // -------------------------------------------------------------
    
    connect(m_fontChooserWidget, SIGNAL(fontSelected (const QFont &)),
            this, SLOT(slotFontPropertiesChanged(const QFont &)));       
            
    connect(m_fontColorButton, SIGNAL(changed(const QColor &)),
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

    connect(this, SIGNAL(signalUpdatePreview()), SLOT(slotUpdatePreview()));
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
    m_textRotation->setCurrentItem( group.readEntry("Text Rotation", 0) );
    m_fontColorButton->setColor(group.readEntry("Font Color", black));
    m_textEdit->setText(group.readEntry("Text String", i18n("Enter your text here!")));
    m_textFont = group.readEntry("Font Properties", defaultFont);
    m_fontChooserWidget->setFont(m_textFont);
    m_alignTextMode = group.readEntry("Text Alignment", (int)ALIGN_LEFT);
    m_borderText->setChecked( group.readEntry("Border Text", false) );
    m_transparentText->setChecked( group.readEntry("Transparent Text", false) );
    m_previewWidget->setPositionHint( group.readEntry("Position Hint",QRect()) );

    static_cast<QPushButton*>(m_alignButtonGroup->find(m_alignTextMode))->setOn(true);
    slotAlignModeChanged(m_alignTextMode);
}

void ImageEffect_InsertText::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("inserttext Tool Dialog");

    group.writeEntry( "Text Rotation", m_textRotation->currentItem() );
    group.writeEntry( "Font Color", m_fontColorButton->color() );
    group.writeEntry( "Text String", m_textEdit->text() );
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
    
    m_textRotation->setCurrentItem(0);    // No rotation.
    m_fontColorButton->setColor(Qt::black);      
    QFont defaultFont;  
    m_textFont = defaultFont; // Reset to default KDE font.
    m_textFont.setPointSize( m_defaultSizeFont );
    m_fontChooserWidget->setFont(m_textFont);
    m_borderText->setChecked( false ); 
    m_transparentText->setChecked( false ); 
    m_previewWidget->resetEdit();
    static_cast<QPushButton*>(m_alignButtonGroup->find(ALIGN_LEFT))->setOn(true);
    
    m_fontChooserWidget->blockSignals(false);
    m_fontColorButton->blockSignals(false);
    m_alignButtonGroup->blockSignals(false);    
    slotAlignModeChanged(ALIGN_LEFT);
} 

void ImageEffect_InsertText::slotAlignModeChanged(int mode)
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

void ImageEffect_InsertText::slotFontPropertiesChanged(const QFont &font)
{
    m_textFont = font;
    emit signalUpdatePreview();
}

void ImageEffect_InsertText::slotUpdatePreview()
{
    m_previewWidget->setText(m_textEdit->text(), m_textFont, m_fontColorButton->color(), m_alignTextMode, 
                             m_borderText->isChecked(), m_transparentText->isChecked(),
                             m_textRotation->currentItem());
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

