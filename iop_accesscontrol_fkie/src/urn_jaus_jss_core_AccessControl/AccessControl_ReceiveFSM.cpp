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


#include "urn_jaus_jss_core_AccessControl/AccessControl_ReceiveFSM.h"
#include <ros/console.h>



using namespace JTS;

namespace urn_jaus_jss_core_AccessControl
{


// Helper function.  This needs to be static so we can
// call it from pthread_create.
static void Timeout(void* arg)
{
	((AccessControl_ReceiveFSM*)arg)->timeout(arg);
}

AccessControl_ReceiveFSM::AccessControl_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events::Events_ReceiveFSM* pEvents_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	p_current_controller = NULL;
	p_current_authority = 0;
	p_default_authority = 1;
	p_default_timeout = 60;
	p_control_available = true;
	p_timeout_event = new InternalEvent("Timedout", "ControlTimeout");
	context = new AccessControl_ReceiveFSMContext(*this);
	p_timer = new DeVivo::Junior::JrTimer(Timeout, this, p_default_timeout*1000);

	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	this->pEvents_ReceiveFSM = pEvents_ReceiveFSM;
}



AccessControl_ReceiveFSM::~AccessControl_ReceiveFSM()
{
	p_timer->stop();
	delete p_timer;
	delete context;
	delete p_timeout_event;
}

void AccessControl_ReceiveFSM::setupNotifications()
{
	pEvents_ReceiveFSM->registerNotification("Receiving_Ready", ieHandler, "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Events_ReceiveFSM");
	pEvents_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Events_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving_Ready", "AccessControl_ReceiveFSM");
	registerNotification("Receiving_Ready_Controlled", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving_Ready", "AccessControl_ReceiveFSM");
	registerNotification("Receiving_Ready", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving_Ready", "AccessControl_ReceiveFSM");
	registerNotification("Receiving", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving", "AccessControl_ReceiveFSM");

}

void AccessControl_ReceiveFSM::timeout(void* arg)
{
	ROS_DEBUG_NAMED("AccessControl", "control timedout to %d.%d.%d", p_current_controller->getSubsystemID(), p_current_controller->getNodeID(), p_current_controller->getComponentID());
	((AccessControl_ReceiveFSM*)arg)->getHandler()->invoke(p_timeout_event);
	// create a new event, since the InternalEventHandler deletes the given.
	p_timeout_event = new InternalEvent("Timedout", "ControlTimeout");
}

void AccessControl_ReceiveFSM::initAction()
{
	/// Insert User Code HERE
	p_current_authority = p_default_authority;
}

void AccessControl_ReceiveFSM::resetTimerAction()
{
	/// Insert User Code HERE
//  printf("[AccessControl] ResetTimerAction: restart timer\n");
	p_timer->stop();
	p_timer->start();
}

void AccessControl_ReceiveFSM::sendConfirmControlAction(RequestControl msg, std::string arg0, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	ROS_DEBUG_NAMED("AccessControl", "sendConfirmControlAction to %d.%d.%d, code: %s", subsystem_id, node_id, component_id, arg0.c_str());
	ConfirmControl confirm_msg;
	uint8_t responseCode = 0;
	if (arg0 == "CONTROL_ACCEPTED") {
		responseCode = 0;
		p_timer->start();
	} else if (arg0 == "NOT_AVAILABLE") {
		responseCode = 1;
	} else if (arg0 == "INSUFFICIENT_AUTHORITY") {
		responseCode = 2;
	} else {
		ROS_WARN_NAMED("AccessControl", "sendConfirmControlAction unknown code: %s", arg0.c_str());
	}
	confirm_msg.getBody()->getConfirmControlRec()->setResponseCode(responseCode);
	// Now send it to the requesting component
	sendJausMessage( confirm_msg, sender );
}

void AccessControl_ReceiveFSM::sendRejectControlAction(ReleaseControl msg, std::string arg0, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	ROS_DEBUG_NAMED("AccessControl", "sendRejectControlAction to %d.%d.%d, code: %s",
			subsystem_id, node_id, component_id, arg0.c_str());
	RejectControl reject_msg;
	if (arg0 == "CONTROL_RELEASED") {
		reject_msg.getBody()->getRejectControlRec()->setResponseCode(0);
		if (p_current_controller != NULL) {
			ROS_DEBUG_NAMED("AccessControl", "delete CONTROLER");
			delete p_current_controller;
			p_current_controller = NULL;
			p_current_authority = p_default_authority;
			p_timer->stop();
		}
	} else if (arg0 == "NOT_AVAILABLE") {
		reject_msg.getBody()->getRejectControlRec()->setResponseCode(1);
	} else {
		ROS_WARN_NAMED("AccessControl", "sendRejectControlAction unknown code: %s", arg0.c_str());
	}
	// Now send it to the requesting component
	sendJausMessage( reject_msg, sender );
}

void AccessControl_ReceiveFSM::sendRejectControlToControllerAction(std::string arg0)
{
	/// Insert User Code HERE
	if (p_current_controller != NULL) {
		uint16_t subsystem_id = p_current_controller->getSubsystemID();
		uint8_t node_id = p_current_controller->getNodeID();
		uint8_t component_id = p_current_controller->getComponentID();
		JausAddress controllerid(subsystem_id, node_id, component_id);
		ROS_DEBUG_NAMED("AccessControl", "sendRejectControlToControllerAction to %d.%d.%d, code: %s",
					subsystem_id, node_id, component_id, arg0.c_str());
		RejectControl reject_msg;
		if (arg0 == "CONTROL_RELEASED") {
			if (p_current_controller != NULL) {
				ROS_DEBUG_NAMED("AccessControl", "delete current CONTROLER");
				delete p_current_controller;
				p_current_controller = NULL;
				p_current_authority = p_default_authority;
				p_timer->stop();
			}
			reject_msg.getBody()->getRejectControlRec()->setResponseCode(0);
		} else if (arg0 == "NOT_AVAILABLE") {
			reject_msg.getBody()->getRejectControlRec()->setResponseCode(1);
		} else {
			ROS_WARN_NAMED("AccessControl", "sendRejectControlToControllerAction unknown code: %s", arg0.c_str());
		}
		// Now send it to the requesting component
		sendJausMessage( reject_msg, controllerid );
	} else {
		ROS_DEBUG_NAMED("AccessControl", "sendRejectControlToControllerAction not controller");
	}

}

void AccessControl_ReceiveFSM::sendReportAuthorityAction(QueryAuthority msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	ROS_DEBUG_NAMED("AccessControl", "sendReportAuthorityAction to %d.%d.%d, authority: %d",
				subsystem_id, node_id, component_id, p_current_authority);
	ReportAuthority response;
	response.getBody()->getReportAuthorityRec()->setAuthorityCode(p_current_authority);
	sendJausMessage(response, sender);
}

void AccessControl_ReceiveFSM::sendReportControlAction(QueryControl msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	ReportControl response;
	unsigned int sid = 0;
	unsigned int nid = 0;
	unsigned int cid = 0;
	if (p_current_controller != NULL) {
		sid = p_current_controller->getSubsystemID();
		nid = p_current_controller->getNodeID();
		cid = p_current_controller->getComponentID();
	}
	response.getBody()->getReportControlRec()->setSubsystemID(sid);
	response.getBody()->getReportControlRec()->setNodeID(nid);
	response.getBody()->getReportControlRec()->setComponentID(cid);
	response.getBody()->getReportControlRec()->setAuthorityCode(p_current_authority);
	ROS_DEBUG_NAMED("AccessControl", "Sending ReportControl to %d.%d.%d; current controller: %d.%d.%d",
				subsystem_id, node_id, component_id, sid, nid, cid);

	sendJausMessage(response, sender);
}

void AccessControl_ReceiveFSM::sendReportTimeoutAction(QueryTimeout msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	jUnsignedByte timeout = p_default_timeout;
	ROS_DEBUG_NAMED("AccessControl", "Sending ReportTimeout to %d.%d.%d, timeout: %d",
				subsystem_id, node_id, component_id, timeout);
	ReportTimeout response;
	response.getBody()->getReportTimoutRec()->setTimeout(timeout);
	sendJausMessage(response, sender);
}

void AccessControl_ReceiveFSM::setAuthorityAction(RequestControl msg)
{
	/// Insert User Code HERE
  p_current_authority = msg.getBody()->getRequestControlRec()->getAuthorityCode();
  ROS_DEBUG_NAMED("AccessControl", "setAuthotityAction while RequestControl to %d", p_current_authority);
}

void AccessControl_ReceiveFSM::setAuthorityAction(SetAuthority msg)
{
	/// Insert User Code HERE
	p_current_authority = msg.getBody()->getAuthorityRec()->getAuthorityCode();
	ROS_DEBUG_NAMED("AccessControl", "setAuthorityAction to %d", p_current_authority);
}

void AccessControl_ReceiveFSM::storeAddressAction(Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	ROS_DEBUG_NAMED("AccessControl", "store address of controlling component as %d.%d.%d",
			subsystem_id, node_id, component_id);
	if (p_current_controller != NULL) {
		delete p_current_controller;
	}
	p_current_controller = new JausAddress(subsystem_id, node_id, component_id);
}



bool AccessControl_ReceiveFSM::isAuthorityValid(SetAuthority msg)
{
	/// Insert User Code HERE
	if ((msg.getBody()->getAuthorityRec()->getAuthorityCode() <= p_current_authority)
		&& msg.getBody()->getAuthorityRec()->getAuthorityCode() >= p_default_authority)
	return true;
	return false;
}
bool AccessControl_ReceiveFSM::isControlAvailable()
{
	/// Insert User Code HERE
	return p_control_available;
}
bool AccessControl_ReceiveFSM::isControllingClient(Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	if (p_current_controller != NULL) {
		JausAddress requester(transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
		if (requester == *p_current_controller) {
			return true;
		}
	}
	return false;
}
bool AccessControl_ReceiveFSM::isCurrentAuthorityLess(RequestControl msg)
{
	/// Insert User Code HERE
	if (p_current_authority < msg.getBody()->getRequestControlRec()->getAuthorityCode())
		return true;
	return false;
}
bool AccessControl_ReceiveFSM::isDefaultAuthorityGreater(RequestControl msg)
{
	/// Insert User Code HERE
	if (p_default_authority > msg.getBody()->getRequestControlRec()->getAuthorityCode())
		return true;
	return false;
}


};
