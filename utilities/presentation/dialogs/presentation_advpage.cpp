/* ============================================================
 *
 * This file is a part of kipi-plugins project
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

#include "presentationcontainer.h"
#include "presentation.h"

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

    connect(m_cacheCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotCacheToggled()));
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
    m_cacheCheckBox->setChecked(m_sharedData->enableCache);

    slotUseMillisecondsToggled();
    slotCacheToggled();
}

void PresentationAdvPage::saveSettings()
{
    m_sharedData->useMilliseconds    = m_useMillisecondsCheckBox->isChecked();
    m_sharedData->enableMouseWheel   = m_enableMouseWheelCheckBox->isChecked();
    m_sharedData->kbDisableFadeInOut = m_kbDisableFadeCheckBox->isChecked();
    m_sharedData->kbDisableCrossFade = m_kbDisableCrossfadeCheckBox->isChecked();
    m_sharedData->cacheSize          = m_cacheSizeSpinBox->value();
    m_sharedData->enableCache        = m_cacheCheckBox->isChecked();
}

// --- Slots

void PresentationAdvPage::slotUseMillisecondsToggled()
{
    m_sharedData->useMilliseconds = m_useMillisecondsCheckBox->isChecked();
    emit useMillisecondsToggled();
}

void PresentationAdvPage::slotCacheToggled()
{
    bool isEnabled = m_cacheCheckBox->isChecked();

    m_cacheSizeLabel1->setEnabled(isEnabled);
    m_cacheSizeLabel2->setEnabled(isEnabled);
    m_cacheSizeSpinBox->setEnabled(isEnabled);
}

}  // namespace Digikam
