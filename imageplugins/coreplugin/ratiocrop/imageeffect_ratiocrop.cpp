/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : digiKam image editor Ratio Crop tool
 *
 * Copyright (C) 2007 by Jaromir Malenko <malenko at email dot cz>
 * Copyright (C) 2008 by Roberto Castagnola <roberto dot castagnola at gmail dot com>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QFrame>
#include <QRect>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QImage>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>
#include <QGridLayout>
#include <QPixmap>
#include <QVBoxLayout>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kcolorbutton.h>

// Digikam includes.

#include "imageiface.h"
#include "imageselectionwidget.h"

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

    QFrame *frame          = new QFrame(mainWidget());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l         = new QVBoxLayout(frame);
    m_imageSelectionWidget = new ImageSelectionWidget(480, 320, frame);
    m_imageSelectionWidget->setWhatsThis(i18n("<p>Here you can see the aspect ratio selection preview "
                                              "used for cropping. You can use the mouse for moving and "
                                              "resizing the crop area. "
                                              "Hold CTRL to move the opposite corner too. "
                                              "Hold SHIFT to move the closest corner to the "
                                              "mouse pointer."));
    l->setMargin(5);
    l->setSpacing(0);
    l->addWidget(m_imageSelectionWidget);
    setPreviewAreaWidget(frame);

    m_originalIsLandscape = m_imageSelectionWidget->getOriginalImageWidth() >
                            m_imageSelectionWidget->getOriginalImageHeight();

    // -------------------------------------------------------------

    QWidget *gbox2        = new QWidget(mainWidget());
    QGridLayout *gridBox2 = new QGridLayout(gbox2);

    QFrame *cropSelection = new QFrame( gbox2 );
    QGridLayout* grid     = new QGridLayout(cropSelection);
    cropSelection->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QLabel *label = new QLabel(i18n("Aspect ratio:"), cropSelection);
    m_ratioCB     = new QComboBox( cropSelection );
    m_ratioCB->addItem( i18n("Custom") );
    m_ratioCB->addItem( "1:1" );
    m_ratioCB->addItem( "2:3" );
    m_ratioCB->addItem( "3:4" );
    m_ratioCB->addItem( "4:5" );
    m_ratioCB->addItem( "5:7" );
    m_ratioCB->addItem( "7:10" );
    m_ratioCB->addItem( i18n("Golden Ratio") );
    m_ratioCB->addItem( i18n("None") );
    setRatioCBText(ImageSelectionWidget::Landscape);
    m_ratioCB->setWhatsThis( i18n("<p>Select here your constrained aspect ratio for cropping. "
                                  "Aspect Ratio Crop tool uses a relative ratio. That means it "
                                  "is the same if you use centimeters or inches and it doesn't "
                                  "specify the physical size.<p>"
                                  "You can see below a correspondence list of traditional photographic "
                                  "paper sizes and aspect ratio crop:<p>"
                                  "<b>2:3</b>: 10x15cm, 20x30cm, 30x45cm, 4x6\", 8x12\", "
                                  "12x18\", 16x24\", 20x30\"<p>"
                                  "<b>3:4</b>: 6x8cm, 15x20cm, 18x24cm, 30x40cm, 3.75x5\", 4.5x6\", "
                                  "6x8\", 7.5x10\", 9x12\"<p>"
                                  "<b>4:5</b>: 20x25cm, 40x50cm, 8x10\", 16x20\"<p>"
                                  "<b>5:7</b>: 15x21cm, 30x42cm, 5x7\"<p>"
                                  "<b>7:10</b>: 21x30cm, 42x60cm, 3.5x5\"<p>"
                                  "The <b>Golden Ratio</b> is 1:1.618. A composition following this rule "
                                  "is considered visually harmonious but can be unadapted to print on "
                                  "standard photographic paper."));

    m_preciseCrop = new QCheckBox(i18n("Exact"), cropSelection);
    m_preciseCrop->setWhatsThis( i18n("<p>Enable this option to force exact aspect ratio crop."));

    m_orientLabel = new QLabel(i18n("Orientation:"), cropSelection);
    m_orientCB    = new QComboBox( cropSelection );
    m_orientCB->addItem( i18n("Landscape") );
    m_orientCB->addItem( i18n("Portrait") );
    m_orientCB->setWhatsThis( i18n("<p>Select here constrained aspect ratio orientation."));

    m_autoOrientation = new QCheckBox(i18n("Auto"), cropSelection);
    m_autoOrientation->setWhatsThis( i18n("<p>Enable this option to automatically set the orientation."));

    // -------------------------------------------------------------

    m_customLabel1 = new QLabel(i18n("Custom ratio:"), cropSelection);
    m_customLabel1->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    m_customRatioNInput = new KIntSpinBox(cropSelection);
    m_customRatioNInput->setRange(1, 10000);
    m_customRatioNInput->setSingleStep(1);
    m_customRatioNInput->setValue(1);
    m_customRatioNInput->setBase(10);
    m_customRatioNInput->setWhatsThis( i18n("<p>Set here the desired custom aspect numerator value."));

    m_customLabel2 = new QLabel(" : ", cropSelection);
    m_customLabel2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
    m_customRatioDInput = new KIntSpinBox(cropSelection);
    m_customRatioDInput->setRange(1, 10000);
    m_customRatioDInput->setSingleStep(1);
    m_customRatioDInput->setValue(1);
    m_customRatioDInput->setBase(10);
    m_customRatioDInput->setWhatsThis( i18n("<p>Set here the desired custom aspect denominator value."));

    // -------------------------------------------------------------

    m_xInput = new KIntNumInput(cropSelection);
    m_xInput->setWhatsThis( i18n("<p>Set here the top left selection corner position for cropping."));
    m_xInput->setLabel(i18n("X:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1);
    m_xInput->setSliderEnabled(true);

    m_widthInput = new KIntNumInput(cropSelection);
    m_widthInput->setLabel(i18n("Width:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_widthInput->setWhatsThis( i18n("<p>Set here the width selection for cropping."));
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getMaxWidthRange(),
                           m_imageSelectionWidget->getWidthStep());
    m_widthInput->setSliderEnabled(true);

    m_centerWidth = new QPushButton(cropSelection);
    m_centerWidth->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/centerwidth.png")));
    m_centerWidth->setWhatsThis( i18n("<p>Set width position to center."));

    // -------------------------------------------------------------

    m_yInput = new KIntNumInput(cropSelection);
    m_yInput->setLabel(i18n("Y:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_yInput->setWhatsThis( i18n("<p>Set here the top left selection corner position for cropping."));
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1);
    m_yInput->setSliderEnabled(true);

    m_heightInput = new KIntNumInput(cropSelection);
    m_heightInput->setLabel(i18n("Height:"), Qt::AlignLeft|Qt::AlignVCenter);
    m_heightInput->setWhatsThis( i18n("<p>Set here the height selection for cropping."));
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getMaxHeightRange(),
                            m_imageSelectionWidget->getHeightStep());
    m_heightInput->setSliderEnabled(true);

    m_centerHeight = new QPushButton(cropSelection);
    m_centerHeight->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/centerheight.png")));
    m_centerHeight->setWhatsThis( i18n("<p>Set height position to center."));

    // -------------------------------------------------------------

    QFrame* compositionGuide = new QFrame( gbox2 );
    QGridLayout* grid2       = new QGridLayout(compositionGuide);
    compositionGuide->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QLabel *labelGuideLines = new QLabel(i18n("Composition guide:"), compositionGuide);
    m_guideLinesCB = new QComboBox(compositionGuide);
    m_guideLinesCB->addItem( i18n("Rules of Thirds") );
    m_guideLinesCB->addItem( i18n("Harmonious Triangles") );
    m_guideLinesCB->addItem( i18n("Golden Mean") );
    m_guideLinesCB->addItem( i18n("None") );
    m_guideLinesCB->setCurrentIndex(3);
    m_guideLinesCB->setWhatsThis( i18n("<p>With this option, you can display guide lines "
                                       "which help you to compose your photograph."));

    m_goldenSectionBox = new QCheckBox(i18n("Golden sections"), compositionGuide);
    m_goldenSectionBox->setWhatsThis(i18n("<p>Enable this option to show golden sections."));

    m_goldenSpiralSectionBox = new QCheckBox(i18n("Golden spiral sections"), compositionGuide);
    m_goldenSpiralSectionBox->setWhatsThis(i18n("<p>Enable this option to show golden spiral sections."));

    m_goldenSpiralBox = new QCheckBox(i18n("Golden spiral"), compositionGuide);
    m_goldenSpiralBox->setWhatsThis(i18n("<p>Enable this option to show golden spiral guide."));

    m_goldenTriangleBox = new QCheckBox(i18n("Golden triangles"), compositionGuide);
    m_goldenTriangleBox->setWhatsThis(i18n("<p>Enable this option to show golden triangles."));

    m_flipHorBox = new QCheckBox(i18n("Flip horizontally"), compositionGuide);
    m_flipHorBox->setWhatsThis(i18n("<p>Enable this option to flip horizontally guidelines."));

    m_flipVerBox = new QCheckBox(i18n("Flip vertically"), compositionGuide);
    m_flipVerBox->setWhatsThis(i18n("<p>Enable this option to flip vertically guidelines."));

    m_colorGuideLabel = new QLabel(i18n("Color and width:"), compositionGuide);
    m_guideColorBt    = new KColorButton( QColor( 250, 250, 255 ), compositionGuide );
    m_guideSize       = new QSpinBox(compositionGuide);
    m_guideSize->setRange(1, 5);
    m_guideSize->setSingleStep(1);
    m_guideColorBt->setWhatsThis( i18n("<p>Set here the color used to draw composition guides."));
    m_guideSize->setWhatsThis( i18n("<p>Set here the width in pixels used to draw composition guides."));

    // -------------------------------------------------------------

    grid->addWidget(label, 0, 0, 1, 1);
    grid->addWidget(m_ratioCB, 0, 1, 1, 3);
    grid->addWidget(m_preciseCrop, 0, 1, 1, 1);
    grid->addWidget(m_orientLabel, 2, 0, 1, 1);
    grid->addWidget(m_orientCB, 2, 1, 1, 3);
    grid->addWidget(m_autoOrientation, 2, 4, 1, 1);
    grid->addWidget(m_customLabel1, 1, 0, 1, 1);
    grid->addWidget(m_customRatioNInput, 1, 1, 1, 1);
    grid->addWidget(m_customLabel2, 1, 2, 1, 1);
    grid->addWidget(m_customRatioDInput, 1, 3, 1, 1);
    grid->addWidget(m_xInput, 3, 0, 1, 4 );
    grid->addWidget(m_widthInput, 4, 0, 1, 4 );
    grid->addWidget(m_centerWidth, 4, 4, 1, 1);
    grid->addWidget(m_yInput, 5, 0, 1, 4 );
    grid->addWidget(m_heightInput, 6, 0, 1, 4 );
    grid->addWidget(m_centerHeight, 6, 4, 1, 1);
    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());

    grid2->addWidget(labelGuideLines, 0, 0, 1, 1);
    grid2->addWidget(m_guideLinesCB, 0, 1, 1, 2);
    grid2->addWidget(m_goldenSectionBox, 1, 0, 1, 3 );
    grid2->addWidget(m_goldenSpiralSectionBox, 2, 0, 1, 3 );
    grid2->addWidget(m_goldenSpiralBox, 3, 0, 1, 3 );
    grid2->addWidget(m_goldenTriangleBox, 4, 0, 1, 3 );
    grid2->addWidget(m_flipHorBox, 5, 0, 1, 3 );
    grid2->addWidget(m_flipVerBox, 6, 0, 1, 3 );
    grid2->addWidget(m_colorGuideLabel, 7, 0, 1, 1);
    grid2->addWidget(m_guideColorBt, 7, 1, 1, 1);
    grid2->addWidget(m_guideSize, 7, 2, 1, 1);
    grid2->setMargin(spacingHint());
    grid2->setSpacing(spacingHint());

    gridBox2->addWidget(cropSelection, 0, 0, 1, 1);
    gridBox2->addWidget(compositionGuide, 1, 0, 1, 1);
    gridBox2->setRowStretch(2, 10);    
    gridBox2->setMargin(spacingHint());
    gridBox2->setSpacing(spacingHint());

    setUserAreaWidget(gbox2);

    // -------------------------------------------------------------

    connect(m_ratioCB, SIGNAL(activated(int)),
            this, SLOT(slotRatioChanged(int)));

    connect(m_preciseCrop, SIGNAL(toggled(bool)),
            this, SLOT(slotPreciseCropChanged(bool)));

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

    // Sets current region selection
    slotSelectionChanged(m_imageSelectionWidget->getRegionSelection());

    readSettings();
}

ImageEffect_RatioCrop::~ImageEffect_RatioCrop()
{
}

void ImageEffect_RatioCrop::readSettings()
{
    QColor defaultGuideColor(250, 250, 255);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("aspectratiocrop Tool Dialog");

    // No guide lines per default.
    m_guideLinesCB->setCurrentIndex(group.readEntry("Guide Lines Type",
                                   (int)ImageSelectionWidget::GuideNone));
    m_goldenSectionBox->setChecked(group.readEntry("Golden Section", true));
    m_goldenSpiralSectionBox->setChecked(group.readEntry("Golden Spiral Section", false));
    m_goldenSpiralBox->setChecked(group.readEntry("Golden Spiral", false));
    m_goldenTriangleBox->setChecked(group.readEntry("Golden Triangle", false));
    m_flipHorBox->setChecked(group.readEntry("Golden Flip Horizontal", false));
    m_flipVerBox->setChecked(group.readEntry("Golden Flip Vertical", false));
    m_guideColorBt->setColor(group.readEntry("Guide Color", defaultGuideColor));
    m_guideSize->setValue(group.readEntry("Guide Width", 1));
    m_imageSelectionWidget->slotGuideLines(m_guideLinesCB->currentIndex());
    m_imageSelectionWidget->slotChangeGuideColor(m_guideColorBt->color());

    m_preciseCrop->setChecked( group.readEntry("Precise Aspect Ratio Crop", false) );
    m_imageSelectionWidget->setPreciseCrop( m_preciseCrop->isChecked() );

    if (m_originalIsLandscape)
    {
        m_orientCB->setCurrentIndex(group.readEntry("Hor.Oriented Aspect Ratio Orientation",
                                   (int)ImageSelectionWidget::Landscape));

        m_imageSelectionWidget->setSelectionOrientation(m_orientCB->currentIndex());

        m_customRatioNInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Num", 1));
        m_customRatioDInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Den", 1));
        m_ratioCB->setCurrentIndex(group.readEntry("Hor.Oriented Aspect Ratio",
                                  (int)ImageSelectionWidget::RATIO03X04));

        applyRatioChanges(m_ratioCB->currentIndex());

        // Empty selection so it can be moved w/out size constraint
        m_widthInput->setValue( 0 );
        m_heightInput->setValue( 0 );

        m_xInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Xpos", 50));
        m_yInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Ypos", 50));

        m_widthInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Width", 800));
        m_heightInput->setValue(group.readEntry("Hor.Oriented Custom Aspect Ratio Height", 600));
    }
    else
    {
        m_orientCB->setCurrentIndex(group.readEntry("Ver.Oriented Aspect Ratio Orientation",
                                   (int)ImageSelectionWidget::Portrait) );

        m_imageSelectionWidget->setSelectionOrientation(m_orientCB->currentIndex());

        m_customRatioNInput->setValue(group.readEntry("Ver.Oriented Custom Aspect Ratio Num", 1));
        m_customRatioDInput->setValue(group.readEntry("Ver.Oriented Custom Aspect Ratio Den", 1));
        m_ratioCB->setCurrentIndex(group.readEntry("Ver.Oriented Aspect Ratio",
                                  (int)ImageSelectionWidget::RATIO03X04) );

        applyRatioChanges(m_ratioCB->currentIndex());

        // Empty selection so it can be moved w/out size constraint
        m_widthInput->setValue( 0 );
        m_heightInput->setValue( 0 );

        m_xInput->setValue(group.readEntry("Ver.Oriented Custom Aspect Ratio Xpos", 50));
        m_yInput->setValue(group.readEntry("Ver.Oriented Custom Aspect Ratio Ypos", 50));

        m_widthInput->setValue(group.readEntry("Ver.Oriented Custom Aspect Ratio Width", 800));
        m_heightInput->setValue(group.readEntry("Ver.Oriented Custom Aspect Ratio Height", 600));
    }

    m_autoOrientation->setChecked(group.readEntry("Auto Orientation", false));
    slotAutoOrientChanged( m_autoOrientation->isChecked() );
}

void ImageEffect_RatioCrop::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("aspectratiocrop Tool Dialog");

    if (m_originalIsLandscape)
    {
       group.writeEntry( "Hor.Oriented Aspect Ratio", m_ratioCB->currentIndex() );
       group.writeEntry( "Hor.Oriented Aspect Ratio Orientation", m_orientCB->currentIndex() );
       group.writeEntry( "Hor.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value() );
       group.writeEntry( "Hor.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value() );

       group.writeEntry( "Hor.Oriented Custom Aspect Ratio Xpos", m_xInput->value() );
       group.writeEntry( "Hor.Oriented Custom Aspect Ratio Ypos", m_yInput->value() );
       group.writeEntry( "Hor.Oriented Custom Aspect Ratio Width", m_widthInput->value() );
       group.writeEntry( "Hor.Oriented Custom Aspect Ratio Height", m_heightInput->value() );
    }
    else
    {
       group.writeEntry( "Ver.Oriented Aspect Ratio", m_ratioCB->currentIndex() );
       group.writeEntry( "Ver.Oriented Aspect Ratio Orientation", m_orientCB->currentIndex() );
       group.writeEntry( "Ver.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value() );
       group.writeEntry( "Ver.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value() );

       group.writeEntry( "Ver.Oriented Custom Aspect Ratio Xpos", m_xInput->value() );
       group.writeEntry( "Ver.Oriented Custom Aspect Ratio Ypos", m_yInput->value() );
       group.writeEntry( "Ver.Oriented Custom Aspect Ratio Width", m_widthInput->value() );
       group.writeEntry( "Ver.Oriented Custom Aspect Ratio Height", m_heightInput->value() );
    }

    group.writeEntry( "Precise Aspect Ratio Crop", m_preciseCrop->isChecked() );
    group.writeEntry( "Auto Orientation", m_autoOrientation->isChecked() );
    group.writeEntry( "Guide Lines Type", m_guideLinesCB->currentIndex() );
    group.writeEntry( "Golden Section", m_goldenSectionBox->isChecked() );
    group.writeEntry( "Golden Spiral Section", m_goldenSpiralSectionBox->isChecked() );
    group.writeEntry( "Golden Spiral", m_goldenSpiralBox->isChecked() );
    group.writeEntry( "Golden Triangle", m_goldenTriangleBox->isChecked() );
    group.writeEntry( "Golden Flip Horizontal", m_flipHorBox->isChecked() );
    group.writeEntry( "Golden Flip Vertical", m_flipVerBox->isChecked() );
    group.writeEntry( "Guide Color", m_guideColorBt->color() );
    group.writeEntry( "Guide Width", m_guideSize->value() );
    group.sync();
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
    m_imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterWidth);
}

void ImageEffect_RatioCrop::slotCenterHeight()
{
    m_imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterHeight);
}

void ImageEffect_RatioCrop::slotSelectionChanged(QRect rect)
{
    m_xInput->blockSignals(true);
    m_yInput->blockSignals(true);
    m_widthInput->blockSignals(true);
    m_heightInput->blockSignals(true);

    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth() - rect.width(), 1);
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageHeight() - rect.height(), 1);
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getMaxWidthRange(), 
                           m_imageSelectionWidget->getWidthStep());
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getMaxHeightRange(), 
                            m_imageSelectionWidget->getHeightStep());

    m_xInput->setValue(rect.x());
    m_yInput->setValue(rect.y());
    m_widthInput->setValue(rect.width());
    m_heightInput->setValue(rect.height());

    enableButtonOk( rect.isValid() );
    m_preciseCrop->setEnabled(m_imageSelectionWidget->preciseCropAvailable());

    m_xInput->blockSignals(false);
    m_yInput->blockSignals(false);
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::setRatioCBText(int orientation)
{
    int item = m_ratioCB->currentIndex();
    m_ratioCB->blockSignals(true);
    m_ratioCB->clear();
    m_ratioCB->addItem( i18n("Custom") );
    m_ratioCB->addItem( "1:1" );
    if ( orientation == ImageSelectionWidget::Landscape )
    {
        m_ratioCB->addItem( "3:2" );
        m_ratioCB->addItem( "4:3" );
        m_ratioCB->addItem( "5:4" );
        m_ratioCB->addItem( "7:5" );
        m_ratioCB->addItem( "10:7" );
    }
    else
    {
        m_ratioCB->addItem( "2:3" );
        m_ratioCB->addItem( "3:4" );
        m_ratioCB->addItem( "4:5" );
        m_ratioCB->addItem( "5:7" );
        m_ratioCB->addItem( "7:10" );
    }
    m_ratioCB->addItem( i18n("Golden Ratio") );
    m_ratioCB->addItem( i18n("None") );
    m_ratioCB->setCurrentIndex( item );
    m_ratioCB->blockSignals(false);
}

void ImageEffect_RatioCrop::slotSelectionOrientationChanged(int newOrientation)
{
    // Change text for Aspect ratio ComboBox

    setRatioCBText(newOrientation);

    // Change Orientation ComboBox

    m_orientCB->setCurrentIndex(newOrientation);

    // Reverse custom values

    if ( ( m_customRatioNInput->value() < m_customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Landscape ) ||
         ( m_customRatioNInput->value() > m_customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Portrait ) )
    {
        m_customRatioNInput->blockSignals(true);
        m_customRatioDInput->blockSignals(true);

        int tmp = m_customRatioNInput->value();
        m_customRatioNInput->setValue( m_customRatioDInput->value() );
        m_customRatioDInput->setValue( tmp );

        m_customRatioNInput->blockSignals(false);
        m_customRatioDInput->blockSignals(false);
    }
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

void ImageEffect_RatioCrop::slotPreciseCropChanged(bool a)
{
    m_imageSelectionWidget->setPreciseCrop(a);
}

void ImageEffect_RatioCrop::slotOrientChanged(int o)
{
    m_imageSelectionWidget->setSelectionOrientation(o);

    // Reset selection area.
    slotDefault();
}

void ImageEffect_RatioCrop::slotAutoOrientChanged(bool a)
{
    m_orientCB->setEnabled(!a /*|| m_ratioCB->currentIndex() == ImageSelectionWidget::RATIONONE*/);
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

    if ( a == ImageSelectionWidget::RATIOCUSTOM )
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
    else if ( a == ImageSelectionWidget::RATIONONE )
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
    if ( t == ImageSelectionWidget::GuideNone )
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
    else if ( t == ImageSelectionWidget::RulesOfThirds )
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
    else if ( t == ImageSelectionWidget::HarmoniousTriangles )
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

void ImageEffect_RatioCrop::slotGoldenGuideTypeChanged()
{
    slotGuideTypeChanged(m_guideLinesCB->currentIndex());
}

void ImageEffect_RatioCrop::slotCustomNRatioChanged(int a)
{
    if ( ! m_autoOrientation->isChecked() )
    {
        if ( ( m_orientCB->currentIndex() == ImageSelectionWidget::Portrait &&
               m_customRatioDInput->value() < a ) ||
             ( m_orientCB->currentIndex() == ImageSelectionWidget::Landscape &&
               m_customRatioDInput->value() > a ) )
        {
            m_customRatioDInput->blockSignals(true);
            m_customRatioDInput->setValue(a);
            m_customRatioDInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void ImageEffect_RatioCrop::slotCustomDRatioChanged(int a)
{
    if ( ! m_autoOrientation->isChecked() )
    {
        if ( ( m_orientCB->currentIndex() == ImageSelectionWidget::Landscape &&
               m_customRatioNInput->value() < a ) ||
             ( m_orientCB->currentIndex() == ImageSelectionWidget::Portrait &&
               m_customRatioNInput->value() > a ) )
        {
            m_customRatioNInput->blockSignals(true);
            m_customRatioNInput->setValue(a);
            m_customRatioNInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void ImageEffect_RatioCrop::slotCustomRatioChanged()
{
    m_imageSelectionWidget->setSelectionAspectRatioValue(
            m_customRatioNInput->value(), m_customRatioDInput->value() );

    // Reset selection area.
    slotDefault();
}

void ImageEffect_RatioCrop::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    QRect currentRegion        = m_imageSelectionWidget->getRegionSelection();
    Digikam::ImageIface* iface = m_imageSelectionWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool a                     = iface->originalHasAlpha();
    bool sb                    = iface->originalSixteenBit();

    QRect normalizedRegion = currentRegion.normalized();
    if (normalizedRegion.right() > w) normalizedRegion.setRight(w);
    if (normalizedRegion.bottom() > h) normalizedRegion.setBottom(h);

    Digikam::DImg imOrg(w, h, sb, a, data);
    delete [] data;
    imOrg.crop(normalizedRegion);

    iface->putOriginalImage(i18n("Aspect Ratio Crop"), imOrg.bits(), imOrg.width(), imOrg.height());

    kapp->restoreOverrideCursor();
    writeSettings();
    accept();
}

}  // NameSpace DigikamImagesPluginCore
