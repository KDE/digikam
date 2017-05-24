/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-10-05
 * Description : a presentation tool.
 *
 * Copyright (C) 2008 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 *
 * Partially based on Renchi Raju's PresentationCtrlWidget class.
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

#ifndef PRENSENTATION_CTRL_WIDGET_H
#define PRENSENTATION_CTRL_WIDGET_H

// Qt includes

#include <QWidget>
#include <QKeyEvent>

// Local includes

#include "ui_presentationctrlwidget.h"

namespace Digikam
{

class PresentationCtrlWidget : public QWidget, public Ui::PresentationCtrlWidget
{
    Q_OBJECT

public:

    explicit PresentationCtrlWidget(QWidget* const parent);
    ~PresentationCtrlWidget();

    bool canHide()  const;
    bool isPaused() const;
    void setPaused(bool val);

    void setEnabledPlay(bool val);
    void setEnabledNext(bool val);
    void setEnabledPrev(bool val);

Q_SIGNALS:

    void signalNext();
    void signalPrev();
    void signalClose();
    void signalPlay();
    void signalPause();

protected:

    void keyPressEvent(QKeyEvent* event);

private Q_SLOTS:

    void slotPlayButtonToggled();
    void slotNexPrevClicked();

private:

    bool         m_canHide;

    friend class PresentationWidget;
    friend class PresentationGL;
};

} // namespace Digikam

#endif // PRENSENTATION_CTRL_WIDGET_H
