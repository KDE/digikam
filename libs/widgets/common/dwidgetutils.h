/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2014-09-12
 * @brief  Simple helpher widgets collection
 *
 * @author Copyright (C) 2014-2017 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DWIDGET_UTILS_H
#define DWIDGET_UTILS_H

// Qt includes

#include <QUrl>
#include <QString>
#include <QFrame>
#include <QLineEdit>
#include <QSize>
#include <QPixmap>
#include <QFileDialog>
#include <QColor>
#include <QPushButton>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** An Horizontal widget to host children widgets
 */
class DIGIKAM_EXPORT DHBox : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(DHBox)

public:

    explicit DHBox(QWidget* const parent=0);
    virtual ~DHBox();

    void setSpacing(int space);
    void setContentsMargins(const QMargins& margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    void setStretchFactor(QWidget* const widget, int stretch);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:

    DHBox(bool vertical, QWidget* const parent);

    virtual void childEvent(QChildEvent* e);
};

// ------------------------------------------------------------------------------------

/** A Vertical widget to host children widgets
 */
class DIGIKAM_EXPORT DVBox : public DHBox
{
    Q_OBJECT
    Q_DISABLE_COPY(DVBox)

  public:

    explicit DVBox(QWidget* const parent=0);
    virtual ~DVBox();
};

// ------------------------------------------------------------------------------------

/** A widget to chosse a single local file or path.
 *  Use line edit and file dialog properties to customize operation modes.
 */
class DIGIKAM_EXPORT DFileSelector : public DHBox
{
    Q_OBJECT

public:

    explicit DFileSelector(QWidget* const parent=0);
    virtual ~DFileSelector();

    QLineEdit* lineEdit() const;

    void setFileDlgPath(const QString& path);
    QString fileDlgPath() const;

    void setFileDlgMode(QFileDialog::FileMode mode);
    void setFileDlgFilter(const QString& filter);
    void setFileDlgTitle(const QString& title);
    void setFileDlgOptions(QFileDialog::Options opts);

Q_SIGNALS:

    void signalOpenFileDialog();
    void signalUrlSelected(const QUrl&);

private Q_SLOTS:

    void slotBtnClicked();

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------------------------

/** A widget to draw progress wheel indicator over thumbnails.
 */
class DIGIKAM_EXPORT DWorkingPixmap
{
public:

    explicit DWorkingPixmap();
    virtual ~DWorkingPixmap();

    bool    isEmpty()          const;
    QSize   frameSize()        const;
    int     frameCount()       const;
    QPixmap frameAt(int index) const;

private:

    QVector<QPixmap> m_frames;
};

} // namespace Digikam

#endif // DWIDGET_UTILS_H
