/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-21
 * Description : setup tab for showfoto slideshow options.
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef SETUPSLIDESHOW_H
#define SETUPSLIDESHOW_H

// Qt includes.

#include <qwidget.h>

class QCheckBox;

class KIntNumInput;

namespace ShowFoto
{

class SetupSlideShow : public QWidget
{
    Q_OBJECT
    
public:

    SetupSlideShow(QWidget* parent = 0);
    ~SetupSlideShow();

    void applySettings();

private:

    QCheckBox    *m_startWithCurrent;
    QCheckBox    *m_loopMode;
    QCheckBox    *m_fullScreenMode;
    
    KIntNumInput *m_delayInput;
    
    void readSettings();
};

}   // namespace ShowFoto

#endif /* SETUPSLIDESHOW_H */
