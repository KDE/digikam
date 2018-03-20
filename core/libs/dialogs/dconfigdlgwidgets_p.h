/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Matthias Kretz <kretz at kde dot org>
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

#ifndef DBCONFIGDLG_WIDGETS_P_H
#define DBCONFIGDLG_WIDGETS_P_H

#include "dconfigdlgwidgets.h"
#include "dconfigdlgview_p.h"

// Qt includes

#include <QLabel>
#include <QApplication>

namespace Digikam
{

class DConfigDlgWdgModel;

class DConfigDlgWdgPrivate : public DConfigDlgViewPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgWdg)

protected:

    DConfigDlgWdgPrivate(DConfigDlgWdg* const q);

    DConfigDlgWdgModel* model() const
    {
        return static_cast<DConfigDlgWdgModel*>(DConfigDlgViewPrivate::model);
    }

    void _k_slotCurrentPageChanged(const QModelIndex&, const QModelIndex&);
};

// -----------------------------------------------------------------------------------

class DConfigDlgTitle::Private
{
public:

    Private(DConfigDlgTitle* const parent)
        : q(parent),
          headerLayout(0),
          imageLabel(0),
          textLabel(0),
          commentLabel(0),
          autoHideTimeout(0),
          messageType(InfoMessage)
    {
    }

    QString textStyleSheet() const
    {
        const int fontSize = qRound(QApplication::font().pointSize() * 1.4);

        return QString::fromLatin1("QLabel { font-size: %1pt; color: %2 }")
               .arg(QString::number(fontSize), q->palette().color(QPalette::WindowText).name());
    }

    QString commentStyleSheet() const
    {
        QString styleSheet;

        switch (messageType)
        {
            //FIXME: we need the usability color styles to implement different
            //       yet palette appropriate colours for the different use cases!
            //       also .. should we include an icon here,
            //       perhaps using the imageLabel?
            case InfoMessage:
            case WarningMessage:
            case ErrorMessage:
                styleSheet = QString::fromLatin1("QLabel { color: palette(%1); background: palette(%2); }")
                             .arg(q->palette().color(QPalette::HighlightedText).name())
                             .arg(q->palette().color(QPalette::Highlight).name());
                break;
            case PlainMessage:
            default:
                break;
        }

        return styleSheet;
    }

    /**
     * @brief Get the icon name from the icon type
     * @param type icon type from the enum
     * @return named icon as QString
     */
    QString iconTypeToIconName(DConfigDlgTitle::MessageType type)
    {
        switch (type)
        {
            case DConfigDlgTitle::InfoMessage:
                return QLatin1String("dialog-information");
                break;
            case DConfigDlgTitle::ErrorMessage:
                return QLatin1String("dialog-error");
                break;
            case DConfigDlgTitle::WarningMessage:
                return QLatin1String("dialog-warning");
                break;
            case DConfigDlgTitle::PlainMessage:
                break;
        }

        return QString();
    }

    void _k_timeoutFinished()
    {
        q->setVisible(false);
    }

public:

    DConfigDlgTitle* q;
    QGridLayout*     headerLayout;
    QLabel*          imageLabel;
    QLabel*          textLabel;
    QLabel*          commentLabel;
    int              autoHideTimeout;
    MessageType      messageType;
};

}  // namespace Digikam

#endif // DBCONFIGDLG_WIDGETS_P_H
