/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to change image perspective.
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

#include <qcheckbox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
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
#include <kseparator.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "editortoolsettings.h"
#include "perspectivewidget.h"
#include "perspectivetool.h"
#include "perspectivetool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamPerspectiveImagesPlugin
{

PerspectiveTool::PerspectiveTool(QObject* parent)
               : EditorTool(parent)
{
    setName("perspective");
    setToolName(i18n("Perspective"));
    setToolIcon(SmallIcon("perspective"));

    // -------------------------------------------------------------

    QFrame *frame   = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new PerspectiveWidget(525, 350, frame);
    l->addWidget(m_previewWidget);
    QWhatsThis::add(m_previewWidget, i18n("<p>This is the perspective transformation operation preview. "
                                          "You can use the mouse for dragging the corner to adjust the "
                                          "perspective transformation area."));
    setToolView(frame);

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout *gridLayout = new QGridLayout( m_gboxSettings->plainPage(), 13, 2);

    QLabel *label1  = new QLabel(i18n("New width:"), m_gboxSettings->plainPage());
    m_newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newWidthLabel->setAlignment( AlignBottom | AlignRight );

    QLabel *label2   = new QLabel(i18n("New height:"), m_gboxSettings->plainPage());
    m_newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"), m_gboxSettings->plainPage());
    m_newHeightLabel->setAlignment( AlignBottom | AlignRight );

    // -------------------------------------------------------------

    KSeparator *line = new KSeparator (Horizontal, m_gboxSettings->plainPage());

    QLabel *angleLabel = new QLabel(i18n("Angles (in degrees):"), m_gboxSettings->plainPage());
    QLabel *label3 = new QLabel(i18n("  Top left:"), m_gboxSettings->plainPage());
    m_topLeftAngleLabel = new QLabel(m_gboxSettings->plainPage());
    QLabel *label4 = new QLabel(i18n("  Top right:"), m_gboxSettings->plainPage());
    m_topRightAngleLabel = new QLabel(m_gboxSettings->plainPage());
    QLabel *label5 = new QLabel(i18n("  Bottom left:"), m_gboxSettings->plainPage());
    m_bottomLeftAngleLabel = new QLabel(m_gboxSettings->plainPage());
    QLabel *label6 = new QLabel(i18n("  Bottom right:"), m_gboxSettings->plainPage());
    m_bottomRightAngleLabel = new QLabel(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    KSeparator *line2 = new KSeparator (Horizontal, m_gboxSettings->plainPage());

    m_drawWhileMovingCheckBox = new QCheckBox(i18n("Draw preview while moving"), m_gboxSettings->plainPage());
    gridLayout->addMultiCellWidget(line2, 8, 8, 0, 2);
    gridLayout->addMultiCellWidget(m_drawWhileMovingCheckBox, 9, 9, 0, 2);

    m_drawGridCheckBox = new QCheckBox(i18n("Draw grid"), m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *label7 = new QLabel(i18n("Guide color:"), m_gboxSettings->plainPage());
    m_guideColorBt = new KColorButton( QColor( Qt::red ), m_gboxSettings->plainPage() );
    QWhatsThis::add( m_guideColorBt, i18n("<p>Set here the color used to draw guides dashed-lines."));
    gridLayout->addMultiCellWidget(label7, 11, 11, 0, 0);
    gridLayout->addMultiCellWidget(m_guideColorBt, 11, 11, 2, 2);

    QLabel *label8 = new QLabel(i18n("Guide width:"), m_gboxSettings->plainPage());
    m_guideSize    = new RIntNumInput(m_gboxSettings->plainPage());
    m_guideSize->input()->setRange(1, 5, 1, false);
    m_guideSize->setDefaultValue(1);
    QWhatsThis::add( m_guideSize, i18n("<p>Set here the width in pixels used to draw guides dashed-lines."));

    gridLayout->addMultiCellWidget(label1,                  0, 0, 0, 0);
    gridLayout->addMultiCellWidget(m_newWidthLabel,         0, 0, 1, 2);
    gridLayout->addMultiCellWidget(label2,                  1, 1, 0, 0);
    gridLayout->addMultiCellWidget(m_newHeightLabel,        1, 1, 1, 2);
    gridLayout->addMultiCellWidget(line,                    2, 2, 0, 2);
    gridLayout->addMultiCellWidget(angleLabel,              3, 3, 0, 2);
    gridLayout->addMultiCellWidget(label3,                  4, 4, 0, 0);
    gridLayout->addMultiCellWidget(m_topLeftAngleLabel,     4, 4, 1, 2);
    gridLayout->addMultiCellWidget(label4,                  5, 5, 0, 0);
    gridLayout->addMultiCellWidget(m_topRightAngleLabel,    5, 5, 1, 2);
    gridLayout->addMultiCellWidget(label5,                  6, 6, 0, 0);
    gridLayout->addMultiCellWidget(m_bottomLeftAngleLabel,  6, 6, 1, 2);
    gridLayout->addMultiCellWidget(label6,                  7, 7, 0, 0);
    gridLayout->addMultiCellWidget(m_bottomRightAngleLabel, 7, 7, 1, 2);
    gridLayout->addMultiCellWidget(m_drawGridCheckBox,      10, 10, 0, 2);
    gridLayout->addMultiCellWidget(label8,                  12, 12, 0, 0);
    gridLayout->addMultiCellWidget(m_guideSize,             12, 12, 2, 2);
    gridLayout->setColStretch(1, 10);
    gridLayout->setRowStretch(13, 10);
    gridLayout->setMargin(m_gboxSettings->spacingHint());
    gridLayout->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_previewWidget, SIGNAL(signalPerspectiveChanged(QRect, float, float, float, float)),
            this, SLOT(slotUpdateInfo(QRect, float, float, float, float)));

    connect(m_drawWhileMovingCheckBox, SIGNAL(toggled(bool)),
            m_previewWidget, SLOT(slotToggleDrawWhileMoving(bool)));

    connect(m_drawGridCheckBox, SIGNAL(toggled(bool)),
            m_previewWidget, SLOT(slotToggleDrawGrid(bool)));

    connect(m_guideColorBt, SIGNAL(changed(const QColor&)),
            m_previewWidget, SLOT(slotChangeGuideColor(const QColor&)));

    connect(m_guideSize, SIGNAL(valueChanged(int)),
            m_previewWidget, SLOT(slotChangeGuideSize(int)));
}

PerspectiveTool::~PerspectiveTool()
{
}

void PerspectiveTool::readSettings()
{
    QColor defaultGuideColor(Qt::red);
    KConfig *config = kapp->config();
    config->setGroup("perspective Tool");
    m_drawWhileMovingCheckBox->setChecked(config->readBoolEntry("Draw While Moving", true));
    m_drawGridCheckBox->setChecked(config->readBoolEntry("Draw Grid", false));
    m_guideColorBt->setColor(config->readColorEntry("Guide Color", &defaultGuideColor));
    m_guideSize->setValue(config->readNumEntry("Guide Width", m_guideSize->defaultValue()));
    m_previewWidget->slotToggleDrawWhileMoving(m_drawWhileMovingCheckBox->isChecked());
    m_previewWidget->slotToggleDrawGrid(m_drawGridCheckBox->isChecked());
    m_previewWidget->slotChangeGuideColor(m_guideColorBt->color());
    m_previewWidget->slotChangeGuideSize(m_guideSize->value());
}

void PerspectiveTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("perspective Tool");
    config->writeEntry("Draw While Moving", m_drawWhileMovingCheckBox->isChecked());
    config->writeEntry("Draw Grid", m_drawGridCheckBox->isChecked());
    config->writeEntry("Guide Color", m_guideColorBt->color());
    config->writeEntry("Guide Width", m_guideSize->value());
    config->sync();
}

void PerspectiveTool::slotResetSettings()
{
    m_previewWidget->reset();
}

void PerspectiveTool::finalRendering()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_previewWidget->applyPerspectiveAdjustment();
    kapp->restoreOverrideCursor();
}

void PerspectiveTool::slotUpdateInfo(QRect newSize, float topLeftAngle, float topRightAngle,
                                             float bottomLeftAngle, float bottomRightAngle)
{
    QString temp;
    m_newWidthLabel->setText(temp.setNum( newSize.width())   + i18n(" px") );
    m_newHeightLabel->setText(temp.setNum( newSize.height()) + i18n(" px") );

    m_topLeftAngleLabel->setText(temp.setNum( topLeftAngle, 'f', 1 ));
    m_topRightAngleLabel->setText(temp.setNum( topRightAngle, 'f', 1 ));
    m_bottomLeftAngleLabel->setText(temp.setNum( bottomLeftAngle, 'f', 1 ));
    m_bottomRightAngleLabel->setText(temp.setNum( bottomRightAngle, 'f', 1 ));
}

}  // NameSpace DigikamPerspectiveImagesPlugin
