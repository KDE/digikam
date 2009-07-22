/*
 * databaseconfigelement.h
 *
 *  Created on: 27.06.2009
 *      Author: mueller
 */

#ifndef DATABASECONFIGELEMENT_H_
#define DATABASECONFIGELEMENT_H_

#include <QtGlobal>
#include <QMap>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam{
            class DIGIKAM_EXPORT databaseActionElement {
            public:
                QString m_Mode;
                int     m_Order;
                QString m_Statement;
        };

        class DIGIKAM_EXPORT databaseAction {
                public:
                    QString m_Name;
		    QString m_Mode;
                    QList<databaseActionElement> m_DBActionElements;
        };

	class DIGIKAM_EXPORT databaseconfigelement {
	public:
		databaseconfigelement();
		virtual ~databaseconfigelement();

		QString m_DatabaseID;
		QString m_HostName;
		QString m_Port;
		QString m_ConnectOptions;
		QString m_DatabaseName;
		QString m_UserName;
		QString m_Password;
                QMap<QString, databaseAction> m_SQLStatements;
	};
}
#endif /* DATABASECONFIGELEMENT_H_ */
