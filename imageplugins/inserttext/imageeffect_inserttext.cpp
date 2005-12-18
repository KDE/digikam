/* ============================================================
 * File  : imageeffect_inserttext.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-14
 * Description : a digiKam image plugin for insert text  
 *               to an image.
 * 
 * Copyright 2005 by Gilles Caulier
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
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qpen.h>
#include <qfont.h>
#include <qtimer.h> 
#include <qhbuttongroup.h> 
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kcolorbutton.h>
#include <ktextedit.h> 

// Local includes.

#include "version.h"
#include "inserttextwidget.h"
#include "fontchooserwidget.h"
#include "imageeffect_inserttext.h"

namespace DigikamInsertTextImagesPlugin
{

ImageEffect_InsertText::ImageEffect_InsertText(QWidget* parent, QString title, QFrame* banner)
                      : Digikam::ImageDlgBase(parent, title, "inserttext", false, banner)
{
    QString whatsThis;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Insert Text"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin for inserting text on a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    setAboutData(about);
    
    // -------------------------------------------------------------
    
    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new InsertTextWidget(480, 320, frame);
    l->addWidget(m_previewWidget);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the text inserted to the image. "
                                           "You can use the mouse for moving the text at the right location."));
    setPreviewAreaWidget(frame);                                           
    
    // -------------------------------------------------------------

    QWidget *gbox2 = new QWidget(plainPage());
    QGridLayout *gridBox2 = new QGridLayout( gbox2, 9, 1, marginHint(), spacingHint());
    
    m_textEdit = new KTextEdit(gbox2);
    m_textEdit->setCheckSpellingEnabled(true);
    m_textEdit->setWordWrap(QTextEdit::NoWrap);
    QWhatsThis::add( m_textEdit, i18n("<p>Enter here your text to insert on your image."));
    gridBox2->addMultiCellWidget(m_textEdit, 0, 2, 0, 1);
    
    // -------------------------------------------------------------
    
    m_fontChooserWidget = new FontChooserWidget(gbox2);
    QWhatsThis::add( m_textEdit, i18n("<p>Here you can choose the font to be used."));
    gridBox2->addMultiCellWidget(m_fontChooserWidget, 3, 3, 0, 1);
    
    // -------------------------------------------------------------
    
    KIconLoader icon;
    m_alignButtonGroup = new QHButtonGroup(gbox2);
    
    QPushButton *alignLeft = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignLeft, ALIGN_LEFT);
    alignLeft->setPixmap( icon.loadIcon( "text_left", (KIcon::Group)KIcon::Small ) );
    alignLeft->setToggleButton(true);
    QToolTip::add( alignLeft, i18n( "Align text to the left" ) );
    
    QPushButton *alignRight = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignRight, ALIGN_RIGHT);
    alignRight->setPixmap( icon.loadIcon( "text_right", (KIcon::Group)KIcon::Small ) );
    alignRight->setToggleButton(true);
    QToolTip::add( alignRight, i18n( "Align text to the right" ) );
    
    QPushButton *alignCenter = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignCenter, ALIGN_CENTER);
    alignCenter->setPixmap( icon.loadIcon( "text_center", (KIcon::Group)KIcon::Small ) );
    alignCenter->setToggleButton(true);
    QToolTip::add( alignCenter, i18n( "Align text to center" ) );
    
    QPushButton *alignBlock = new QPushButton( m_alignButtonGroup );
    m_alignButtonGroup->insert(alignBlock, ALIGN_BLOCK);
    alignBlock->setPixmap( icon.loadIcon( "text_block", (KIcon::Group)KIcon::Small ) );
    alignBlock->setToggleButton(true);
    QToolTip::add( alignBlock, i18n( "Align text to a block" ) );
    
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
    QWhatsThis::add( m_textRotation, i18n("<p>Select here the text rotation to use."));
    gridBox2->addMultiCellWidget(label1, 5, 5, 0, 0);
    gridBox2->addMultiCellWidget(m_textRotation, 5, 5, 1, 1);

    // -------------------------------------------------------------
        
    QLabel *label2 = new QLabel(i18n("Color:"), gbox2);
    m_fontColorButton = new KColorButton( Qt::black, gbox2 );
    QWhatsThis::add( m_fontColorButton, i18n("<p>Set here the font color to use."));
    gridBox2->addMultiCellWidget(label2, 6, 6, 0, 0);
    gridBox2->addMultiCellWidget(m_fontColorButton, 6, 6, 1, 1);

    // -------------------------------------------------------------
        
    m_borderText = new QCheckBox( i18n( "Add border"), gbox2 );
    QToolTip::add( m_borderText, i18n( "Add a solid border around text using current text color" ) );

    m_transparentText = new QCheckBox( i18n( "Semi-transparent"), gbox2 );
    QToolTip::add( m_transparentText, i18n( "Use semi-transparent text background under image" ) );

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
                        
    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(readSettings())); 
}

ImageEffect_InsertText::~ImageEffect_InsertText()
{
    writeSettings();
}

void ImageEffect_InsertText::readSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Insert Text Tool Settings");
    QColor *black = new QColor( 0, 0, 0 );
    QFont *defaultFont = new QFont();
    
    int orgW = m_previewWidget->imageIface()->originalWidth();
    int orgH = m_previewWidget->imageIface()->originalHeight();
    
    if ( orgW > orgH ) m_defaultSizeFont = (int)(orgH / 8.0);
    else m_defaultSizeFont = (int)(orgW / 8.0);
    
    defaultFont->setPointSize( m_defaultSizeFont );
    m_textRotation->setCurrentItem( config->readNumEntry("Text Rotation", 0) );
    m_fontColorButton->setColor(config->readColorEntry("Font Color", black));
    m_textEdit->setText(config->readEntry("Text String", i18n("Enter your text here!")));
    m_textFont = config->readFontEntry("Font Properties", defaultFont);
    m_fontChooserWidget->setFont(m_textFont);
    m_alignTextMode = config->readNumEntry("Text Alignment", ALIGN_LEFT);
    m_borderText->setChecked( config->readBoolEntry("Border Text", false) );
    m_transparentText->setChecked( config->readBoolEntry("Transparent Text", false) );
    
    delete black;
    delete defaultFont;
    
    static_cast<QPushButton*>(m_alignButtonGroup->find(m_alignTextMode))->setOn(true);
    slotAlignModeChanged(m_alignTextMode);
    
    m_previewWidget->resetEdit();
}
    
void ImageEffect_InsertText::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup("Insert Text Tool Settings");
    
    config->writeEntry( "Text Rotation", m_textRotation->currentItem() );
    config->writeEntry( "Font Color", m_fontColorButton->color() );
    config->writeEntry( "Text String", m_textEdit->text() );
    config->writeEntry( "Font Properties", m_textFont );
    config->writeEntry( "Text Alignment", m_alignTextMode );
    config->writeEntry( "Border Text", m_borderText->isChecked() );
    config->writeEntry( "Transparent Text", m_transparentText->isChecked() );
    
    config->sync();
}

void ImageEffect_InsertText::slotDefault()
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
    slotUpdatePreview();
}

void ImageEffect_InsertText::slotFontPropertiesChanged(const QFont &font)
{
    m_textFont = font;
    slotUpdatePreview();
}

void ImageEffect_InsertText::slotUpdatePreview()
{
    m_previewWidget->setText(m_textEdit->text(), m_textFont, m_fontColorButton->color(), m_alignTextMode, 
                             m_borderText->isChecked(), m_transparentText->isChecked(),
                             m_textRotation->currentItem());
}

void ImageEffect_InsertText::slotOk()
{
    accept();
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    Digikam::ImageIface iface(0, 0);
    QImage dest = m_previewWidget->makeInsertText();
    iface.putOriginalData(i18n("Insert Text"), (uint*)dest.bits(), dest.width(), dest.height());
       
    kapp->restoreOverrideCursor();       
}

}  // NameSpace DigikamInsertTextImagesPlugin

#include "imageeffect_inserttext.moc"
