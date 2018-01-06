/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-30
 * Description : a widget to display an image histogram and its control widgets
 *
 * Copyright (C) 2008-2009 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QWidget>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"
#include "digikam_globals.h"

class QColor;

namespace Digikam
{

class HistogramWidget;

class DIGIKAM_EXPORT HistogramBox : public QWidget
{
    Q_OBJECT

public:

    explicit HistogramBox(QWidget* const parent = 0, HistogramBoxType type = Digikam::LRGB, bool selectMode = false);
    ~HistogramBox();

    void setHistogramType(HistogramBoxType type);
    void setHistogramMargin(int);

    void setGradientColors(const QColor& from, const QColor& to);
    void setGradientVisible(bool visible);

    ChannelType channel() const;
    void setChannelEnabled(bool enabled);

    void setStatisticsVisible(bool b);

    HistogramScale scale() const;

    HistogramWidget* histogram() const;

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

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* HISTOGRAMBOX_H */
