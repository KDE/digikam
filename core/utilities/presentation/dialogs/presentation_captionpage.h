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

#ifndef PRESENTATION_CAPTION_PAGE_H
#define PRESENTATION_CAPTION_PAGE_H

// Local includes

#include "ui_presentation_captionpage.h"

namespace Digikam
{

class PresentationContainer;

class PresentationCaptionPage : public QWidget, Ui::PresentationCaptionPage
{
    Q_OBJECT

public:

    PresentationCaptionPage(QWidget* const parent, PresentationContainer* const sharedData);
    ~PresentationCaptionPage();

    void readSettings();
    void saveSettings();

private Q_SLOTS:

    void slotCommentsFontColorChanged();
    void slotCommentsBgColorChanged();
    void slotOpenFontDialog();

private:

    PresentationContainer* m_sharedData;
};

} // namespace Digikam

#endif // PRESENTATION_CAPTION_PAGE_H
