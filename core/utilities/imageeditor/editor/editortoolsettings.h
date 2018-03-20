/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : Editor tool settings template box
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2014 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef EDITORTOOLSETTINGS_H
#define EDITORTOOLSETTINGS_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"
#include "digikam_debug.h"
#include "histogrambox.h"
#include "digikam_globals.h"

class QPushButton;
class QPixmap;
class QString;
class QIcon;

namespace Digikam
{
class HistogramBox;

class DIGIKAM_EXPORT EditorToolSettings : public QScrollArea
{
    Q_OBJECT

public:

    enum ButtonCode
    {
        Default = 0x00000001,
        Try     = 0x00000002,
        Ok      = 0x00000004,
        Cancel  = 0x00000008,
        SaveAs  = 0x00000010,
        Load    = 0x00000020
    };
    Q_DECLARE_FLAGS(Buttons, ButtonCode)

    enum ToolCode
    {
        NoTool     = 0x00000000,
        ColorGuide = 0x00000001,
        Histogram  = 0x00000002
    };
    Q_DECLARE_FLAGS(Tools, ToolCode)

public:

    explicit EditorToolSettings(QWidget* const parent = 0);
    virtual ~EditorToolSettings();

    void setButtons(Buttons buttonMask);
    void setTools(Tools toolMask);
    void setHistogramType(HistogramBoxType type);
    void setToolIcon(const QIcon& pixmap);
    void setToolName(const QString& name);

    int marginHint();
    int spacingHint();

    QWidget*      plainPage()    const;
    HistogramBox* histogramBox() const;

    QColor guideColor() const;
    void setGuideColor(const QColor& color);

    int guideSize() const;
    void setGuideSize(int size);

    QPushButton* button(int buttonCode) const;
    void enableButton(int buttonCode, bool state);

    virtual QSize minimumSizeHint() const;
    virtual void setBusy(bool)   {};
    virtual void writeSettings() {};
    virtual void readSettings()  {};
    virtual void resetSettings() {};

Q_SIGNALS:

    void signalOkClicked();
    void signalCancelClicked();
    void signalTryClicked();
    void signalDefaultClicked();
    void signalSaveAsClicked();
    void signalLoadClicked();
    void signalColorGuideChanged();
    void signalChannelChanged();
    void signalScaleChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::EditorToolSettings::Buttons)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::EditorToolSettings::Tools)

#endif // EDITORTOOLSETTINGS_H
