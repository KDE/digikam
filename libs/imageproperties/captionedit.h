/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : caption editor
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAPTION_EDIT_H
#define CAPTION_EDIT_H

// Qt includes

#include <QWidget>
#include <QString>
#include <QDateTime>
#include <QTextEdit>

// Local includes

#include "dlayoutbox.h"
#include "captionvalues.h"

namespace Digikam
{

class CaptionEdit : public DVBox
{
    Q_OBJECT

public:

    explicit CaptionEdit(QWidget* const parent);
    ~CaptionEdit();

    void setValues(const CaptionsMap& values);
    CaptionsMap& values() const;

    void setCurrentLanguageCode(const QString& lang);
    QString currentLanguageCode() const;

    void reset();

    QTextEdit* textEdit() const;

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSelectionChanged(const QString&);
    void slotCaptionModified(const QString&, const QString&);
    void slotAddValue(const QString&, const QString&);
    void slotDeleteValue(const QString&);
    void slotAuthorChanged(const QString&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // CAPTION_EDIT_H
