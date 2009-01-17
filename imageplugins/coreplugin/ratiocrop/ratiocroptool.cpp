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
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qcheckbox.h>
#include <qframe.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qrect.h>
#include <qspinbox.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Digikam includes.

#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageselectionwidget.h"

// Local includes.

#include "ratiocroptool.h"
#include "ratiocroptool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

RatioCropTool::RatioCropTool(QObject* parent)
             : EditorTool(parent)
{
    setName("aspectratiocrop");
    setToolName(i18n("Aspect Ratio Crop"));
    setToolIcon(SmallIcon("ratiocrop"));
    setToolHelp("ratiocroptool.anchor");

    // -------------------------------------------------------------

    m_imageSelectionWidget = new ImageSelectionWidget(480, 320);
    QWhatsThis::add(m_imageSelectionWidget,
                    i18n("<p>Here you can see the aspect ratio selection preview "
                         "used for cropping. You can use the mouse to move and "
                         "resize the crop area. "
                         "Press and hold the CTRL key to move the opposite corner too. "
                         "Press and hold the SHIFT key to move the closest corner to the "
                         "mouse pointer."));

    m_originalIsLandscape = ((m_imageSelectionWidget->getOriginalImageWidth()) >
                             (m_imageSelectionWidget->getOriginalImageHeight()));

    setToolView(m_imageSelectionWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Try|
                                            EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    // need to set the button to a KStdGuiItem that has no icon
    m_gboxSettings->button(EditorToolSettings::Try)->setGuiItem(KStdGuiItem::Test);
    // now we can set the correct text for the button
    m_gboxSettings->button(EditorToolSettings::Try)->setText(i18n("Max. Aspect"));

    QToolTip::add(m_gboxSettings->button(EditorToolSettings::Try),
                  i18n("<p>Set selection area to the maximum size according "
                       "to the current ratio."));

    // -------------------------------------------------------------

    QGridLayout *gboxLayout = new QGridLayout(m_gboxSettings->plainPage(), 3, 2);

    QFrame *cropSelection   = new QFrame(m_gboxSettings->plainPage());
    cropSelection->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QGridLayout* grid       = new QGridLayout(cropSelection, 7, 5);

    QLabel *label           = new QLabel(i18n("Ratio:"), cropSelection);
    m_ratioCB               = new RComboBox(cropSelection);
    m_ratioCB->setDefaultItem(ImageSelectionWidget::RATIO03X04);
    setRatioCBText(ImageSelectionWidget::Landscape);
    QWhatsThis::add( m_ratioCB, i18n("<p>Select your constrained aspect ratio for cropping. "
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
    QWhatsThis::add( m_preciseCrop, i18n("<p>Enable this option to force exact aspect ratio crop."));

    m_orientLabel = new QLabel(i18n("Orientation:"), cropSelection);
    m_orientCB    = new RComboBox(cropSelection);
    m_orientCB->insertItem( i18n("Landscape"));
    m_orientCB->insertItem( i18n("Portrait"));
    m_orientCB->setDefaultItem(ImageSelectionWidget::Landscape);
    QWhatsThis::add( m_orientCB, i18n("<p>Select constrained aspect ratio orientation."));

    m_autoOrientation = new QCheckBox(i18n("Auto"), cropSelection);
    QWhatsThis::add( m_autoOrientation, i18n("<p>Enable this option to automatically set the orientation."));

    // -------------------------------------------------------------

    m_customLabel1 = new QLabel(i18n("Custom:"), cropSelection);
    m_customLabel1->setAlignment(AlignLeft|AlignVCenter);
    m_customRatioNInput = new RIntNumInput(cropSelection);
    m_customRatioNInput->input()->setRange(1, 10000, 1, false);
    m_customRatioNInput->setDefaultValue(1);
    QWhatsThis::add( m_customRatioNInput, i18n("<p>Set here the desired custom aspect numerator value."));

    m_customLabel2 = new QLabel(" : ", cropSelection);
    m_customLabel2->setAlignment(AlignCenter|AlignVCenter);
    m_customRatioDInput = new RIntNumInput(cropSelection);
    m_customRatioDInput->input()->setRange(1, 10000, 1, false);
    m_customRatioDInput->setDefaultValue(1);
    QWhatsThis::add( m_customRatioDInput, i18n("<p>Set here the desired custom aspect denominator value."));

    // -------------------------------------------------------------

    m_xInput = new RIntNumInput(cropSelection);
    m_xInput->input()->setLabel(i18n("X:"), AlignLeft|AlignVCenter);
    m_xInput->setRange(0, m_imageSelectionWidget->getOriginalImageWidth(), 1);
    m_xInput->setDefaultValue(50);
    QWhatsThis::add( m_xInput, i18n("<p>Set here the top left selection corner position for cropping."));

    m_widthInput = new RIntNumInput(cropSelection);
    m_widthInput->input()->setLabel(i18n("Width:"), AlignLeft|AlignVCenter);
    m_widthInput->setRange(m_imageSelectionWidget->getMinWidthRange(),
                           m_imageSelectionWidget->getMaxWidthRange(),
                           m_imageSelectionWidget->getWidthStep());
    m_widthInput->setDefaultValue(800);
    QWhatsThis::add( m_widthInput, i18n("<p>Set here the width selection for cropping."));

    m_centerWidth = new QToolButton(cropSelection);
    KGlobal::dirs()->addResourceType("centerwidth", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("centerwidth", "centerwidth.png");
    m_centerWidth->setPixmap(QPixmap(directory + "centerwidth.png"));
    QWhatsThis::add(m_centerWidth, i18n("<p>Set width position to center."));

    // -------------------------------------------------------------

    m_yInput = new RIntNumInput(cropSelection);
    m_yInput->input()->setLabel(i18n("Y:"), AlignLeft | AlignVCenter);
    m_yInput->setRange(0, m_imageSelectionWidget->getOriginalImageHeight(), 1);
    m_yInput->setDefaultValue(50);
    QWhatsThis::add(m_yInput, i18n("<p>Set here the top left selection corner position for cropping."));

    m_heightInput = new RIntNumInput(cropSelection);
    m_heightInput->input()->setLabel(i18n("Height:"), AlignLeft | AlignVCenter);
    m_heightInput->setRange(m_imageSelectionWidget->getMinHeightRange(),
                            m_imageSelectionWidget->getMaxHeightRange(),
                            m_imageSelectionWidget->getHeightStep());
    m_heightInput->setDefaultValue(600);
    QWhatsThis::add( m_heightInput, i18n("<p>Set here the height selection for cropping."));

    m_centerHeight = new QToolButton(cropSelection);
    KGlobal::dirs()->addResourceType("centerheight", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("centerheight", "centerheight.png");
    m_centerHeight->setPixmap(QPixmap(directory + "centerheight.png"));
    QWhatsThis::add(m_centerHeight, i18n("<p>Set height position to center."));

    grid->addMultiCellWidget(label,               0, 0, 0, 0);
    grid->addMultiCellWidget(m_ratioCB,           0, 0, 1, 3);
    grid->addMultiCellWidget(m_preciseCrop,       0, 0, 4, 4);
    grid->addMultiCellWidget(m_customLabel1,      1, 1, 0, 0);
    grid->addMultiCellWidget(m_customRatioNInput, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_customLabel2,      1, 1, 2, 2);
    grid->addMultiCellWidget(m_customRatioDInput, 1, 1, 3, 3);
    grid->addMultiCellWidget(m_orientLabel,       2, 2, 0, 0);
    grid->addMultiCellWidget(m_orientCB,          2, 2, 1, 3);
    grid->addMultiCellWidget(m_autoOrientation,   2, 2, 4, 4);
    grid->addMultiCellWidget(m_xInput,            3, 3, 0, 3);
    grid->addMultiCellWidget(m_widthInput,        4, 4, 0, 3);
    grid->addMultiCellWidget(m_centerWidth,       4, 4, 4, 4);
    grid->addMultiCellWidget(m_yInput,            5, 5, 0, 3);
    grid->addMultiCellWidget(m_heightInput,       6, 6, 0, 3);
    grid->addMultiCellWidget(m_centerHeight,      6, 6, 4, 4);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    // -------------------------------------------------------------

    QFrame* compositionGuide = new QFrame(m_gboxSettings->plainPage());
    QGridLayout* grid2       = new QGridLayout(compositionGuide, 8, 3);
    compositionGuide->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    QLabel *labelGuideLines = new QLabel(i18n("Composition guide:"), compositionGuide);
    m_guideLinesCB = new RComboBox(compositionGuide);
    m_guideLinesCB->insertItem( i18n("Rules of Thirds"));
    m_guideLinesCB->insertItem( i18n("Diagonal Method"));
    m_guideLinesCB->insertItem( i18n("Harmonious Triangles"));
    m_guideLinesCB->insertItem( i18n("Golden Mean"));
    m_guideLinesCB->insertItem( i18n("None"));
    m_guideLinesCB->setDefaultItem(ImageSelectionWidget::GuideNone);
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

    m_colorGuideLabel   = new QLabel(i18n("Color and width:"), compositionGuide);
    m_guideColorBt      = new KColorButton(QColor(250, 250, 255), compositionGuide);
    m_guideSize         = new RIntNumInput(compositionGuide);
    m_guideSize->input()->setRange(1, 5, 1, false);
    m_guideSize->setDefaultValue(1);
    QWhatsThis::add( m_guideColorBt, i18n("<p>Set here the color used to draw composition guides."));
    QWhatsThis::add( m_guideSize, i18n("<p>Set here the width in pixels used to draw composition guides."));

    grid2->addMultiCellWidget(labelGuideLines,          0, 0, 0, 0);
    grid2->addMultiCellWidget(m_guideLinesCB,           0, 0, 1, 2);
    grid2->addMultiCellWidget(m_goldenSectionBox,       1, 1, 0, 2);
    grid2->addMultiCellWidget(m_goldenSpiralSectionBox, 2, 2, 0, 2);
    grid2->addMultiCellWidget(m_goldenSpiralBox,        3, 3, 0, 2);
    grid2->addMultiCellWidget(m_goldenTriangleBox,      4, 4, 0, 2);
    grid2->addMultiCellWidget(m_flipHorBox,             5, 5, 0, 2);
    grid2->addMultiCellWidget(m_flipVerBox,             6, 6, 0, 2);
    grid2->addMultiCellWidget(m_colorGuideLabel,        7, 7, 0, 0);
    grid2->addMultiCellWidget(m_guideColorBt,           7, 7, 1, 1);
    grid2->addMultiCellWidget(m_guideSize,              7, 7, 2, 2);
    grid2->setMargin(m_gboxSettings->spacingHint());
    grid2->setSpacing(m_gboxSettings->spacingHint());


    // -------------------------------------------------------------

    gboxLayout->addMultiCellWidget(cropSelection,    0, 0, 0, 1);
    gboxLayout->addMultiCellWidget(compositionGuide, 1, 1, 0, 1);
    gboxLayout->setRowStretch(2, 10);
    gboxLayout->setMargin(m_gboxSettings->spacingHint());
    gboxLayout->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

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
            this, SLOT(slotCustomNRatioChanged(int)));

    connect(m_customRatioDInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomDRatioChanged(int)));

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

    connect(m_guideColorBt, SIGNAL(changed(const QColor&)),
            m_imageSelectionWidget, SLOT(slotChangeGuideColor(const QColor&)));

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

    // we need to disconnect the standard connection of the Try button first
    disconnect(m_gboxSettings, SIGNAL(signalTryClicked()),
               this, SLOT(slotEffect()));

    connect(m_gboxSettings, SIGNAL(signalTryClicked()),
            this, SLOT(slotMaxAspectRatio()));

    // -------------------------------------------------------------

    // Sets current region selection
    slotSelectionChanged(m_imageSelectionWidget->getRegionSelection());
}

RatioCropTool::~RatioCropTool()
{
}

void RatioCropTool::readSettings()
{
    QColor defaultGuideColor(250, 250, 255);
    KConfig *config = kapp->config();
    config->setGroup("aspectratiocrop Tool");

    // No guide lines per default.
    m_guideLinesCB->setCurrentItem(config->readNumEntry("Guide Lines Type", ImageSelectionWidget::GuideNone));
    m_goldenSectionBox->setChecked(config->readBoolEntry("Golden Section", true));
    m_goldenSpiralSectionBox->setChecked(config->readBoolEntry("Golden Spiral Section", false));
    m_goldenSpiralBox->setChecked(config->readBoolEntry("Golden Spiral", false));
    m_goldenTriangleBox->setChecked(config->readBoolEntry("Golden Triangle", false));
    m_flipHorBox->setChecked(config->readBoolEntry("Golden Flip Horizontal", false));
    m_flipVerBox->setChecked(config->readBoolEntry("Golden Flip Vertical", false));
    m_guideColorBt->setColor(config->readColorEntry("Guide Color", &defaultGuideColor));
    m_guideSize->setValue(config->readNumEntry("Guide Width", m_guideSize->defaultValue()));
    m_imageSelectionWidget->slotGuideLines(m_guideLinesCB->currentItem());
    m_imageSelectionWidget->slotChangeGuideColor(m_guideColorBt->color());

    m_preciseCrop->setChecked(config->readBoolEntry("Precise Aspect Ratio Crop", false));
    m_imageSelectionWidget->setPreciseCrop(m_preciseCrop->isChecked());

    // Empty selection so it can be moved w/out size constraint
    m_widthInput->setValue(0);
    m_heightInput->setValue(0);

    m_xInput->setValue(config->readNumEntry("Hor.Oriented Custom Aspect Ratio Xpos",
                       m_xInput->defaultValue()));
    m_yInput->setValue(config->readNumEntry("Hor.Oriented Custom Aspect Ratio Ypos",
                       m_yInput->defaultValue()));

    m_widthInput->setValue(config->readNumEntry("Hor.Oriented Custom Aspect Ratio Width",
                           m_widthInput->defaultValue()));
    m_heightInput->setValue(config->readNumEntry("Hor.Oriented Custom Aspect Ratio Height",
                            m_heightInput->defaultValue()));

    m_imageSelectionWidget->setSelectionOrientation(m_orientCB->currentItem());

    m_customRatioNInput->setValue(config->readNumEntry("Hor.Oriented Custom Aspect Ratio Num",
            m_customRatioNInput->defaultValue()));
    m_customRatioDInput->setValue(config->readNumEntry("Hor.Oriented Custom Aspect Ratio Den",
            m_customRatioDInput->defaultValue()));

    m_ratioCB->setCurrentItem(config->readNumEntry("Hor.Oriented Aspect Ratio",
            m_ratioCB->defaultItem()));

    if (m_originalIsLandscape)
    {
        m_orientCB->setCurrentItem(config->readNumEntry("Hor.Oriented Aspect Ratio Orientation",
                                   ImageSelectionWidget::Landscape));
        m_orientCB->setDefaultItem(ImageSelectionWidget::Landscape);
    }
    else
    {
        m_orientCB->setCurrentItem(config->readNumEntry("Ver.Oriented Aspect Ratio Orientation",
                                   ImageSelectionWidget::Portrait));
        m_orientCB->setDefaultItem(ImageSelectionWidget::Portrait);
    }

    applyRatioChanges(m_ratioCB->currentItem());

    m_autoOrientation->setChecked( config->readBoolEntry("Auto Orientation", false) );
    slotAutoOrientChanged( m_autoOrientation->isChecked() );
}

void RatioCropTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("aspectratiocrop Tool");

    if (m_originalIsLandscape)
    {
        config->writeEntry("Hor.Oriented Aspect Ratio", m_ratioCB->currentItem());
        config->writeEntry("Hor.Oriented Aspect Ratio Orientation", m_orientCB->currentItem());
        config->writeEntry("Hor.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value());
        config->writeEntry("Hor.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value());

        config->writeEntry("Hor.Oriented Custom Aspect Ratio Xpos", m_xInput->value());
        config->writeEntry("Hor.Oriented Custom Aspect Ratio Ypos", m_yInput->value());
        config->writeEntry("Hor.Oriented Custom Aspect Ratio Width", m_widthInput->value());
        config->writeEntry("Hor.Oriented Custom Aspect Ratio Height", m_heightInput->value());
    }
    else
    {
        config->writeEntry("Ver.Oriented Aspect Ratio", m_ratioCB->currentItem());
        config->writeEntry("Ver.Oriented Aspect Ratio Orientation", m_orientCB->currentItem());
        config->writeEntry("Ver.Oriented Custom Aspect Ratio Num", m_customRatioNInput->value());
        config->writeEntry("Ver.Oriented Custom Aspect Ratio Den", m_customRatioDInput->value());

        config->writeEntry("Ver.Oriented Custom Aspect Ratio Xpos", m_xInput->value());
        config->writeEntry("Ver.Oriented Custom Aspect Ratio Ypos", m_yInput->value());
        config->writeEntry("Ver.Oriented Custom Aspect Ratio Width", m_widthInput->value());
        config->writeEntry("Ver.Oriented Custom Aspect Ratio Height", m_heightInput->value());
    }

    config->writeEntry("Precise Aspect Ratio Crop", m_preciseCrop->isChecked());
    config->writeEntry("Auto Orientation", m_autoOrientation->isChecked());
    config->writeEntry("Guide Lines Type", m_guideLinesCB->currentItem());
    config->writeEntry("Golden Section", m_goldenSectionBox->isChecked());
    config->writeEntry("Golden Spiral Section", m_goldenSpiralSectionBox->isChecked());
    config->writeEntry("Golden Spiral", m_goldenSpiralBox->isChecked());
    config->writeEntry("Golden Triangle", m_goldenTriangleBox->isChecked());
    config->writeEntry("Golden Flip Horizontal", m_flipHorBox->isChecked());
    config->writeEntry("Golden Flip Vertical", m_flipVerBox->isChecked());
    config->writeEntry("Guide Color", m_guideColorBt->color());
    config->writeEntry("Guide Width", m_guideSize->value());
    config->sync();
}

void RatioCropTool::slotResetSettings()
{
    m_imageSelectionWidget->resetSelection();
}

void RatioCropTool::slotMaxAspectRatio()
{
    m_imageSelectionWidget->maxAspectSelection();
}

void RatioCropTool::slotCenterWidth()
{
    m_imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterWidth);
}

void RatioCropTool::slotCenterHeight()
{
    m_imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterHeight);
}

void RatioCropTool::slotSelectionChanged(QRect rect)
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

    m_gboxSettings->enableButton(EditorToolSettings::Ok, rect.isValid());
    m_preciseCrop->setEnabled(m_imageSelectionWidget->preciseCropAvailable());

    m_xInput->blockSignals(false);
    m_yInput->blockSignals(false);
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void RatioCropTool::setRatioCBText(int orientation)
{
    int item = m_ratioCB->currentItem();

    m_ratioCB->blockSignals(true);
    m_ratioCB->combo()->clear();
    m_ratioCB->insertItem(i18n("Custom"));
    m_ratioCB->insertItem("1:1");
    if (orientation == ImageSelectionWidget::Landscape)
    {
        m_ratioCB->insertItem("3:2");
        m_ratioCB->insertItem("4:3");
        m_ratioCB->insertItem("5:4");
        m_ratioCB->insertItem("7:5");
        m_ratioCB->insertItem("10:7");
    }
    else
    {
        m_ratioCB->insertItem("2:3");
        m_ratioCB->insertItem("3:4");
        m_ratioCB->insertItem("4:5");
        m_ratioCB->insertItem("5:7");
        m_ratioCB->insertItem("7:10");
    }
    m_ratioCB->insertItem(i18n("Golden Ratio"));
    m_ratioCB->insertItem(i18n("None"));
    m_ratioCB->setCurrentItem(item);
    m_ratioCB->blockSignals(false);
}

void RatioCropTool::slotSelectionOrientationChanged(int newOrientation)
{
    // Change text for Aspect ratio ComboBox

    setRatioCBText(newOrientation);

    // Change Orientation ComboBox

    m_orientCB->setCurrentItem(newOrientation);

    // Reverse custom values

    if ( ( m_customRatioNInput->value() < m_customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Landscape) ||
         ( m_customRatioNInput->value() > m_customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Portrait))
    {
        m_customRatioNInput->blockSignals(true);
        m_customRatioDInput->blockSignals(true);

        int tmp = m_customRatioNInput->value();
        m_customRatioNInput->setValue(m_customRatioDInput->value());
        m_customRatioDInput->setValue(tmp);

        m_customRatioNInput->blockSignals(false);
        m_customRatioDInput->blockSignals(false);
    }
}

void RatioCropTool::slotXChanged(int x)
{
    m_imageSelectionWidget->setSelectionX(x);
}

void RatioCropTool::slotYChanged(int y)
{
    m_imageSelectionWidget->setSelectionY(y);
}

void RatioCropTool::slotWidthChanged(int w)
{
    m_imageSelectionWidget->setSelectionWidth(w);
}

void RatioCropTool::slotHeightChanged(int h)
{
    m_imageSelectionWidget->setSelectionHeight(h);
}

void RatioCropTool::slotPreciseCropChanged(bool a)
{
    m_imageSelectionWidget->setPreciseCrop(a);
}

void RatioCropTool::slotOrientChanged(int o)
{
    m_imageSelectionWidget->setSelectionOrientation(o);

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::slotAutoOrientChanged(bool a)
{
    m_orientCB->setEnabled(!a /*|| m_ratioCB->currentItem() == ImageSelectionWidget::RATIONONE*/);
    m_imageSelectionWidget->setAutoOrientation(a);
}

void RatioCropTool::slotRatioChanged(int a)
{
    applyRatioChanges(a);

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::applyRatioChanges(int a)
{
    m_imageSelectionWidget->setSelectionAspectRatioType(a);

    if (a == ImageSelectionWidget::RATIOCUSTOM)
    {
        m_customLabel1->setEnabled(true);
        m_customLabel2->setEnabled(true);
        m_customRatioNInput->setEnabled(true);
        m_customRatioDInput->setEnabled(true);
        m_orientLabel->setEnabled(true);
        m_orientCB->setEnabled(!m_autoOrientation->isChecked());
        m_autoOrientation->setEnabled(true);
        slotCustomRatioChanged();
    }
    else if (a == ImageSelectionWidget::RATIONONE)
    {
        m_orientLabel->setEnabled(false);
        m_orientCB->setEnabled(false);
        m_autoOrientation->setEnabled(false);
        m_customLabel1->setEnabled(false);
        m_customLabel2->setEnabled(false);
        m_customRatioNInput->setEnabled(false);
        m_customRatioDInput->setEnabled(false);
    }
    else // Pre-config ratio selected.
    {
        m_orientLabel->setEnabled(true);
        m_orientCB->setEnabled(!m_autoOrientation->isChecked());
        m_autoOrientation->setEnabled(true);
        m_customLabel1->setEnabled(false);
        m_customLabel2->setEnabled(false);
        m_customRatioNInput->setEnabled(false);
        m_customRatioDInput->setEnabled(false);
    }
}

void RatioCropTool::slotGuideTypeChanged(int t)
{
    if (t == ImageSelectionWidget::GuideNone)
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
    else if (t == ImageSelectionWidget::RulesOfThirds)
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
    else if (t == ImageSelectionWidget::DiagonalMethod)
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
    else if (t == ImageSelectionWidget::HarmoniousTriangles)
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

void RatioCropTool::slotGoldenGuideTypeChanged()
{
    slotGuideTypeChanged(m_guideLinesCB->currentItem());
}

void RatioCropTool::slotCustomNRatioChanged(int a)
{
    if ( ! m_autoOrientation->isChecked() )
    {
        if ( ( m_orientCB->currentItem() == ImageSelectionWidget::Portrait &&
               m_customRatioDInput->value() < a) ||
             ( m_orientCB->currentItem() == ImageSelectionWidget::Landscape &&
               m_customRatioDInput->value() > a))
        {
            m_customRatioDInput->blockSignals(true);
            m_customRatioDInput->setValue(a);
            m_customRatioDInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void RatioCropTool::slotCustomDRatioChanged(int a)
{
    if ( ! m_autoOrientation->isChecked() )
    {
        if ( ( m_orientCB->currentItem() == ImageSelectionWidget::Landscape &&
               m_customRatioNInput->value() < a) ||
             ( m_orientCB->currentItem() == ImageSelectionWidget::Portrait &&
               m_customRatioNInput->value() > a))
        {
            m_customRatioNInput->blockSignals(true);
            m_customRatioNInput->setValue(a);
            m_customRatioNInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void RatioCropTool::slotCustomRatioChanged()
{
    m_imageSelectionWidget->setSelectionAspectRatioValue(m_customRatioNInput->value(),
            m_customRatioDInput->value());

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    QRect currentRegion = m_imageSelectionWidget->getRegionSelection();
    ImageIface* iface   = m_imageSelectionWidget->imageIface();
    uchar *data         = iface->getOriginalImage();
    int w               = iface->originalWidth();
    int h               = iface->originalHeight();
    bool a              = iface->originalHasAlpha();
    bool sb             = iface->originalSixteenBit();

    QRect normalizedRegion = currentRegion.normalize();
    if (normalizedRegion.right() > w)
        normalizedRegion.setRight(w);

    if (normalizedRegion.bottom() > h)
        normalizedRegion.setBottom(h);

    DImg imOrg(w, h, sb, a, data);
    delete [] data;
    imOrg.crop(normalizedRegion);

    iface->putOriginalImage(i18n("Aspect Ratio Crop"), imOrg.bits(), imOrg.width(), imOrg.height());

    kapp->restoreOverrideCursor();
    writeSettings();
}

}  // NameSpace DigikamImagesPluginCore
