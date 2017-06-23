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


#ifndef DISCOVERY_RECEIVEFSM_H
#define DISCOVERY_RECEIVEFSM_H

#include "JausUtils.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JTSStateMachine.h"
#include "urn_jaus_jss_core_Discovery/Messages/MessageSet.h"
#include "urn_jaus_jss_core_Discovery/InternalEvents/InternalEventsSet.h"

typedef JTS::Receive Receive;
typedef JTS::Send Send;

#include "urn_jaus_jss_core_Transport/Transport_ReceiveFSM.h"
#include "urn_jaus_jss_core_Events/Events_ReceiveFSM.h"


#include "Discovery_ReceiveFSM_sm.h"
#include "SimpleThread.h"
#include "iop_discovery_fkie/DiscoveryConfig.h"

#define RS_SSList ReportServiceList::Body::SubsystemList
#define RS_NList  RS_SSList::SubsystemSeq::NodeList
#define RS_CList  RS_NList::NodeSeq::ComponentList

namespace urn_jaus_jss_core_Discovery
{

class DllExport Discovery_ReceiveFSM : public JTS::StateMachine
{
public:
	Discovery_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events::Events_ReceiveFSM* pEvents_ReceiveFSM);
	virtual ~Discovery_ReceiveFSM();

	/// Handle notifications on parent state changes
	virtual void setupNotifications();

	/// Action Methods
	virtual void publishServicesAction(RegisterServices msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportConfigurationAction(QueryConfiguration msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportIdentificationAction(QueryIdentification msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportServiceListAction(QueryServiceList msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportServicesAction(QueryServices msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportSubsystemListAction(QuerySubsystemList msg, Receive::Body::ReceiveRec transportData);

	/// Guard Methods
	/// User Methods
	void registerService(int minver, int maxver, std::string serviceuri, JausAddress address);
	void registerSubsystem(JausAddress address);
	int getSystemID() { return p_discovery_config.system_id; }

	Discovery_ReceiveFSMContext *context;

protected:

    /// References to parent FSMs
	urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM;
	urn_jaus_jss_core_Events::Events_ReceiveFSM* pEvents_ReceiveFSM;

	discovery_config::DiscoveryConfig p_discovery_config;
	std::vector< std::pair<RegisterServices::RegisterServicesBody::ServiceList::ServiceRec, JausAddress> > uri_services;
	/** Variables used for registration by system **/
	std::vector<JausAddress> p_subsystems;

	bool isComponentRequested(QueryServices &msg, unsigned int nodeid, unsigned int compid);
	bool isComponentRequested(QueryServiceList &msg, unsigned int subsystemid, unsigned int nodeid, unsigned int compid);
	RS_SSList::SubsystemSeq *p_add_subsystem(RS_SSList *list, unsigned int id);
	RS_NList::NodeSeq *p_add_node(RS_NList *list, unsigned int id);
	RS_CList::ComponentSeq *p_add_component(RS_CList *list, unsigned int id);
};

};

#endif // DISCOVERY_RECEIVEFSM_H
