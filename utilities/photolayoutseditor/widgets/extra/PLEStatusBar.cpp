/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "PLEStatusBar.h"

#include <QDebug>
#include <QLabel>

using namespace PhotoLayoutsEditor;

PLEStatusBar::PLEStatusBar(QWidget * parent) :
    QStatusBar(parent)
{
    addWidget(new QLabel(this), 1);
    m_pb = new QProgressBar(this);
    m_pb->setMinimum(0);
    m_pb->setMaximum(0);
    addPermanentWidget(m_pb);
    stopBusyIndicator();
}

void PLEStatusBar::runBusyIndicator()
{
    m_pb->show();
}

void PLEStatusBar::stopBusyIndicator()
{
    m_pb->hide();
}
