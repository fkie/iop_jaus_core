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


#ifndef EVENTSSERVICE_H
#define EVENTSSERVICE_H

#include "Service.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JConstants.h"
#include "urn_jaus_jss_core_Events_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_Events_1_0/InternalEvents/InternalEventsSet.h"
#include "Transport/OS.h"

#include "urn_jaus_jss_core_Transport_1_0/TransportService.h"
#include "Events_ReceiveFSM.h"
#include "Events_SendFSM.h"


namespace urn_jaus_jss_core_Events_1_0
{

class DllExport EventsService : public JTS::Service
{
public:
	EventsService( JTS::JausRouter* jausRouter , urn_jaus_jss_core_Transport_1_0::TransportService* pTransportService);
	virtual ~EventsService();

	virtual bool processTransitions(JTS::InternalEvent* ie);
	virtual bool defaultTransitions(JTS::InternalEvent* ie);

	// FSMs are public so that children services can access them
	Events_ReceiveFSM* pEvents_ReceiveFSM;
	Events_SendFSM* pEvents_SendFSM;

	/** Register/update the supported event. The caller has to store the report, because only the given reference to the
	 * report will be stored and used to send the report to a client, which create an event.
	 * If on change is supported you have to call this method each time the report was changed.
	 * **/
	void set_event_report(jUnsignedShortInteger query_msg_id, JTS::Message &report, bool on_change_supported=true) {
	  pEvents_ReceiveFSM->set_event_report(query_msg_id, report, on_change_supported);
	}

protected:
	virtual void run();
};

};

#endif // EVENTSSERVICE_H
