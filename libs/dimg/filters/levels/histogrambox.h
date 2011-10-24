/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-30
 * Description : a widget to display an image histogram and its control widgets
 *
 * Copyright (C) 2008-2009 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#ifndef HISTOGRAMBOX_H
#define HISTOGRAMBOX_H

// Qt includes

#include <QtGui/QWidget>

// KDE includes

#include <kdebug.h>

// Local includes

#include "digikam_export.h"
#include "globals.h"

class QColor;

namespace Digikam
{

class HistogramWidget;
class HistogramBoxPriv;

class DIGIKAM_EXPORT HistogramBox : public QWidget
{
    Q_OBJECT

public:

    explicit HistogramBox(QWidget* parent = 0, HistogramBoxType type = Digikam::LRGB, bool selectMode = false);
    ~HistogramBox();

    void setHistogramType(HistogramBoxType type);
    void setHistogramMargin(int);

    void setGradientColors(const QColor& from, const QColor& to);
    void setGradientVisible(bool visible);

    HistogramWidget* histogram() const;

    ChannelType channel() const;
    void setChannelEnabled(bool enabled);

    HistogramScale scale() const;

Q_SIGNALS:

    void signalChannelChanged(ChannelType channel);
    void signalScaleChanged(HistogramScale scale);

public Q_SLOTS:

    void setChannel(ChannelType channel);
    void setScale(HistogramScale scale);

protected Q_SLOTS:

    void slotChannelChanged();
    void slotScaleChanged();

private:

    HistogramBoxPriv* const d;
};

} // namespace Digikam

#endif /* HISTOGRAMBOX_H */
