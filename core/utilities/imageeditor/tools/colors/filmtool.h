/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : film color negative inverter tool
 *
 * Copyright (C) 2012 by Matthias Welwarsky <matthias at welwarsky dot de>
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

#ifndef FILMTOOL_H_
#define FILMTOOL_H_

// Local includes

#include "editortool.h"
#include "dcolor.h"

class QListWidgetItem;

namespace Digikam
{

class FilmProfile;

class FilmTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit FilmTool(QObject* const parent);
    ~FilmTool();

private Q_SLOTS:

    void slotResetSettings();
    void slotScaleChanged();
    void slotChannelChanged();
    void slotAdjustSliders();
    void slotFilmItemActivated(QListWidgetItem* item);
    void slotExposureChanged(double val);
    void slotGammaInputChanged(double val);
    void slotColorSelectedFromTarget(const Digikam::DColor& color, const QPoint& p);
    void slotPickerColorButtonActived(bool checked);
    void slotResetWhitePoint();
    void slotColorBalanceStateChanged(int);
    void slotAutoWhitePoint(void);

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();

    void gammaInputChanged(double val);
    void setLevelsFromFilm();
    bool eventFilter(QObject*, QEvent*);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* FILMTOOL_H_ */
