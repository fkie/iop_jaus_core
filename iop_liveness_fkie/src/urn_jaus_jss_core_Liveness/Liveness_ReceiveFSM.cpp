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


#include "urn_jaus_jss_core_Liveness/Liveness_ReceiveFSM.h"

#include <ros/console.h>


using namespace JTS;

namespace urn_jaus_jss_core_Liveness
{

static ReportHeartbeatPulse report_heartbeart_pulse_;

Liveness_ReceiveFSM::Liveness_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events::Events_ReceiveFSM* pEvents_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new Liveness_ReceiveFSMContext(*this);

	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	this->pEvents_ReceiveFSM = pEvents_ReceiveFSM;

}



Liveness_ReceiveFSM::~Liveness_ReceiveFSM()
{
	delete context;
}

void Liveness_ReceiveFSM::setupNotifications()
{
	pEvents_ReceiveFSM->registerNotification("Receiving_Ready", ieHandler, "InternalStateChange_To_Liveness_ReceiveFSM_Receiving_Ready", "Events_ReceiveFSM");
	pEvents_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_Liveness_ReceiveFSM_Receiving_Ready", "Events_ReceiveFSM");
	registerNotification("Receiving_Ready", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving_Ready", "Liveness_ReceiveFSM");
	registerNotification("Receiving", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving", "Liveness_ReceiveFSM");
	this->pEvents_ReceiveFSM->set_event_report(0x2202, report_heartbeart_pulse_, false);
}

void Liveness_ReceiveFSM::sendReportHeartbeatPulseAction(QueryHeartbeatPulse msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
  /// Insert User Code HERE
    uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
    uint8_t node_id = transportData.getSourceID()->getNodeID();
    uint8_t component_id = transportData.getSourceID()->getComponentID();
    JausAddress sender(subsystem_id, node_id, component_id);
    ROS_DEBUG_NAMED("Liveness", "Sending ReportHeartbeatPulse to %d.%d.%d", subsystem_id, node_id, component_id);
    ReportHeartbeatPulse response;
    sendJausMessage(response, sender);
}





};
