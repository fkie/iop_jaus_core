/*
 * Copyright (c) 2015, Fraunhofer FKIE/CMS
 * All rights reserved.
 * Parts of this code are generated by JAUS Tool Set
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Fraunhofer nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


/** \author Alexander Tiderko */


#ifndef ACCESSCONTROL_RECEIVEFSM_H
#define ACCESSCONTROL_RECEIVEFSM_H

#include "JausUtils.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JTSStateMachine.h"
#include "urn_jaus_jss_core_AccessControl_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_AccessControl_1_0/InternalEvents/InternalEventsSet.h"

typedef JTS::Receive_1_0 Receive;
typedef JTS::Send_1_0 Send;

#include "urn_jaus_jss_core_Transport_1_0/Transport_ReceiveFSM.h"
#include "urn_jaus_jss_core_Events_1_0/Events_ReceiveFSM.h"


#include "AccessControl_ReceiveFSM_sm.h"

#include <ros/ros.h>

namespace urn_jaus_jss_core_AccessControl_1_0
{
	
class DllExport AccessControl_ReceiveFSM : public JTS::StateMachine
{
public:
	AccessControl_ReceiveFSM(urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events_1_0::Events_ReceiveFSM* pEvents_ReceiveFSM);
	virtual ~AccessControl_ReceiveFSM();
	
	/// Handle notifications on parent state changes
	virtual void setupNotifications();

	/// Action Methods
	virtual void initAction();
	virtual void resetTimerAction();
	virtual void sendConfirmControlAction(RequestControl msg, std::string arg0, Receive::Body::ReceiveRec transportData);
	virtual void sendRejectControlAction(ReleaseControl msg, std::string arg0, Receive::Body::ReceiveRec transportData);
	virtual void sendRejectControlToControllerAction(std::string arg0);
	virtual void sendReportAuthorityAction(QueryAuthority msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportControlAction(QueryControl msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportTimeoutAction(QueryTimeout msg, Receive::Body::ReceiveRec transportData);
	virtual void setAuthorityAction(RequestControl msg);
	virtual void setAuthorityAction(SetAuthority msg);
	virtual void storeAddressAction(Receive::Body::ReceiveRec transportData);


	/// Guard Methods
	virtual bool isAuthorityValid(SetAuthority msg);
	virtual bool isControlAvailable();
	virtual bool isControllingClient(Receive::Body::ReceiveRec transportData);
	virtual bool isCurrentAuthorityLess(RequestControl msg);
	virtual bool isDefaultAuthorityGreater(RequestControl msg);

	
	void timeout(void* arg);
	
	AccessControl_ReceiveFSMContext *context;
	
protected:
	DeVivo::Junior::JrTimer *p_timer;
	JausAddress *p_current_controller;
	jUnsignedByte p_current_authority;
	jUnsignedByte p_default_authority;
	jUnsignedByte p_default_timeout;
	bool p_control_available;
    /// References to parent FSMs
	urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM;
	urn_jaus_jss_core_Events_1_0::Events_ReceiveFSM* pEvents_ReceiveFSM;

	JTS::InternalEvent *p_timeout_event;

	ros::NodeHandle p_nh;
	
	ros::Publisher p_is_controlled_publisher;
};

};

#endif // ACCESSCONTROL_RECEIVEFSM_H
