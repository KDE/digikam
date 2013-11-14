/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2010-11-15
 * Description : a kipi plugin to export images to Yandex.Fotki web service
 *
 * Copyright (C) 2010 by Roman Tsisyk <roman at tsisyk dot com>
 *
 * GUI based on PicasaWeb KIPI Plugin
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Luka Renko <lure at kubuntu dot org>
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

#ifndef ET_DIALOG_H
#define ET_DIALOG_H

#include "kdialog.h"
#include "etwidget.h"

class ETDialog : public KDialog
{
    Q_OBJECT

public:

    ETDialog(QWidget* parent, const QList<KUrl>& images, const QString& tool = QString());
    ~ETDialog();

    virtual void slotButtonClicked(int button);

    Q_SIGNAL void processFinished();

private Q_SLOTS:

    void onStarted();
    void onError(const QString& title, const QString& text);

private:
    ETWidget*       m_etwidget;
    QList<KUrl>     m_images;
};


#endif /* ET_DIALOG_H */
