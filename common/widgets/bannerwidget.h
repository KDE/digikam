/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-07-07
 * Description : a nice banner widget.
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

#ifndef BANNERWIDGET_H
#define BANNERWIDGET_H

// Qt includes.

#include <qframe.h>
#include <qstring.h>

namespace DigikamImagePlugins
{

class BannerWidget : public QFrame
{
Q_OBJECT

public:

    BannerWidget(QWidget *parent=0, QString title=QString::null);
    ~BannerWidget();

private slots:        

    void processURL(const QString& url);
};

}  // namespace DigikamImagePlugins

#endif /* BANNERWIDGET_H */
