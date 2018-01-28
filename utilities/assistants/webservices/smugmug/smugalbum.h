/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-07
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

#ifndef SMUG_ALBUM_H
#define SMUG_ALBUM_H

// Qt includes

#include <QDialog>

class QGroupBox;
class QRadioButton;
class QComboBox;
class QLineEdit;
class QTextEdit;

namespace Digikam
{

class SmugAlbum;

class SmugNewAlbum : public QDialog
{
    Q_OBJECT

public:

    explicit SmugNewAlbum(QWidget* const parent);
    ~SmugNewAlbum();

    void getAlbumProperties(SmugAlbum& album);

private:

    QLineEdit*    m_titleEdt;
    QTextEdit*    m_descEdt;
    QComboBox*    m_categCoB;
    QComboBox*    m_subCategCoB;
    QComboBox*    m_templateCoB;

    QGroupBox*    m_privBox;
    QLineEdit*    m_passwdEdt;
    QLineEdit*    m_hintEdt;
    QRadioButton* m_publicRBtn;
    QRadioButton* m_unlistedRBtn;

    friend class SmugWindow;
};

} // namespace Digikam

#endif // SMUG_ALBUM_H
