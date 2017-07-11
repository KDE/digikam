/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-04
 * Description : A label to show transition preview
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TRANSITION_PREVIEW_H
#define TRANSITION_PREVIEW_H

// Qt includes

#include <QLabel>
#include <QString>
#include <QList>
#include <QUrl>

// Local includes

#include "transitionmngr.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TransitionPreview : public QLabel
{
    Q_OBJECT

public:

    explicit TransitionPreview(QWidget* const parent=0);
    ~TransitionPreview();

    void setImagesList(const QList<QUrl>& images);

    void startPreview(TransitionMngr::TransType eff);
    void stopPreview();

private Q_SLOTS:

    void slotProgressTransition();
    void slotRestart();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TRANSITION_PREVIEW_H
