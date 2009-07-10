/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC subjects editor.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SUBJECTWIDGET_H
#define SUBJECTWIDGET_H

// Qt includes

#include <QtGui/QButtonGroup>
#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtGui/QWidget>

// KDE includes

#include <kurl.h>

class QCheckBox;
class QLabel;

class KLineEdit;

namespace Digikam
{

class SubjectWidgetPriv;

class SubjectData
{
public:

    SubjectData(const QString& n, const QString& m, const QString& d)
    {
        name   = n;
        matter = m;
        detail = d;
    }

    QString name;         // English and Ascii Name of subject.
    QString matter;       // English and Ascii Matter Name of subject.
    QString detail;       // English and Ascii Detail Name of subject.
};

// --------------------------------------------------------------------------------

class SubjectWidget : public QWidget
{
    Q_OBJECT

public:

    SubjectWidget(QWidget* parent);
    ~SubjectWidget();

    void setSubjectList(const QStringList& list);
    QStringList subjectsList() const;

Q_SIGNALS:

    void signalModified();

protected Q_SLOTS:

    virtual void slotRefChanged();
    virtual void slotEditOptionChanged(int);
    virtual void slotSubjectSelectionChanged();
    virtual void slotAddSubject();
    virtual void slotDelSubject();
    virtual void slotRepSubject();

protected:

    virtual bool loadSubjectCodesFromXML(const KUrl& url);
    virtual QString buildSubject() const;

protected:

    QLabel    *m_note;

    KLineEdit *m_iprEdit;
    KLineEdit *m_refEdit;
    KLineEdit *m_nameEdit;
    KLineEdit *m_matterEdit;
    KLineEdit *m_detailEdit;

private:

    SubjectWidgetPriv* const d;
};

}  // namespace Digikam

#endif // SUBJECTWIDGET_H
