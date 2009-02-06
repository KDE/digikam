/*
This file is a part of digiKam project
http://www.digikam.org
Description : image editor printing interface.

Copyright 2009 Angelo Naselli <anaselli@linux.it>
- From  Gwenview code (Aurélien Gâteau)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#ifndef PRINTOPTIONSPAGE_H
#define PRINTOPTIONSPAGE_H

// Qt
#include <QWidget>

// KDE

// Local

namespace Digikam
{


  class PrintOptionsPagePrivate;
  class PrintOptionsPage : public QWidget
  {
      Q_OBJECT
    public:
      enum ScaleMode
      {
        NoScale,
        ScaleToPage,
        ScaleToCustomSize
      };

      // Order should match the content of the unit combbox in the ui file
      enum Unit
      {
        Millimeters,
        Centimeters,
        Inches
      };

      PrintOptionsPage (QWidget *parent, const QSize& imageSize );
      ~PrintOptionsPage();

      Qt::Alignment alignment() const;
      ScaleMode scaleMode() const;
      bool enlargeSmallerImages() const;
      Unit scaleUnit() const;
      double scaleWidth() const;
      double scaleHeight() const;
      bool colorManaged();
      QString inProfilePath();
      QString outputProfilePath();

      void loadConfig();
      void saveConfig();

    private Q_SLOTS:
      void adjustWidthToRatio();
      void adjustHeightToRatio();
      void slotAlertSettings ( bool );
      void slotSetupDlg();

    private:
      PrintOptionsPagePrivate* const d;
  };


} // namespace

#endif /* PRINTOPTIONSPAGE_H */
