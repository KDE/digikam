/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-7-30
 * Description : Widget for displaying HTML in the backends
 *
 * Copyright (C) 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
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
#ifndef DIGIKAM_WEB_WIDGET_H
#define DIGIKAM_WEB_WIDGET_H

// Qt includes

#include <qwebview.h>

namespace Digikam
{

class WebWidget : public QWebView
{
    Q_OBJECT

public:

  explicit WebWidget(QWidget* const parent = 0);
  ~WebWidget();

Q_SIGNALS:

    void closeView(bool val);


protected:
  void closeEvent(QCloseEvent *event) override;

private:

  class Private;
  Private* const d;
};
}
#endif
