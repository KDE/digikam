/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RAJCE_COMMAND_H
#define RAJCE_COMMAND_H

// Qt includes

#include <QImage>
#include <QObject>
#include <QString>
#include <QXmlQuery>

// Local includes

#include "rajcesession.h"
#include "rajcempform.h"

namespace Digikam
{

class RajceCommand
{
public:

    explicit RajceCommand(const QString& name, RajceCommandType commandType);
    virtual ~RajceCommand();

public:

    void processResponse(const QString& response, RajceSession& state);

    QString            getXml()      const;
    RajceCommandType   commandType() const;
    virtual QByteArray encode()      const;
    virtual QString    contentType() const;

protected:

    virtual void parseResponse(QXmlQuery& query, RajceSession& state) = 0;
    virtual void cleanUpOnError(RajceSession& state)                  = 0;

    // Allow modification in const methods for lazy init to be possible
    QMap<QString, QString>& parameters() const;

    // Aooend additional xml after the "parameters"
    virtual QString additionalXml() const;

private:

    bool parseErrorFromQuery(QXmlQuery& query, RajceSession& state);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------------------

class LoginCommand : public RajceCommand
{
public:

    explicit LoginCommand(const QString& username, const QString& password);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) Q_DECL_OVERRIDE;
    void cleanUpOnError(RajceSession& state)                     Q_DECL_OVERRIDE;
};

// -----------------------------------------------------------------------

class OpenAlbumCommand : public RajceCommand
{
public:

    explicit OpenAlbumCommand(unsigned albumId, const RajceSession& state);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) Q_DECL_OVERRIDE;
    void cleanUpOnError(RajceSession& state)                     Q_DECL_OVERRIDE;
};

// -----------------------------------------------------------------------

class CreateAlbumCommand : public RajceCommand
{
public:

    explicit CreateAlbumCommand(const QString& name,
                                const QString& description,
                                bool  visible,
                                const RajceSession& state);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) Q_DECL_OVERRIDE;
    void cleanUpOnError(RajceSession& state)                     Q_DECL_OVERRIDE;
};

// -----------------------------------------------------------------------

class CloseAlbumCommand : public RajceCommand
{
public:

    explicit CloseAlbumCommand(const RajceSession& state);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) Q_DECL_OVERRIDE;
    void cleanUpOnError(RajceSession& state)                     Q_DECL_OVERRIDE;
};

// -----------------------------------------------------------------------

class AlbumListCommand : public RajceCommand
{
public:

    explicit AlbumListCommand(const RajceSession&);

protected:

    void parseResponse(QXmlQuery& response, RajceSession& state) Q_DECL_OVERRIDE;
    void cleanUpOnError(RajceSession& state)                     Q_DECL_OVERRIDE;
};

// -----------------------------------------------------------------------

class AddPhotoCommand : public RajceCommand
{
public:

    explicit AddPhotoCommand(const    QString& tmpDir,
                             const    QString& path,
                             unsigned dimension,
                             int      jpgQuality,
                             const    RajceSession& state);
    virtual ~AddPhotoCommand();

public:

    QByteArray encode()      const Q_DECL_OVERRIDE;
    QString    contentType() const Q_DECL_OVERRIDE;

protected:

    void    cleanUpOnError(RajceSession& state)                  Q_DECL_OVERRIDE;
    void    parseResponse(QXmlQuery& query, RajceSession& state) Q_DECL_OVERRIDE;
    QString additionalXml() const                                Q_DECL_OVERRIDE;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // RAJCE_COMMAND_H
