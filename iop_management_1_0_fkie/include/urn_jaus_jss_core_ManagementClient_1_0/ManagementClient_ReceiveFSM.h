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


#ifndef MANAGEMENTCLIENT_RECEIVEFSM_H
#define MANAGEMENTCLIENT_RECEIVEFSM_H

#include "JausUtils.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JTSStateMachine.h"
#include "urn_jaus_jss_core_ManagementClient_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_ManagementClient_1_0/InternalEvents/InternalEventsSet.h"

typedef JTS::Receive_1_0 Receive;
typedef JTS::Send_1_0 Send;

#include "urn_jaus_jss_core_Transport_1_0/Transport_ReceiveFSM.h"
#include "urn_jaus_jss_core_EventsClient_1_0/EventsClient_ReceiveFSM.h"
#include "urn_jaus_jss_core_AccessControlClient_1_0/AccessControlClient_ReceiveFSM.h"


#include "ManagementClient_ReceiveFSM_sm.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace urn_jaus_jss_core_ManagementClient_1_0
{

class DllExport ManagementClient_ReceiveFSM : public JTS::StateMachine
{
public:
	static unsigned char MANAGEMENT_STATE_INIT;
	static unsigned char MANAGEMENT_STATE_READY;
	static unsigned char MANAGEMENT_STATE_STANDBY;
	static unsigned char MANAGEMENT_STATE_SHUTDOWN;
	static unsigned char MANAGEMENT_STATE_FAILURE;
	static unsigned char MANAGEMENT_STATE_EMERGENCY;
	static unsigned char MANAGEMENT_STATE_UNKNOWN;

	ManagementClient_ReceiveFSM(urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_EventsClient_1_0::EventsClient_ReceiveFSM* pEventsClient_ReceiveFSM, urn_jaus_jss_core_AccessControlClient_1_0::AccessControlClient_ReceiveFSM* pAccessControlClient_ReceiveFSM);
	virtual ~ManagementClient_ReceiveFSM();

	/// Handle notifications on parent state changes
	virtual void setupNotifications();

	/// Action Methods
	virtual void reportStatusAction(ReportStatus msg, Receive::Body::ReceiveRec transportData);


	/// Guard Methods

	/// user methods
	template<class T>
	void set_status_handler(void(T::*handler)(JausAddress &, unsigned char code), T*obj) {
		p_class_interface_callback = boost::bind(handler, obj, _1, _2);
	}
	void queryStatus(JausAddress address);
	void resume(JausAddress address);

	ManagementClient_ReceiveFSMContext *context;

protected:

    /// References to parent FSMs
	urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM;
	urn_jaus_jss_core_EventsClient_1_0::EventsClient_ReceiveFSM* pEventsClient_ReceiveFSM;
	urn_jaus_jss_core_AccessControlClient_1_0::AccessControlClient_ReceiveFSM* pAccessControlClient_ReceiveFSM;

	boost::function<void (JausAddress &, unsigned char code)> p_class_interface_callback;

	unsigned char p_status;
};

};

#endif // MANAGEMENTCLIENT_RECEIVEFSM_H
