/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect binary program and version
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef DBINARYIFACE_H
#define DBINARYIFACE_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSet>
#include <QGridLayout>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DBinaryIface : public QObject
{
    Q_OBJECT

public:

    DBinaryIface(const QString& binaryName,
                 const QString& projectName,
                 const QString& url,
                 const QString& pluginName,
                 const QStringList& args = QStringList(),
                 const QString& desc = QString()
                );
    DBinaryIface(const QString& binaryName,
                 const QString& minimalVersion,
                 const QString& header,
                 const int headerLine,
                 const QString& projectName,
                 const QString& url,
                 const QString& pluginName,
                 const QStringList& args = QStringList(),
                 const QString& desc = QString()
                );
    virtual ~DBinaryIface();

    bool                isFound()                   const { return m_isFound;                       }
    const QString&      version()                   const;
    bool                versionIsRight()            const;
    bool                versionIsRight(const float) const;
    inline bool         isValid()                   const { return (m_isFound && versionIsRight()); }
    inline bool         developmentVersion()        const { return m_developmentVersion;            }
    const QString&      description()               const { return m_description;                   }

    virtual void        setup(const QString& prev = QString());
    virtual bool        checkDir()                        { return checkDir(m_pathDir);             }
    virtual bool        checkDir(const QString& path);
    virtual bool        recheckDirectories();

    virtual QString     path(const QString& dir)    const;
    virtual QString     path()                      const { return path(m_pathDir);                 }
    virtual QString     baseName()                  const { return m_binaryBaseName;                }
    virtual QString     minimalVersion()            const { return m_minimalVersion;                }


    virtual QUrl        url()                       const { return m_url;                           }
    virtual QString     projectName()               const { return m_projectName;                   }

    static QString      goodBaseName(const QString& b);

public Q_SLOTS:

    virtual void        slotNavigateAndCheck();
    virtual void        slotAddPossibleSearchDirectory(const QString& dir);
    virtual void        slotAddSearchDirectory(const QString& dir);

Q_SIGNALS:

    void                signalSearchDirectoryAdded(const QString& dir);
    void                signalBinaryValid();

protected:

    QString             findHeader(const QStringList& output, const QString& header) const;
    virtual bool        parseHeader(const QString& output);
    void                setVersion(QString& version);

    virtual QString     readConfig();
    virtual void        writeConfig();

protected:

    const bool          m_checkVersion;
    const QString       m_headerStarts;
    const int           m_headerLine;
    const QString       m_minimalVersion;
    const QString       m_configGroup;
    const QString       m_binaryBaseName;
    const QStringList   m_binaryArguments;
    const QString       m_projectName;
    const QUrl          m_url;

    bool                m_isFound;
    bool                m_developmentVersion;

    QString             m_version;
    QString             m_pathDir;
    QString             m_description;

    QFrame*             m_pathWidget;
    QLabel*             m_binaryLabel;
    QLabel*             m_versionLabel;
    QPushButton*        m_pathButton;
    QLabel*             m_downloadButton;
    QLineEdit*          m_lineEdit;
    QLabel*             m_statusIcon;

    QSet<QString>       m_searchPaths;
};

} // namespace Digikam

#endif // DBINARYIFACE_H
