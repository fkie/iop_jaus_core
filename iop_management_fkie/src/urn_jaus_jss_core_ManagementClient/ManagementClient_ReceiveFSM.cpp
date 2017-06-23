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


#include "urn_jaus_jss_core_ManagementClient/ManagementClient_ReceiveFSM.h"


#include <ros/ros.h>
#include <ros/console.h>

using namespace JTS;

namespace urn_jaus_jss_core_ManagementClient
{

unsigned char MANAGEMENT_STATE_INIT      = 0;
unsigned char MANAGEMENT_STATE_READY     = 1;
unsigned char MANAGEMENT_STATE_STANDBY   = 2;
unsigned char MANAGEMENT_STATE_SHUTDOWN  = 3;
unsigned char MANAGEMENT_STATE_FAILURE   = 4;
unsigned char MANAGEMENT_STATE_EMERGENCY = 5;
unsigned char MANAGEMENT_STATE_UNKNOWN   = 255;

ManagementClient_ReceiveFSM::ManagementClient_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_EventsClient::EventsClient_ReceiveFSM* pEventsClient_ReceiveFSM, urn_jaus_jss_core_AccessControlClient::AccessControlClient_ReceiveFSM* pAccessControlClient_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new ManagementClient_ReceiveFSMContext(*this);

	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	this->pEventsClient_ReceiveFSM = pEventsClient_ReceiveFSM;
	this->pAccessControlClient_ReceiveFSM = pAccessControlClient_ReceiveFSM;
	p_status = 255;
//	p_on_status_query = false;
//	p_ts_status_query = 0;
//	p_delete_ts_after = 3.0;
}



ManagementClient_ReceiveFSM::~ManagementClient_ReceiveFSM()
{
	delete context;
}

void ManagementClient_ReceiveFSM::setupNotifications()
{
	pAccessControlClient_ReceiveFSM->registerNotification("Receiving_Ready", ieHandler, "InternalStateChange_To_ManagementClient_ReceiveFSM_Receiving_Ready", "AccessControlClient_ReceiveFSM");
	pAccessControlClient_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_ManagementClient_ReceiveFSM_Receiving_Ready", "AccessControlClient_ReceiveFSM");
	registerNotification("Receiving_Ready", pAccessControlClient_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControlClient_ReceiveFSM_Receiving_Ready", "ManagementClient_ReceiveFSM");
	registerNotification("Receiving", pAccessControlClient_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControlClient_ReceiveFSM_Receiving", "ManagementClient_ReceiveFSM");

}

void ManagementClient_ReceiveFSM::reportStatusAction(ReportStatus msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	if (!p_class_interface_callback.empty()) {
		uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
		uint8_t node_id = transportData.getSourceID()->getNodeID();
		uint8_t component_id = transportData.getSourceID()->getComponentID();
		JausAddress sender(subsystem_id, node_id, component_id);
		p_status = msg.getBody()->getReportStatusRec()->getStatus();
		//    p_on_status_query = false;
		ROS_DEBUG_NAMED("ManagementClient", "ReportStatus, status: %d (%s)", p_status, p_status_to_str(p_status).c_str());
		//    if (p_do_resume and p_status != 3) {
		//      ROS_DEBUG_NAMED("ManagementClient", "  Resume!\n");
		//      p_do_resume = false;
		//      queryStatus(sender);
		//    }
		ROS_DEBUG_NAMED("ManagementClient", "  forward to handler");
		p_class_interface_callback(sender, p_status);
	}
}

void ManagementClient_ReceiveFSM::queryStatus(JausAddress address)
{
	QueryStatus msg;
	sendJausMessage( msg, address );
}

void ManagementClient_ReceiveFSM::resume(JausAddress address)
{
//  pAccessControlClient_ReceiveFSM->requestAccess(address, authority);
	Resume resume_msg;
	sendJausMessage( resume_msg, address);
	queryStatus(address);
}

std::string ManagementClient_ReceiveFSM::p_status_to_str(unsigned char status) {
	switch (status) {
		case 0: return "INIT";
		case 1: return "READY";
		case 2: return "STANDBY";
		case 3: return "SHUTDOWN";
		case 4: return "FAILURE";
		case 5: return "EMERGENCY";
		default: return "UNKNOWN";
	}
}

};
