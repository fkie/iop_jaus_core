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


#ifndef MANAGEMENT_RECEIVEFSM_H
#define MANAGEMENT_RECEIVEFSM_H

#include "JausUtils.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JTSStateMachine.h"
#include "urn_jaus_jss_core_Management_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_Management_1_0/InternalEvents/InternalEventsSet.h"

typedef JTS::Receive_1_0 Receive;
typedef JTS::Send_1_0 Send;

#include "urn_jaus_jss_core_Transport_1_0/Transport_ReceiveFSM.h"
#include "urn_jaus_jss_core_Events_1_0/Events_ReceiveFSM.h"
#include "urn_jaus_jss_core_AccessControl_1_0/AccessControl_ReceiveFSM.h"


#include "Management_ReceiveFSM_sm.h"

namespace urn_jaus_jss_core_Management_1_0
{
/**
 * Internal status, see StatusReport
 **/
const int STATE_INITIALIZE = 0;
const int STATE_READY = 1;
const int STATE_STANDBY = 2;
const int STATE_SHUTDOWN = 3;
const int STATE_FAILURE = 4;
const int STATE_EMERGENCY = 5;

class DllExport Management_ReceiveFSM : public JTS::StateMachine
{
public:
	Management_ReceiveFSM(urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events_1_0::Events_ReceiveFSM* pEvents_ReceiveFSM, urn_jaus_jss_core_AccessControl_1_0::AccessControl_ReceiveFSM* pAccessControl_ReceiveFSM);
	virtual ~Management_ReceiveFSM();

	/// Handle notifications on parent state changes
	virtual void setupNotifications();

	/// Action Methods
	virtual void deleteIDAction(Receive::Body::ReceiveRec transportData);
	virtual void goReadyAction();
	virtual void goStandbyAction();
	virtual void initializeAction();
//	virtual void resetTimerAction();
	virtual void sendConfirmControlAction(RequestControl msg, std::string arg0, Receive::Body::ReceiveRec transportData);
	virtual void sendRejectControlAction(ReleaseControl msg, std::string arg0, Receive::Body::ReceiveRec transportData);
	virtual void sendRejectControlToControllerAction(std::string arg0);
	virtual void sendReportStatusAction(QueryStatus msg, Receive::Body::ReceiveRec transportData);
	virtual void storeIDAction(Receive::Body::ReceiveRec transportData);
	virtual void popWrapper_0f9fae233502305d99c64a59f0f80a4e(ClearEmergency msg, Receive::Body::ReceiveRec transportData);
	virtual void popWrapper_baf5fee6be943f3c815a84f544a0981e(ClearEmergency msg, Receive::Body::ReceiveRec transportData);


        /// Guard Methods
	virtual bool isControllingClient(Receive::Body::ReceiveRec transportData);
	virtual bool isIDStored(Receive::Body::ReceiveRec transportData);



	Management_ReceiveFSMContext *context;

protected:

    /// References to parent FSMs
	urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM;
	urn_jaus_jss_core_Events_1_0::Events_ReceiveFSM* pEvents_ReceiveFSM;
	urn_jaus_jss_core_AccessControl_1_0::AccessControl_ReceiveFSM* pAccessControl_ReceiveFSM;

        std::vector<JausAddress> p_emergency_clients;
        int getStateID();
};

};

#endif // MANAGEMENT_RECEIVEFSM_H
