/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SMUG_WIDGET_H
#define SMUG_WIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "dinfointerface.h"
#include "dprogresswdg.h"
#include "dimageslist.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QRadioButton;
class QPushButton;
class QComboBox;
class QLineEdit;

namespace Digikam
{

class SmugWidget : public QWidget
{
    Q_OBJECT

public:

    explicit SmugWidget(QWidget* const parent,
                        DInfoInterface* const iface,
                        bool import);
    ~SmugWidget();

public:

    void updateLabels(const QString& email = QString(),
                      const QString& name = QString(),
                      const QString& nick = QString());

    bool isAnonymous()           const;
    void setAnonymous(bool checked);

    QString getNickName()        const;
    void    setNickName(const QString& nick);

    QString getSitePassword()    const;
    QString getAlbumPassword()   const;
    QString getDestinationPath() const;

    DImagesList* imagesList()    const;
    DProgressWdg* progressBar()  const;

Q_SIGNALS:

    void signalUserChangeRequest(bool anonymous);

private Q_SLOTS:

    void slotAnonymousToggled(bool checked);
    void slotChangeUserClicked();
    void slotResizeChecked();

private:

    QLabel*         m_headerLbl;
    QLabel*         m_userNameLbl;
    QLabel*         m_userName;
    QLabel*         m_emailLbl;
    QLabel*         m_email;
    QLabel*         m_nickNameLbl;
    QLabel*         m_sitePasswordLbl;
    QLabel*         m_albumPasswordLbl;

    QRadioButton*   m_anonymousRBtn;
    QRadioButton*   m_accountRBtn;

    QCheckBox*      m_resizeChB;

    QSpinBox*       m_dimensionSpB;
    QSpinBox*       m_imageQualitySpB;

    QComboBox*      m_albumsCoB;

    QPushButton*    m_newAlbumBtn;
    QPushButton*    m_reloadAlbumsBtn;
    QPushButton*    m_changeUserBtn;

    QLineEdit*      m_albumPasswordEdt;
    QLineEdit*      m_nickNameEdt;
    QLineEdit*      m_sitePasswordEdt;

    DInfoInterface* m_iface;
    DProgressWdg*   m_progressBar;
    DImagesList*    m_imgList;
    QWidget*        m_uploadWidget;

    friend class SmugWindow;
};

} // namespace Digikam

#endif // SMUG_WIDGET_H
