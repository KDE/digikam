/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * Copyright (C) 2008      by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "presentation_captionpage.h"

// Qt includes

#include <QFont>
#include <QFontDialog>
#include <QPalette>

// Local includes

#include "presentationcontainer.h"

namespace Digikam
{

PresentationCaptionPage::PresentationCaptionPage( QWidget* const parent, PresentationContainer* const sharedData)
    : QWidget(parent)
{
    setupUi(this);
    m_sharedData = sharedData;
    m_fontSampleLbl->setText(i18n("This is a comment sample..."));
    m_fontSampleLbl->setAutoFillBackground(true);
}

PresentationCaptionPage::~PresentationCaptionPage()
{
}

void PresentationCaptionPage::readSettings()
{
    connect(m_commentsFontColor, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotCommentsFontColorChanged()));

    connect(m_commentsBgColor, SIGNAL(signalColorSelected(QColor)),
            this, SLOT(slotCommentsBgColorChanged()));

    connect(m_fontselectBtn, SIGNAL(clicked()),
            this, SLOT(slotOpenFontDialog()));

    m_commentsLinesLengthSpinBox->setValue(m_sharedData->commentsLinesLength);
    m_commentsFontColor->setColor(QColor(m_sharedData->commentsFontColor));
    m_commentsBgColor->setColor(QColor(m_sharedData->commentsBgColor));
    m_commentsDrawOutlineCheckBox->setChecked(m_sharedData->commentsDrawOutline);
    m_fontSampleLbl->setFont(*(m_sharedData->captionFont));
    m_commentsBgTransparency->setValue(m_sharedData->bgOpacity);

    slotCommentsBgColorChanged();
    slotCommentsFontColorChanged();
}

void PresentationCaptionPage::saveSettings()
{
    delete m_sharedData->captionFont;
    m_sharedData->captionFont         = new QFont(m_fontSampleLbl->font());
    QColor fontColor                  = QColor(m_commentsFontColor->color());
    m_sharedData->commentsFontColor   = fontColor.rgb();
    QColor bgColor                    = QColor(m_commentsBgColor->color());
    m_sharedData->commentsBgColor     = bgColor.rgb();
    m_sharedData->commentsDrawOutline = m_commentsDrawOutlineCheckBox->isChecked();
    m_sharedData->commentsLinesLength = m_commentsLinesLengthSpinBox->value();
    m_sharedData->bgOpacity           = m_commentsBgTransparency->value();
}

void PresentationCaptionPage::slotCommentsBgColorChanged()
{
    QPalette palette = m_fontSampleLbl->palette();
    palette.setColor(m_fontSampleLbl->backgroundRole(), m_commentsBgColor->color());
    m_fontSampleLbl->setPalette(palette);
}

void PresentationCaptionPage::slotCommentsFontColorChanged()
{
    QPalette palette = m_fontSampleLbl->palette();
    palette.setColor(m_fontSampleLbl->foregroundRole(), m_commentsFontColor->color());
    m_fontSampleLbl->setPalette(palette);
}

void PresentationCaptionPage::slotOpenFontDialog()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, *(m_sharedData->captionFont), this);

    if (ok)
    {
        m_fontSampleLbl->setFont(f);
    }
}

}  // namespace Digikam
