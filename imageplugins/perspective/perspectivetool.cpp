/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to change image perspective .
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "perspectivetool.h"
#include "perspectivetool.moc"

// Qt includes.

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// Local includes.

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "version.h"
#include "perspectivewidget.h"

using namespace Digikam;

namespace DigikamPerspectiveImagesPlugin
{

PerspectiveTool::PerspectiveTool(QObject* parent)
               : EditorTool(parent)
{
    setObjectName("perspective");
    setToolName(i18n("Perspective"));
    setToolIcon(SmallIcon("perspective"));

    // -------------------------------------------------------------

    QFrame *frame   = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame);
    m_previewWidget = new PerspectiveWidget(525, 350, frame);
    l->addWidget(m_previewWidget);
    m_previewWidget->setWhatsThis(i18n("This is the perspective transformation operation preview. "
                                       "You can use the mouse for dragging the corner to adjust the "
                                       "perspective transformation area."));
    setToolView(frame);

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout *gridLayout = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1  = new QLabel(i18n("New width:"), m_gboxSettings->plainPage());
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label2   = new QLabel(i18n("New height:"), m_gboxSettings->plainPage());
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    // -------------------------------------------------------------

    KSeparator *line        = new KSeparator (Qt::Horizontal, m_gboxSettings->plainPage());

    QLabel *angleLabel      = new QLabel(i18n("Angles (in degrees):"), m_gboxSettings->plainPage());
    QLabel *label3          = new QLabel(i18n("  Top left:"), m_gboxSettings->plainPage());
    m_topLeftAngleLabel     = new QLabel(m_gboxSettings->plainPage());
    QLabel *label4          = new QLabel(i18n("  Top right:"), m_gboxSettings->plainPage());
    m_topRightAngleLabel    = new QLabel(m_gboxSettings->plainPage());
    QLabel *label5          = new QLabel(i18n("  Bottom left:"), m_gboxSettings->plainPage());
    m_bottomLeftAngleLabel  = new QLabel(m_gboxSettings->plainPage());
    QLabel *label6          = new QLabel(i18n("  Bottom right:"), m_gboxSettings->plainPage());
    m_bottomRightAngleLabel = new QLabel(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    KSeparator *line2         = new KSeparator (Qt::Horizontal, m_gboxSettings->plainPage());
    m_drawWhileMovingCheckBox = new QCheckBox(i18n("Draw preview while moving"), m_gboxSettings->plainPage());
    m_drawGridCheckBox        = new QCheckBox(i18n("Draw grid"), m_gboxSettings->plainPage());
    m_inverseTransformation   = new QCheckBox(i18n("Inverse transformation"), m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *label7 = new QLabel(i18n("Guide color:"), m_gboxSettings->plainPage());
    m_guideColorBt = new KColorButton(QColor(Qt::red), m_gboxSettings->plainPage());
    m_guideColorBt->setWhatsThis(i18n("Set here the color used to draw guides dashed-lines."));

    QLabel *space  = new QLabel(m_gboxSettings->plainPage());
    space->setFixedHeight(m_gboxSettings->spacingHint());

    QLabel *label8 = new QLabel(i18n("Guide width:"), m_gboxSettings->plainPage());
    m_guideSize    = new QSpinBox(m_gboxSettings->plainPage());
    m_guideSize->setRange(1, 5);
    m_guideSize->setSingleStep(1);
    m_guideSize->setWhatsThis(i18n("Set here the width in pixels used to draw guides dashed-lines."));

    gridLayout->setMargin(m_gboxSettings->spacingHint());
    gridLayout->setSpacing(0);
    gridLayout->addWidget(label1,                       0, 0, 1, 1);
    gridLayout->addWidget(m_newWidthLabel,              0, 1, 1, 2);
    gridLayout->addWidget(label2,                       1, 0, 1, 1);
    gridLayout->addWidget(m_newHeightLabel,             1, 1, 1, 2);
    gridLayout->addWidget(line,                         2, 0, 1, 3);
    gridLayout->addWidget(angleLabel,                   3, 0, 1, 3);
    gridLayout->addWidget(label3,                       4, 0, 1, 1);
    gridLayout->addWidget(m_topLeftAngleLabel,          4, 1, 1, 2);
    gridLayout->addWidget(label4,                       5, 0, 1, 1);
    gridLayout->addWidget(m_topRightAngleLabel,         5, 1, 1, 2);
    gridLayout->addWidget(label5,                       6, 0, 1, 1);
    gridLayout->addWidget(m_bottomLeftAngleLabel,       6, 1, 1, 2);
    gridLayout->addWidget(label6,                       7, 0, 1, 1);
    gridLayout->addWidget(m_bottomRightAngleLabel,      7, 1, 1, 2);
    gridLayout->addWidget(line2,                        8, 0, 1, 3);
    gridLayout->addWidget(m_drawWhileMovingCheckBox,    9, 0, 1, 3);
    gridLayout->addWidget(m_drawGridCheckBox,          10, 0, 1, 3);
    gridLayout->addWidget(m_inverseTransformation,     11, 0, 1, 3);
    gridLayout->addWidget(label7,                      12, 0, 1, 1);
    gridLayout->addWidget(m_guideColorBt,              12, 2, 1, 1);
    gridLayout->addWidget(space,                       13, 0, 1, 3);
    gridLayout->addWidget(label8,                      14, 0, 1, 1);
    gridLayout->addWidget(m_guideSize,                 14, 2, 1, 1);
    gridLayout->setColumnStretch(1, 10);
    gridLayout->setRowStretch(15, 10);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_previewWidget, SIGNAL(signalPerspectiveChanged(const QRect&, float, float, float, float)),
            this, SLOT(slotUpdateInfo(const QRect&, float, float, float, float)));

    connect(m_drawWhileMovingCheckBox, SIGNAL(toggled(bool)),
            m_previewWidget, SLOT(slotToggleDrawWhileMoving(bool)));

    connect(m_drawGridCheckBox, SIGNAL(toggled(bool)),
            m_previewWidget, SLOT(slotToggleDrawGrid(bool)));

    connect(m_guideColorBt, SIGNAL(changed(const QColor &)),
            m_previewWidget, SLOT(slotChangeGuideColor(const QColor &)));

    connect(m_guideSize, SIGNAL(valueChanged(int)),
            m_previewWidget, SLOT(slotChangeGuideSize(int)));

    connect(m_inverseTransformation, SIGNAL(toggled(bool)),
            this, SLOT(slotInverseTransformationChanged(bool)));
}

PerspectiveTool::~PerspectiveTool()
{
}

void PerspectiveTool::readSettings()
{
    QColor defaultGuideColor(Qt::red);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("perspective Tool");
    m_drawWhileMovingCheckBox->setChecked(group.readEntry("Draw While Moving", true));
    m_drawGridCheckBox->setChecked(group.readEntry("Draw Grid", false));
    m_guideColorBt->setColor(group.readEntry("Guide Color", defaultGuideColor));
    m_guideSize->setValue(group.readEntry("Guide Width", 1));
    m_inverseTransformation->setChecked(group.readEntry("Inverse Transformation", false));
    m_previewWidget->slotToggleDrawWhileMoving(m_drawWhileMovingCheckBox->isChecked());
    m_previewWidget->slotToggleDrawGrid(m_drawGridCheckBox->isChecked());
    m_previewWidget->slotChangeGuideColor(m_guideColorBt->color());
    m_previewWidget->slotChangeGuideSize(m_guideSize->value());
}

void PerspectiveTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("perspective Tool");
    group.writeEntry("Draw While Moving", m_drawWhileMovingCheckBox->isChecked());
    group.writeEntry("Draw Grid", m_drawGridCheckBox->isChecked());
    group.writeEntry("Guide Color", m_guideColorBt->color());
    group.writeEntry("Guide Width", m_guideSize->value());
    group.writeEntry("Inverse Transformation", m_inverseTransformation->isChecked());
    config->sync();
}

void PerspectiveTool::slotResetSettings()
{
    m_previewWidget->reset();
}

void PerspectiveTool::finalRendering()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    m_previewWidget->applyPerspectiveAdjustment();
    kapp->restoreOverrideCursor();
}

void PerspectiveTool::slotUpdateInfo(const QRect& newSize, float topLeftAngle, float topRightAngle,
                                     float bottomLeftAngle, float bottomRightAngle)
{
    QString temp;
    m_newWidthLabel->setText(temp.setNum(newSize.width()) + i18n(" px"));
    m_newHeightLabel->setText(temp.setNum(newSize.height()) + i18n(" px"));

    m_topLeftAngleLabel->setText(temp.setNum(topLeftAngle, 'f', 1));
    m_topRightAngleLabel->setText(temp.setNum(topRightAngle, 'f', 1));
    m_bottomLeftAngleLabel->setText(temp.setNum(bottomLeftAngle, 'f', 1));
    m_bottomRightAngleLabel->setText(temp.setNum(bottomRightAngle, 'f', 1));
}

void PerspectiveTool::slotInverseTransformationChanged(bool b)
{
    m_drawWhileMovingCheckBox->setEnabled(!b);
    m_drawGridCheckBox->setEnabled(!b);
    m_previewWidget->slotInverseTransformationChanged(b);
}

}  // namespace DigikamPerspectiveImagesPlugin
