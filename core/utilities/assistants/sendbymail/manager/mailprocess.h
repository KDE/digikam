/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-25
 * Description : a tool to e-mailing images
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAIL_PROCESS_H
#define MAIL_PROCESS_H

// Qt includes

#include <QObject>

// Local includes

#include "mailsettings.h"
#include "dinfointerface.h"

namespace Digikam
{

class MailProcess : public QObject
{
    Q_OBJECT

public:

    explicit MailProcess(MailSettings* const settings,
                         DInfoInterface* const iface,
                         QObject* const parent);
    ~MailProcess();

    void firstStage();

Q_SIGNALS:

    void signalProgress(int);
    void signalMessage(QString, bool);
    void signalDone(bool);

public Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void slotStartingResize(const QUrl&);
    void slotFinishedResize(const QUrl&, const QUrl&, int);
    void slotFailedResize(const QUrl&, const QString&, int);
    void slotCompleteResize();
    void slotCleanUp();

private:

    void secondStage();
    void invokeMailAgentError(const QString& prog, const QStringList& args);
    void invokeMailAgentDone(const QString& prog, const QStringList& args);

    /** Creates a text file with all images Comments, Tags, and Rating.
     */
    void buildPropertiesFile();

    /** Invokes mail agent. Depending on which mail agent to be used, we have different
     *  proceedings. Easy for every agent except of mozilla derivates
     */
    bool invokeMailAgent();

    /** Shows up an error dialog about the problematic resized images.
     */
    bool showFailedResizedImages() const;

    /** Returns a list of image urls, whose sum file-size is smaller than the quota set in dialog.
     *  The returned list are images than we can send immediately, and are removed from
     *  d->attachementFiles list.
     *  Files which still in d->attachementFiles need to be send by another pass.
     */
    QList<QUrl> divideEmails();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // MAIL_PROCESS_H
