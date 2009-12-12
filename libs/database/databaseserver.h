/*
 * databaseserver.h
 *
 *  Created on: 27.11.2009
 *      Author: mueller
 */

#ifndef DATABASESERVER_H_
#define DATABASESERVER_H_

// Local includes

#include "digikam_export.h"

class DIGIKAM_EXPORT DatabaseServer
{
    public:
        void startDatabaseProcess();
        void createDatabase();
        void stopDatabaseProcess();
};


#endif /* DATABASESERVER_H_ */
