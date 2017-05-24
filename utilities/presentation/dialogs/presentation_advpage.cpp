/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * Copyright (C) 2008 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "presentation_advpage.h"

// Local includes

#include "digikam_config.h"
#include "presentationcontainer.h"
#include "presentationwidget.h"

#ifdef HAVE_OPENGL
#   include "presentationgl.h"
#   include "presentationkb.h"
#endif

namespace Digikam
{

PresentationAdvPage::PresentationAdvPage(QWidget* const parent, PresentationContainer* const sharedData)
    : QWidget(parent)
{
    setupUi(this);

    m_sharedData = sharedData;

    connect(m_useMillisecondsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotUseMillisecondsToggled()));

#ifdef HAVE_OPENGL
    m_openGlFullScale->setEnabled(true);
#else
    m_openGlFullScale->setEnabled(false);
#endif
}

PresentationAdvPage::~PresentationAdvPage()
{
}

void PresentationAdvPage::readSettings()
{
    m_enableMouseWheelCheckBox->setChecked(m_sharedData->enableMouseWheel);
    m_useMillisecondsCheckBox->setChecked(m_sharedData->useMilliseconds);
    m_kbDisableFadeCheckBox->setChecked(m_sharedData->kbDisableFadeInOut);
    m_kbDisableCrossfadeCheckBox->setChecked(m_sharedData->kbDisableCrossFade);
    m_openGlFullScale->setChecked(m_sharedData->openGlFullScale);
    m_openGlFullScale->setEnabled(m_sharedData->opengl);

    slotUseMillisecondsToggled();
}

void PresentationAdvPage::saveSettings()
{
#ifdef HAVE_OPENGL
    m_sharedData->openGlFullScale    = m_openGlFullScale->isChecked();
#endif

    m_sharedData->useMilliseconds    = m_useMillisecondsCheckBox->isChecked();
    m_sharedData->enableMouseWheel   = m_enableMouseWheelCheckBox->isChecked();
    m_sharedData->kbDisableFadeInOut = m_kbDisableFadeCheckBox->isChecked();
    m_sharedData->kbDisableCrossFade = m_kbDisableCrossfadeCheckBox->isChecked();
}

void PresentationAdvPage::slotUseMillisecondsToggled()
{
    m_sharedData->useMilliseconds = m_useMillisecondsCheckBox->isChecked();
    emit useMillisecondsToggled();
}

}  // namespace Digikam
