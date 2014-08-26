/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : Objective-C wrapper to post events on OSX notifier
 *
 * Copyright (C) 2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt include

#include <QString>

// OSX header

#import <Foundation/Foundation.h>

void MacSendNotificationCenterMessage(NSString* const summary, NSString* const message)
{
    Class notifierClass     = NSClassFromString(@"NSUserNotificationCenter");
    id notificationCenter   = [notifierClass defaultUserNotificationCenter];
    Class notificationClass = NSClassFromString(@"NSUserNotification");
    id notification         = [[notificationClass alloc] init];

    [notification setTitle: summary];
    [notification setSubtitle: message];

    [notificationCenter deliverNotification: notification];
}

bool MacNotificationCenterSupported()
{
    return NSClassFromString(@"NSUserNotificationCenter");
}

bool MacNativeDispatchNotify(const QString& summary, const QString& message)
{
    if (!MacNotificationCenterSupported())
        return false;

    NSString* const sum = [[NSString alloc] initWithUTF8String:summary.toUtf8().constData()];
    NSString* const mes = [[NSString alloc] initWithUTF8String:message.toUtf8().constData()];
    MacSendNotificationCenterMessage(sum, mes);

    return true;
}

