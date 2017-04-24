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


#include "urn_jaus_jss_core_Management_1_0/Management_ReceiveFSM.h"

#include <ros/console.h>


using namespace JTS;

namespace urn_jaus_jss_core_Management_1_0
{



Management_ReceiveFSM::Management_ReceiveFSM(urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events_1_0::Events_ReceiveFSM* pEvents_ReceiveFSM, urn_jaus_jss_core_AccessControl_1_0::AccessControl_ReceiveFSM* pAccessControl_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new Management_ReceiveFSMContext(*this);

	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	this->pEvents_ReceiveFSM = pEvents_ReceiveFSM;
	this->pAccessControl_ReceiveFSM = pAccessControl_ReceiveFSM;
}



Management_ReceiveFSM::~Management_ReceiveFSM()
{
	delete context;

}

void Management_ReceiveFSM::setupNotifications()
{
	pAccessControl_ReceiveFSM->registerNotification("Receiving_Ready_NotControlled", ieHandler, "InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StateA_Init", "AccessControl_ReceiveFSM");
	pAccessControl_ReceiveFSM->registerNotification("Receiving_Ready_Controlled", ieHandler, "InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StateB_Standby", "AccessControl_ReceiveFSM");
	pAccessControl_ReceiveFSM->registerNotification("Receiving_Ready", ieHandler, "InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StateA_Init", "AccessControl_ReceiveFSM");
	pAccessControl_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StateA_Init", "AccessControl_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled_StateA_Standby", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled_StateA_Init", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled_StateA_Failure", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled_StateA_Shutdown", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled_StateA_Emergency", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled_StateA", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_NotControlled", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_NotControlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_Controlled_StateB_Standby", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_Controlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_Controlled_StateB_Ready", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_Controlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_Controlled_StateB_Emergency", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_Controlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_Controlled_StateB", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_Controlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready_Controlled", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready_Controlled", "Management_ReceiveFSM");
	registerNotification("Receiving_Ready", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving_Ready", "Management_ReceiveFSM");
	registerNotification("Receiving", pAccessControl_ReceiveFSM->getHandler(), "InternalStateChange_To_AccessControl_ReceiveFSM_Receiving", "Management_ReceiveFSM");
}

void Management_ReceiveFSM::deleteIDAction(Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
  for (unsigned int i = 0; i < p_emergency_clients.size(); i++) {
    if (p_emergency_clients[i].getSubsystemID() == transportData.getSrcSubsystemID()
        && p_emergency_clients[i].getNodeID() == transportData.getSrcNodeID()
        && p_emergency_clients[i].getComponentID() == transportData.getSrcComponentID()) {
      p_emergency_clients.erase(p_emergency_clients.begin()+i);
      break;
    }
  }
}

void Management_ReceiveFSM::goReadyAction()
{
  /// Insert User Code HERE
  ROS_DEBUG_NAMED("Management", "READY");
}

void Management_ReceiveFSM::goStandbyAction()
{
        /// Insert User Code HERE
  ROS_DEBUG_NAMED("Management", "STANDBY");
}


void Management_ReceiveFSM::initializeAction()
{
	/// Insert User Code HERE
  ieHandler->invoke(new Initialized());
}

//void Management_ReceiveFSM::resetTimerAction()
//{
//	/// Insert User Code HERE
//  pAccessControl_ReceiveFSM->resetTimerAction();
//}
//
void Management_ReceiveFSM::sendConfirmControlAction(RequestControl msg, std::string arg0, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
  uint16_t subsystem_id = transportData.getSrcSubsystemID();
  uint8_t node_id = transportData.getSrcNodeID();
  uint8_t component_id = transportData.getSrcComponentID();
  JausAddress sender(subsystem_id, node_id, component_id);
  try {
    ROS_DEBUG_NAMED("Management", "sendConfirmControlAction to %d.%d.%d, code: %s", subsystem_id, node_id, component_id, arg0.c_str());
    ConfirmControl confirm_msg;
    uint8_t responseCode = 0;
    if (arg0 == "CONTROL_ACCEPTED") {
      responseCode = 0;
    } else if (arg0 == "NOT_AVAILABLE") {
      responseCode = 1;
    } else if (arg0 == "INSUFFICIENT_AUTHORITY") {
      responseCode = 2;
    } else {
      ROS_WARN_NAMED("Management", "sendConfirmControlAction unknown code: %s", arg0.c_str());
    }
    confirm_msg.getBody()->getConfirmControlRec()->setResponseCode(responseCode);
    // Now send it to the requesting component
    sendJausMessage( confirm_msg, sender );
  } catch (std::exception &e) {
    ROS_WARN_NAMED("Management", "ERROR: %s", e.what());
  }
}

void Management_ReceiveFSM::sendRejectControlAction(ReleaseControl msg, std::string arg0, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
  uint16_t subsystem_id = transportData.getSrcSubsystemID();
  uint8_t node_id = transportData.getSrcNodeID();
  uint8_t component_id = transportData.getSrcComponentID();
  JausAddress sender(subsystem_id, node_id, component_id);
  ROS_DEBUG_NAMED("Management", "sendRejectControlAction to %d.%d.%d, code: %s", subsystem_id, node_id, component_id, arg0.c_str());
  RejectControl reject_msg;
  if (arg0 == "NOT_AVAILABLE") {
    reject_msg.getBody()->getRejectControlRec()->setResponseCode(1);
  } else {
    ROS_WARN_NAMED("Management", "sendRejectControlAction unknown code: %s", arg0.c_str());
  }
  // Now send it to the requesting component
  sendJausMessage( reject_msg, sender );
}

void Management_ReceiveFSM::sendRejectControlToControllerAction(std::string arg0)
{
	/// Insert User Code HERE
  ROS_DEBUG_NAMED("Management", "sendRejectControlToControllerAction: %s", arg0.c_str());
  pAccessControl_ReceiveFSM->sendRejectControlToControllerAction(arg0);
}

void Management_ReceiveFSM::sendReportStatusAction(QueryStatus msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
  uint16_t subsystem_id = transportData.getSrcSubsystemID();
  uint8_t node_id = transportData.getSrcNodeID();
  uint8_t component_id = transportData.getSrcComponentID();
  JausAddress sender(subsystem_id, node_id, component_id);
  ReportStatus response;
  jUnsignedByte status = getStateID();
  ROS_DEBUG_NAMED("Management", "Send ReportStatus (%d) to %d.%d.%d", status, subsystem_id, node_id, component_id);
  response.getBody()->getReportStatusRec()->setStatus(status);
  sendJausMessage(response, sender);
}

void Management_ReceiveFSM::storeIDAction(Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
  if (!isIDStored(transportData)) {
    uint16_t subsystem_id = transportData.getSrcSubsystemID();
    uint8_t node_id = transportData.getSrcNodeID();
    uint8_t component_id = transportData.getSrcComponentID();
    ROS_DEBUG_NAMED("Management", "Store ID %d.%d.%d", subsystem_id, node_id, component_id);
    JausAddress emergency_client(subsystem_id, node_id, component_id);
    p_emergency_clients.push_back(emergency_client);
  }

}

void Management_ReceiveFSM::popWrapper_0f9fae233502305d99c64a59f0f80a4e(ClearEmergency msg, Receive::Body::ReceiveRec transportData)
{
	std::string tempstr("Receiving_Ready_NotControlled_StateA_Emergency");
	std::string tempstr2(context->peakTopStateStack());
	char *leafStateTOK = strtok(const_cast<char*>(tempstr.c_str()),"_");
	char *stackStateTOK = strtok(const_cast<char*>(tempstr2.c_str()),"_");

	if(strcmp(const_cast<char*>(tempstr2.c_str()),"Receiving_Ready_NotControlled_StateA_Emergency") == 0)
	{
		deleteIDAction(transportData);
		return;
	}

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

}

void Management_ReceiveFSM::popWrapper_baf5fee6be943f3c815a84f544a0981e(ClearEmergency msg, Receive::Body::ReceiveRec transportData)
{
	std::string tempstr("Receiving_Ready_Controlled_StateB_Emergency");
	std::string tempstr2(context->peakTopStateStack());
	char *leafStateTOK = strtok(const_cast<char*>(tempstr.c_str()),"_");
	char *stackStateTOK = strtok(const_cast<char*>(tempstr2.c_str()),"_");

	if(strcmp(const_cast<char*>(tempstr2.c_str()),"Receiving_Ready_Controlled_StateB_Emergency") == 0)
	{
		deleteIDAction(transportData);
		return;
	}

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

	if(strcmp(leafStateTOK,stackStateTOK) != 0)
	{
		deleteIDAction(transportData);
		return;
	}
	leafStateTOK = strtok(leafStateTOK+1,"_");
	stackStateTOK = strtok(stackStateTOK+1,"_");

}



bool Management_ReceiveFSM::isControllingClient(Receive::Body::ReceiveRec transportData)
{
	//// By default, inherited guards call the parent function.
	//// This can be replaced or modified as needed.
	return pAccessControl_ReceiveFSM->isControllingClient(transportData );
}
bool Management_ReceiveFSM::isIDStored(Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
  for (unsigned int i = 0; i < p_emergency_clients.size(); i++) {
    if (p_emergency_clients[i].getSubsystemID() == transportData.getSrcSubsystemID()
        && p_emergency_clients[i].getNodeID() == transportData.getSrcNodeID()
        && p_emergency_clients[i].getComponentID() == transportData.getSrcComponentID()) {
      return true;
    }
  }
  return false;
}

int Management_ReceiveFSM::getStateID()
{
  // Get the state from the context.  Note that since we are inside a transition, the "current state"
  // is ill-defined.  We instead use the state we left to execute this transition.  Recall that the state
  // is actually an amalgamation of all parent states, we're only concerned with the management
  int result = 0;
  std::string state_name(context->getPreviousState()->getName());
  if (state_name.find("_") != std::string::npos)
    state_name = state_name.substr(state_name.find_last_of("_")+1);
  if (state_name == "Init")
    result = 0;
  if (state_name == "Ready")
    result = 1;
  if (state_name == "Standby")
    result = 2;
  if (state_name == "Shutdown")
    result = 3;
  if (state_name == "Failure")
    result = 4;
  if (state_name == "Emergency")
    result = 5;
  return result;
}


};
