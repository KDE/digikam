/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Jaromir Malenko <malenko at email.cz>
 * Date   : 2004-12-06
 * Description : digiKam image editor Ratio Crop tool
 *
 * Copyright 2004-2007 by Gilles Caulier
 * Copyright 2007 by Jaromir Malenko
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

#include <qlayout.h>
#include <qframe.h>
#include <qrect.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qcheckbox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kcolorbutton.h>

// Digikam includes.

#include <imageiface.h>
#include <imageselectionwidget.h>

// Local includes.

#include "imageeffect_ratiocrop.h"
#include "imageeffect_ratiocrop.moc"

namespace DigikamImagesPluginCore
{

ImageEffect_RatioCrop::ImageEffect_RatioCrop(QWidget* parent)
                     : Digikam::ImageDlgBase(parent, i18n("Aspect Ratio Crop & Composition Guide"),
                                             "aspectratiocrop", false)
{
    setHelp("ratiocroptool.anchor", "digikam");
    setButtonWhatsThis ( User1, i18n("<p>Set selection area to the maximum size according "
                                     "to the current ratio.") );
    setButtonText(User1, i18n("&Max. Aspect"));
    showButton(User1, true);

    // -------------------------------------------------------------

    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_imageSelectionWidget = new Digikam::ImageSelectionWidget(480, 320, frame);
    l->addWidget(m_imageSelectionWidget);
    QWhatsThis::add( m_imageSelectionWidget, i18n("<p>Here you can see the aspect ratio selection preview "
                                                  "used for cropping. You can use the mouse for moving and "
                                                  "resizing the crop area. "
                                                  "Hold CTRL to move the opposite corner too. "
                                                  "Hold SHIFT to move the closest corner to the mouse pointer."));
    setPreviewAreaWidget(frame);  

    // -------------------------------------------------------------

    QWidget *gbox2 = new QWidget(plainPage());
    QGridLayout *gridBox2 = new QGridLayout( gbox2, 2, 0);

    QFrame *cropSelection = new QFrame( gbox2 );
    cropSelection->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QGridLayout* grid = new QGridLayout( cropSelection, 6, 4, marginHint(), spacingHint());

    QLabel *label = new QLabel(i18n("Aspect ratio:"), cropSelection);
    m_ratioCB = new QComboBox( false, cropSelection );
    m_ratioCB->insertItem( i18n("Custom") );
    m_ratioCB->insertItem( "1:1" );
    m_ratioCB->insertItem( "2:3" );
    m_ratioCB->insertItem( "3:4" );
    m_ratioCB->insertItem( "4:5" );
    m_ratioCB->insertItem( "5:7" );
    m_ratioCB->insertItem( "7:10" );
    m_ratioCB->insertItem( i18n("Golden Ratio") );
    m_ratioCB->insertItem( i18n("None") );
    m_ratioCB->setCurrentText( "1:1" );
    QWhatsThis::add( m_ratioCB, i18n("<p>Select here your constrained aspect ratio for cropping. "
                                     "Aspect Ratio Crop tool uses a relative ratio. That means it "
                                     "is the same if you use centimeters or inches and it doesn't "
                                     "specify the physical size.<p>"
                                     "You can see below a correspondence list of traditional photographic "
                                     "paper sizes and aspect ratio crop:<p>"
                                     "<b>2:3</b>: 10x15cm, 20x30cm, 30x45cm, 3.5x5\", 4x6\", 8x12\", "
                                     "12x18\", 16x24\", 20x30\"<p>"
                                     "<b>3:4</b>: 6x8cm, 15x20cm, 18x24cm, 30x40cm, 3.75x5\", 4.5x6\", "
                                     "6x8\", 7.5x10\", 9x12\"<p>"
                                     "<b>4:5</b>: 20x25cm, 40x50cm, 8x10\", 16x20\"<p>"
                                     "<b>5:7</b>: 15x21cm, 30x42cm, 5x7\"<p>"
                                     "<b>7:10</b>: 21x30cm, 42x60cm<p>"
                                     "The <b>Golden Ratio</b> is 1:1.618. A composition following this rule "
                                     "is considered visually harmonious but can be unadapted to print on "
                                     "standard photographic paper."));

    m_orientLabel = new QLabel(i18n("Orientation:"), cropSelection);
    m_orientCB = new QComboBox( false, cropSelection );
    m_orientCB->insertItem( i18n("Landscape") );
    m_orientCB->insertItem( i18n("Portrait") );
    QWhatsThis::add( m_orientCB, i18n("<p>Select here constrained aspect ratio orientation."));

    m_autoOrientation = new QCheckBox(i18n("Auto"), cropSelection);
    QWhatsThis::add( m_autoOrientation, i18n("<p>Enable this option to automatic setting of orientation."));

    grid->addMultiCellWidget(label, 0, 0, 0, 0);
    grid->addMultiCellWidget(m_ratioCB, 0, 0, 1, 3);
    grid->addMultiCellWidget(m_orientLabel, 2, 2, 0, 0);
    grid->addMultiCellWidget(m_orientCB, 2, 2, 1, 3);
    grid->addMultiCellWidget(m_autoOrientation, 2, 2, 4, 4);

    // -------------------------------------------------------------

    m_customLabel1 = new QLabel(i18n("Custom ratio:"), cropSelection);
    m_customLabel1->setAlignment(AlignLeft|AlignVCenter);
    m_customRatioNInput = new KIntSpinBox(1, 10000, 1, 1, 10, cropSelection);
    QWhatsThis::add( m_customRatioNInput, i18n("<p>Set here the desired custom aspect numerator value."));
    m_customLabel2 = new QLabel(" : ", cropSelection);
    m_customLabel2->setAlignment(AlignCenter|AlignVCenter);
    m_customRatioDInput = new KIntSpinBox(1, 10000, 1, 1, 10, cropSelection);
    QWhatsThis::add( m_customRatioDInput, i18n("<p>Set here the desired custom aspect denominator value."));

    grid->addMultiCellWidget(m_customLabel1, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_customRatioNInput, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_customLabel2, 1, 1, 2, 2);
    grid->addMultiCellWidget(m_customRatioDInput, 1, 1, 3, 3);

    // -------------------------------------------------------------

    m_xInput = new KIntNumInput(cropSelection);
    QWhatsThis::add( m_xInput, i18n("<p>Set here the top left selection corner position for cropping."));
    m_xInput->setLabel(i18n("X:"), AlignLeft|AlignVCenter);
    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_widthInput = new KIntNumInput(cropSelection);
    m_widthInput->setLabel(i18n("Width:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_widthInput, i18n("<p>Set here the width selection for cropping."));
    m_widthInput->setRange(10, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_centerWidth = new QPushButton(cropSelection);
    KGlobal::dirs()->addResourceType("centerwidth", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("centerwidth", "centerwidth.png");
    m_centerWidth->setPixmap( QPixmap( directory + "centerwidth.png" ) );
    QWhatsThis::add( m_centerWidth, i18n("<p>Set width position to center."));

    grid->addMultiCellWidget(m_xInput, 3, 3, 0, 3);
    grid->addMultiCellWidget(m_widthInput, 4, 4, 0, 3);
    grid->addMultiCellWidget(m_centerWidth, 4, 4, 4, 4);

    // -------------------------------------------------------------

    m_yInput = new KIntNumInput(cropSelection);
    m_yInput->setLabel(i18n("Y:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_yInput, i18n("<p>Set here the top left selection corner position for cropping."));
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    m_heightInput = new KIntNumInput(cropSelection);
    m_heightInput->setLabel(i18n("Height:"), AlignLeft|AlignVCenter);
    QWhatsThis::add( m_heightInput, i18n("<p>Set here the height selection for cropping."));
    m_heightInput->setRange(10, m_imageSelectionWidget->getOriginalImageHeight(), 1, true);
    m_centerHeight = new QPushButton(cropSelection);
    KGlobal::dirs()->addResourceType("centerheight", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("centerheight", "centerheight.png");
    m_centerHeight->setPixmap( QPixmap( directory + "centerheight.png" ) );
    QWhatsThis::add( m_centerHeight, i18n("<p>Set height position to center."));

    grid->addMultiCellWidget(m_yInput, 5, 5, 0, 3);
    grid->addMultiCellWidget(m_heightInput, 6, 6, 0, 3);
    grid->addMultiCellWidget(m_centerHeight, 6, 6, 4, 4);

    gridBox2->addMultiCellWidget(cropSelection, 0, 0, 0, 0);

    // -------------------------------------------------------------

    QFrame* compositionGuide = new QFrame( gbox2 );
    QGridLayout* grid2 = new QGridLayout( compositionGuide, 7, 2, marginHint(), spacingHint());
    compositionGuide->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QLabel *labelGuideLines = new QLabel(i18n("Composition guide:"), compositionGuide);
    m_guideLinesCB = new QComboBox( false, compositionGuide );
    m_guideLinesCB->insertItem( i18n("Rules of Thirds") );
    m_guideLinesCB->insertItem( i18n("Harmonious Triangles") );
    m_guideLinesCB->insertItem( i18n("Golden Mean") );
    m_guideLinesCB->insertItem( i18n("None") );
    m_guideLinesCB->setCurrentText( i18n("None") );
    QWhatsThis::add( m_guideLinesCB, i18n("<p>With this option, you can display guide lines "
                                          "which help you to compose your photograph."));

    m_goldenSectionBox = new QCheckBox(i18n("Golden sections"), compositionGuide);
    QWhatsThis::add( m_goldenSectionBox, i18n("<p>Enable this option to show golden sections."));

    m_goldenSpiralSectionBox = new QCheckBox(i18n("Golden spiral sections"), compositionGuide);
    QWhatsThis::add( m_goldenSpiralSectionBox, i18n("<p>Enable this option to show golden spiral sections."));

    m_goldenSpiralBox = new QCheckBox(i18n("Golden spiral"), compositionGuide);
    QWhatsThis::add( m_goldenSpiralBox, i18n("<p>Enable this option to show golden spiral guide."));

    m_goldenTriangleBox = new QCheckBox(i18n("Golden triangles"), compositionGuide);
    QWhatsThis::add( m_goldenTriangleBox, i18n("<p>Enable this option to show golden triangles."));

    m_flipHorBox = new QCheckBox(i18n("Flip horizontally"), compositionGuide);
    QWhatsThis::add( m_flipHorBox, i18n("<p>Enable this option to flip horizontally guidelines."));

    m_flipVerBox = new QCheckBox(i18n("Flip vertically"), compositionGuide);
    QWhatsThis::add( m_flipVerBox, i18n("<p>Enable this option to flip vertically guidelines."));

    m_colorGuideLabel = new QLabel(i18n("Color and width:"), compositionGuide);
    m_guideColorBt = new KColorButton( QColor( 250, 250, 255 ), compositionGuide );
    m_guideSize = new QSpinBox( 1, 5, 1, compositionGuide);
    QWhatsThis::add( m_guideColorBt, i18n("<p>Set here the color used to draw composition guides."));
    QWhatsThis::add( m_guideSize, i18n("<p>Set here the width in pixels used to draw composition guides."));

    grid2->addMultiCellWidget(labelGuideLines, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_guideLinesCB, 0, 0, 1, 2);
    grid2->addMultiCellWidget(m_goldenSectionBox, 1, 1, 0, 2);
    grid2->addMultiCellWidget(m_goldenSpiralSectionBox, 2, 2, 0, 2);
    grid2->addMultiCellWidget(m_goldenSpiralBox, 3, 3, 0, 2);
    grid2->addMultiCellWidget(m_goldenTriangleBox, 4, 4, 0, 2);
    grid2->addMultiCellWidget(m_flipHorBox, 5, 5, 0, 2);
    grid2->addMultiCellWidget(m_flipVerBox, 6, 6, 0, 2);
    grid2->addMultiCellWidget(m_colorGuideLabel, 7, 7, 0, 0);
    grid2->addMultiCellWidget(m_guideColorBt, 7, 7, 1, 1);
    grid2->addMultiCellWidget(m_guideSize, 7, 7, 2, 2);

    gridBox2->addMultiCellWidget(compositionGuide, 1, 1, 0, 0);
    gridBox2->setRowStretch(2, 10);    

    setUserAreaWidget(gbox2);

    // -------------------------------------------------------------

    connect(m_ratioCB, SIGNAL(activated(int)),
            this, SLOT(slotRatioChanged(int)));

    connect(m_orientCB, SIGNAL(activated(int)),
            this, SLOT(slotOrientChanged(int)));

    connect(m_autoOrientation, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoOrientChanged(bool)));

    connect(m_xInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotXChanged(int)));

    connect(m_yInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotYChanged(int)));

    connect(m_customRatioNInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(m_customRatioDInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(m_guideLinesCB, SIGNAL(activated(int)),
            this, SLOT(slotGuideTypeChanged(int)));

    connect(m_goldenSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_goldenSpiralSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_goldenSpiralBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_goldenTriangleBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_flipHorBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_flipVerBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(m_guideColorBt, SIGNAL(changed(const QColor &)),
            m_imageSelectionWidget, SLOT(slotChangeGuideColor(const QColor &)));

    connect(m_guideSize, SIGNAL(valueChanged(int)),
            m_imageSelectionWidget, SLOT(slotChangeGuideSize(int)));

    connect(m_widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthChanged(int)));

    connect(m_heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightChanged(int)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionWidthChanged(int)),
            this, SLOT(slotSelectionWidthChanged(int)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionHeightChanged(int)),
            this, SLOT(slotSelectionHeightChanged(int)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionMoved(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));

    connect(m_imageSelectionWidget, SIGNAL(signalSelectionOrientationChanged(int)),
            this, SLOT(slotSelectionOrientationChanged(int)));

    connect(m_centerWidth, SIGNAL(clicked()),
            this, SLOT(slotCenterWidth()));

    connect(m_centerHeight, SIGNAL(clicked()),
            this, SLOT(slotCenterHeight()));

    // -------------------------------------------------------------

    readSettings();
}

ImageEffect_RatioCrop::~ImageEffect_RatioCrop()
{
    writeSettings();
}

void ImageEffect_RatioCrop::readSettings(void)
{
    Digikam::ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();

    QColor defaultGuideColor(250, 250, 255);
    KConfig *config = kapp->config();
    config->setGroup("aspectratiocrop Tool Dialog");

    // No guide lines per default.
    m_guideLinesCB->setCurrentItem( config->readNumEntry("Guide Lines Type",
                                    Digikam::ImageSelectionWidget::GuideNone) );
    m_goldenSectionBox->setChecked( config->readBoolEntry("Golden Section", true) );
    m_goldenSpiralSectionBox->setChecked( config->readBoolEntry("Golden Spiral Section", false) );
    m_goldenSpiralBox->setChecked( config->readBoolEntry("Golden Spiral", false) );
    m_goldenTriangleBox->setChecked( config->readBoolEntry("Golden Triangle", false) );
    m_flipHorBox->setChecked( config->readBoolEntry("Golden Flip Horizontal", false) );
    m_flipVerBox->setChecked( config->readBoolEntry("Golden Flip Vertical", false) );
    m_guideColorBt->setColor(config->readColorEntry("Guide Color", &defaultGuideColor));
    m_guideSize->setValue(config->readNumEntry("Guide Width", 1));
    m_imageSelectionWidget->slotGuideLines(m_guideLinesCB->currentItem());
    m_imageSelectionWidget->slotChangeGuideColor(m_guideColorBt->color());

    if (w > h)
    {
        m_xInput->setValue( config->readNumEntry("Hor.Oriented Custom Aspect Ratio Xpos", 50) );
        m_yInput->setValue( config->readNumEntry("Hor.Oriented Custom Aspect Ratio Ypos", 50) );

        m_ratioCB->setCurrentItem( config->readNumEntry("Hor.Oriented Aspect Ratio",
                                   Digikam::ImageSelectionWidget::RATIO03X04) );
        m_customRatioNInput->setValue( config->readNumEntry("Hor.Oriented Custom Aspect Ratio Num", 1) );
        m_customRatioDInput->setValue( config->readNumEntry("Hor.Oriented Custom Aspect Ratio Den", 1) );

        applyRatioChanges(m_ratioCB->currentItem());

        m_orientCB->setCurrentItem( config->readNumEntry("Hor.Oriented Aspect Ratio Orientation",
                                                         Digikam::ImageSelectionWidget::Landscape) );

        if ( m_ratioCB->currentItem() == Digikam::ImageSelectionWidget::RATIONONE )
        {
            m_widthInput->setValue( config->readNumEntry("Hor.Oriented Custom Aspect Ratio Width", 800) );
            m_heightInput->setValue( config->readNumEntry("Hor.Oriented Custom Aspect Ratio Height", 600) );
        }
        else
        {
            m_widthInput->setValue( 1 ); // It will be recalculed automatically with the ratio.
            m_heightInput->setValue( config->readNumEntry("Hor.Oriented Custom Aspect Ratio Height", 600) );
        }

        m_imageSelectionWidget->setSelectionOrientation(m_orientCB->currentItem());
    }
    else
    {
        m_xInput->setValue( config->readNumEntry("Ver.Oriented  Custom Aspect Ratio Xpos", 50) );
        m_yInput->setValue( config->readNumEntry("Ver.Oriented Custom Aspect Ratio Ypos", 50) );

        m_ratioCB->setCurrentItem( config->readNumEntry("Ver.Oriented Aspect Ratio",
                                   Digikam::ImageSelectionWidget::RATIO03X04) );
        m_customRatioNInput->setValue( config->readNumEntry("Ver.Oriented Custom Aspect Ratio Num", 1) );
        m_customRatioDInput->setValue( config->readNumEntry("Ver.Oriented Custom Aspect Ratio Den", 1) );

        applyRatioChanges(m_ratioCB->currentItem());

        m_orientCB->setCurrentItem( config->readNumEntry("Ver.Oriented Aspect Ratio Orientation",
                                                         Digikam::ImageSelectionWidget::Portrait) );

        if ( m_ratioCB->currentItem() == Digikam::ImageSelectionWidget::RATIONONE )
        {
            m_widthInput->setValue( config->readNumEntry("Ver.Oriented Custom Aspect Ratio Width", 800) );
            m_heightInput->setValue( config->readNumEntry("Ver.Oriented Custom Aspect Ratio Height", 600) );
        }
        else
        {
            m_widthInput->setValue( 1 ); // It will be recalculed automatically with the ratio.
            m_heightInput->setValue( config->readNumEntry("Ver.Oriented Custom Aspect Ratio Height", 600) );
        }

        m_imageSelectionWidget->setSelectionOrientation(m_orientCB->currentItem());
    }

    m_autoOrientation->setChecked( config->readBoolEntry("Auto Orientation", false) );
    slotAutoOrientChanged( m_autoOrientation->isChecked() );
}

void ImageEffect_RatioCrop::writeSettings(void)
{
    Digikam::ImageIface iface(0, 0);
    int w = iface.originalWidth();
    int h = iface.originalHeight();

    KConfig *config = kapp->config();
    config->setGroup("aspectratiocrop Tool Dialog");

    if (w > h)
    {
       config->writeEntry( "Hor.Oriented Aspect Ratio", m_ratioCB->currentItem() );
       config->writeEntry( "Hor.Oriented Aspect Ratio Orientation", m_orientCB->currentItem() );
       config->writeEntry( "Hor.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value() );
       config->writeEntry( "Hor.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value() );

       config->writeEntry( "Hor.Oriented Custom Aspect Ratio Xpos", m_xInput->value() );
       config->writeEntry( "Hor.Oriented Custom Aspect Ratio Ypos", m_yInput->value() );
       config->writeEntry( "Hor.Oriented Custom Aspect Ratio Width", m_widthInput->value() );
       config->writeEntry( "Hor.Oriented Custom Aspect Ratio Height", m_heightInput->value() );
    }
    else
    {
       config->writeEntry( "Ver.Oriented Aspect Ratio", m_ratioCB->currentItem() );
       config->writeEntry( "Ver.Oriented Aspect Ratio Orientation", m_orientCB->currentItem() );
       config->writeEntry( "Ver.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value() );
       config->writeEntry( "Ver.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value() );

       config->writeEntry( "Ver.Oriented Custom Aspect Ratio Xpos", m_xInput->value() );
       config->writeEntry( "Ver.Oriented Custom Aspect Ratio Ypos", m_yInput->value() );
       config->writeEntry( "Ver.Oriented Custom Aspect Ratio Width", m_widthInput->value() );
       config->writeEntry( "Ver.Oriented Custom Aspect Ratio Height", m_heightInput->value() );
    }

    config->writeEntry( "Auto Orientation", m_autoOrientation->isChecked() );
    config->writeEntry( "Guide Lines Type", m_guideLinesCB->currentItem() );
    config->writeEntry( "Golden Section", m_goldenSectionBox->isChecked() );
    config->writeEntry( "Golden Spiral Section", m_goldenSpiralSectionBox->isChecked() );
    config->writeEntry( "Golden Spiral", m_goldenSpiralBox->isChecked() );
    config->writeEntry( "Golden Triangle", m_goldenTriangleBox->isChecked() );
    config->writeEntry( "Golden Flip Horizontal", m_flipHorBox->isChecked() );
    config->writeEntry( "Golden Flip Vertical", m_flipVerBox->isChecked() );
    config->writeEntry( "Guide Color", m_guideColorBt->color() );
    config->writeEntry( "Guide Width", m_guideSize->value() );
    config->sync();
}

void ImageEffect_RatioCrop::slotDefault()
{
    m_imageSelectionWidget->resetSelection();
}

void ImageEffect_RatioCrop::slotUser1()
{
    m_imageSelectionWidget->maxAspectSelection();
}

void ImageEffect_RatioCrop::slotCenterWidth()
{
    m_imageSelectionWidget->setCenterSelection(Digikam::ImageSelectionWidget::CenterWidth);
}

void ImageEffect_RatioCrop::slotCenterHeight()
{
    m_imageSelectionWidget->setCenterSelection(Digikam::ImageSelectionWidget::CenterHeight);
}

void ImageEffect_RatioCrop::slotSelectionChanged(QRect rect)
{
    m_xInput->blockSignals(true);
    m_yInput->blockSignals(true);
    m_widthInput->blockSignals(true);
    m_heightInput->blockSignals(true);

    m_xInput->setValue(rect.x());
    m_yInput->setValue(rect.y());
    m_widthInput->setValue(rect.width());
    m_heightInput->setValue(rect.height());

    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth() - rect.width(), 1, true);
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageHeight() - rect.height(), 1, true);
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getOriginalImageWidth() - rect.x() , 1, true);
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getOriginalImageHeight() - rect.y(), 1, true);

    m_xInput->blockSignals(false);
    m_yInput->blockSignals(false);
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionWidthChanged(int newWidth)
{
    m_widthInput->blockSignals(true);
    m_widthInput->setValue(newWidth);
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getOriginalImageWidth() -
                           m_imageSelectionWidget->getRegionSelection().x(), 1, true);
    m_widthInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionHeightChanged(int newHeight)
{
    m_heightInput->blockSignals(true);
    m_heightInput->setValue(newHeight);
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getOriginalImageHeight() -
                            m_imageSelectionWidget->getRegionSelection().y(), 1, true);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionOrientationChanged(int newOrientation)
{
    m_orientCB->setCurrentItem(newOrientation);
}

void ImageEffect_RatioCrop::slotXChanged(int x)
{
    m_imageSelectionWidget->setSelectionX(x);
}

void ImageEffect_RatioCrop::slotYChanged(int y)
{
    m_imageSelectionWidget->setSelectionY(y);
}

void ImageEffect_RatioCrop::slotWidthChanged(int w)
{
    m_imageSelectionWidget->setSelectionWidth(w);
}

void ImageEffect_RatioCrop::slotHeightChanged(int h)
{
    m_imageSelectionWidget->setSelectionHeight(h);
}

void ImageEffect_RatioCrop::slotOrientChanged(int o)
{
    m_imageSelectionWidget->setSelectionOrientation(o);

    // Reset selection area.
    slotDefault();
}

void ImageEffect_RatioCrop::slotAutoOrientChanged(bool a)
{
    m_orientCB->setEnabled(!a /*|| m_ratioCB->currentItem() == Digikam::ImageSelectionWidget::RATIONONE*/);
    m_imageSelectionWidget->setAutoOrientation(a);
}

void ImageEffect_RatioCrop::slotRatioChanged(int a)
{
    applyRatioChanges(a);

    // Reset selection area.
    slotDefault();
}

void ImageEffect_RatioCrop::applyRatioChanges(int a)
{
    m_imageSelectionWidget->setSelectionAspectRatioType(a);

    if ( a == Digikam::ImageSelectionWidget::RATIOCUSTOM )
    {
       m_customLabel1->setEnabled(true);
       m_customLabel2->setEnabled(true);
       m_customRatioNInput->setEnabled(true);
       m_customRatioDInput->setEnabled(true);
       m_orientLabel->setEnabled(true);
       m_orientCB->setEnabled(! m_autoOrientation->isChecked());
       m_autoOrientation->setEnabled(true);
       slotCustomRatioChanged();
    }
    else if ( a == Digikam::ImageSelectionWidget::RATIONONE )
    {
       m_orientLabel->setEnabled(false);
       m_orientCB->setEnabled(false);
       m_autoOrientation->setEnabled(false);
       m_customLabel1->setEnabled(false);
       m_customLabel2->setEnabled(false);
       m_customRatioNInput->setEnabled(false);
       m_customRatioDInput->setEnabled(false);
    }
    else        // Pre-config ratio selected.
    {
       m_orientLabel->setEnabled(true);
       m_orientCB->setEnabled(! m_autoOrientation->isChecked());
       m_autoOrientation->setEnabled(true);
       m_customLabel1->setEnabled(false);
       m_customLabel2->setEnabled(false);
       m_customRatioNInput->setEnabled(false);
       m_customRatioDInput->setEnabled(false);
    }
}

void ImageEffect_RatioCrop::slotGuideTypeChanged(int t)
{
    if ( t == Digikam::ImageSelectionWidget::GuideNone )
    {
       m_goldenSectionBox->setEnabled(false);
       m_goldenSpiralSectionBox->setEnabled(false);
       m_goldenSpiralBox->setEnabled(false);
       m_goldenTriangleBox->setEnabled(false);
       m_flipHorBox->setEnabled(false);
       m_flipVerBox->setEnabled(false);
       m_colorGuideLabel->setEnabled(false);
       m_guideColorBt->setEnabled(false);
       m_guideSize->setEnabled(false);
    }
    else if ( t == Digikam::ImageSelectionWidget::RulesOfThirds )
    {
       m_goldenSectionBox->setEnabled(false);
       m_goldenSpiralSectionBox->setEnabled(false);
       m_goldenSpiralBox->setEnabled(false);
       m_goldenTriangleBox->setEnabled(false);
       m_flipHorBox->setEnabled(false);
       m_flipVerBox->setEnabled(false);
       m_colorGuideLabel->setEnabled(true);
       m_guideColorBt->setEnabled(true);
       m_guideSize->setEnabled(true);
    }
    else if ( t == Digikam::ImageSelectionWidget::HarmoniousTriangles )
    {
       m_goldenSectionBox->setEnabled(false);
       m_goldenSpiralSectionBox->setEnabled(false);
       m_goldenSpiralBox->setEnabled(false);
       m_goldenTriangleBox->setEnabled(false);
       m_flipHorBox->setEnabled(true);
       m_flipVerBox->setEnabled(true);
       m_colorGuideLabel->setEnabled(true);
       m_guideColorBt->setEnabled(true);
       m_guideSize->setEnabled(true);
    }
    else
    {
       m_goldenSectionBox->setEnabled(true);
       m_goldenSpiralSectionBox->setEnabled(true);
       m_goldenSpiralBox->setEnabled(true);
       m_goldenTriangleBox->setEnabled(true);
       m_flipHorBox->setEnabled(true);
       m_flipVerBox->setEnabled(true);
       m_colorGuideLabel->setEnabled(true);
       m_guideColorBt->setEnabled(true);
       m_guideSize->setEnabled(true);
    }

    m_imageSelectionWidget->setGoldenGuideTypes(m_goldenSectionBox->isChecked(),
                                                m_goldenSpiralSectionBox->isChecked(),
                                                m_goldenSpiralBox->isChecked(),
                                                m_goldenTriangleBox->isChecked(),
                                                m_flipHorBox->isChecked(),
                                                m_flipVerBox->isChecked());
    m_imageSelectionWidget->slotGuideLines(t);
}

void ImageEffect_RatioCrop::slotGoldenGuideTypeChanged(void)
{
    slotGuideTypeChanged(m_guideLinesCB->currentItem());
}

void ImageEffect_RatioCrop::slotCustomRatioChanged(void)
{
    m_imageSelectionWidget->setSelectionAspectRatioValue(
            (float)(m_customRatioNInput->value()) / (float)(m_customRatioDInput->value()) );

    // Reset selection area.
    slotDefault();
}

void ImageEffect_RatioCrop::slotOk()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    
    QRect currentRegion        = m_imageSelectionWidget->getRegionSelection();
    Digikam::ImageIface* iface = m_imageSelectionWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();
    
    QRect normalizedRegion = currentRegion.normalize();
    if (normalizedRegion.right() > w) normalizedRegion.setRight(w);
    if (normalizedRegion.bottom() > h) normalizedRegion.setBottom(h);

    Digikam::DImg imOrg(w, h, sb, a, data);
    delete [] data;
    imOrg.crop(normalizedRegion);

    iface->putOriginalImage(i18n("Aspect Ratio Crop"), imOrg.bits(), imOrg.width(), imOrg.height());

    kapp->restoreOverrideCursor();
    accept();
}

}  // NameSpace DigikamImagesPluginCore

