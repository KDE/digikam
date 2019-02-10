/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * Copyright (C) 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_TWITTER_MPFORM_H
#define DIGIKAM_TWITTER_MPFORM_H

// Qt includes

#include <QByteArray>
#include <QString>
#include <QList>

#define MAX_MEDIA_SIZE 1048576

namespace DigikamGenericTwitterPlugin
{

class TwMPForm
{

public:

    explicit TwMPForm();
    ~TwMPForm();

    void reset();
    QByteArray fileHeader(const QString& imgPath);
    bool addFile(const QString& imgPath, bool fragmented = false);
    QByteArray createPair(const QByteArray& name, const QByteArray& value);
    bool addPair(const QByteArray& pair);
    void finish();
    QByteArray border();

    int numberOfChunks() const;
    QString contentType() const;
    QByteArray formData() const;
    QByteArray getChunk(int index) const;

private:

    void formChunks(const QByteArray& data);

private:

    QByteArray m_buffer;
    QByteArray m_boundary;
    QList<QByteArray> m_chunks;
};

} // namespace DigikamGenericTwitterPlugin

#endif // DIGIKAM_TWITTER_MPFORM_H
