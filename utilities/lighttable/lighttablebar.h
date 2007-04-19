/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-04-11
 * Description : light table thumbs bar
 *
 * Copyright 2007 by Gilles Caulier
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

#ifndef LIGHTTABLEBAR_H
#define LIGHTTABLEBAR_H

// Local includes.

#include "thumbbar.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageInfo;
class LightTableBarItem;
class LightTableBarPriv;
class LightTableBarItemPriv;

class DIGIKAM_EXPORT LightTableBar : public ThumbBarView
{
    Q_OBJECT

public:

    LightTableBar(QWidget* parent, int orientation=Vertical, bool exifRotate=false);
    ~LightTableBar();

    ImageInfo* currentItemImageInfo() const;

    /** Read tool tip settings from Album Settings instance */
    void readToolTipSettings();

signals:

    void signalLightTableBarItemSelected(ImageInfo*);
    
private slots:

    void slotItemSelected(ThumbBarItem* i);

private:

    void setupToolTip();

    friend class LightTableBarItem;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT LightTableBarItem : public ThumbBarItem
{
public:

    LightTableBarItem(LightTableBar* view, ImageInfo* item);
    ~LightTableBarItem();

    ImageInfo* info();
    
private:

    ImageInfo   *m_info;

    friend class LightTableBar;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT LightTableBarToolTip : public ThumbBarToolTip
{
public:

    LightTableBarToolTip(ThumbBarView *parent);

private:

    QString tipContent(ThumbBarItem* item);
};

}  // NameSpace Digikam

#endif /* LIGHTTABLEBAR_H */
