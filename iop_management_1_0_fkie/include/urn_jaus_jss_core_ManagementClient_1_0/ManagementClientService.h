/**
ROS/IOP Bridge
Copyright (c) 2017 Fraunhofer

This program is dual licensed; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation, or
enter into a proprietary license agreement with the copyright
holder.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; or you can read the full license at
<http://www.gnu.de/documents/gpl-2.0.html>
*/

/** \author Alexander Tiderko */


#ifndef MANAGEMENTCLIENTSERVICE_H
#define MANAGEMENTCLIENTSERVICE_H

#include "Service.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JConstants.h"
#include "urn_jaus_jss_core_ManagementClient_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_ManagementClient_1_0/InternalEvents/InternalEventsSet.h"
#include "Transport/OS.h"

#include "urn_jaus_jss_core_Transport_1_0/TransportService.h"
#include "urn_jaus_jss_core_EventsClient_1_0/EventsClientService.h"
#include "urn_jaus_jss_core_AccessControlClient_1_0/AccessControlClientService.h"

#include "../urn_jaus_jss_core_ManagementClient_1_0/ManagementClient_ReceiveFSM.h"
#include "ManagementClient_SendFSM.h"

namespace urn_jaus_jss_core_ManagementClient_1_0
{

class DllExport ManagementClientService : public JTS::Service
{
public:
	ManagementClientService( JTS::JausRouter* jausRouter , urn_jaus_jss_core_Transport_1_0::TransportService* pTransportService, urn_jaus_jss_core_EventsClient_1_0::EventsClientService* pEventsClientService, urn_jaus_jss_core_AccessControlClient_1_0::AccessControlClientService* pAccessControlClientService);
	virtual ~ManagementClientService();

	virtual bool processTransitions(JTS::InternalEvent* ie);
	virtual bool defaultTransitions(JTS::InternalEvent* ie);

	// FSMs are public so that children services can access them
	ManagementClient_ReceiveFSM* pManagementClient_ReceiveFSM;
	ManagementClient_SendFSM* pManagementClient_SendFSM;

        jVariableLengthString getUri() { return "urn:jaus:jss:core:ManagementClient"; }
        jUnsignedByte getMajorVersionNumber() { return 1; }
        jUnsignedByte getMinorVersionNumber() { return 0; }
	template<class T>
	void set_status_handler(void(T::*handler)(JausAddress &, unsigned char code), T*obj) {
	  pManagementClient_ReceiveFSM->set_status_handler(handler, obj);
	}
	void queryStatus(JausAddress address) {
	  pManagementClient_ReceiveFSM->queryStatus(address);
	}
	void resume(JausAddress address) {
	  pManagementClient_ReceiveFSM->resume(address);
	}

protected:
	virtual void run();
};

};

#endif // MANAGEMENTCLIENTSERVICE_H
