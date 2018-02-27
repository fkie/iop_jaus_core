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


#ifndef DISCOVERYCLIENT_RECEIVEFSM_H
#define DISCOVERYCLIENT_RECEIVEFSM_H

#include "JausUtils.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JTSStateMachine.h"
#include "urn_jaus_jss_core_DiscoveryClient/Messages/MessageSet.h"
#include "urn_jaus_jss_core_DiscoveryClient/InternalEvents/InternalEventsSet.h"

#include "InternalEvents/Receive.h"
#include "InternalEvents/Send.h"

#include "urn_jaus_jss_core_Transport/Transport_ReceiveFSM.h"
#include "urn_jaus_jss_core_EventsClient/EventsClient_ReceiveFSM.h"


#include "DiscoveryClient_ReceiveFSM_sm.h"
#include "urn_jaus_jss_core_Discovery/Discovery_ReceiveFSM.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <ros/ros.h>
#include <iop_msgs_fkie/Identification.h>
#include <iop_msgs_fkie/Component.h>
#include <iop_msgs_fkie/QueryIdentification.h>
#include <iop_msgs_fkie/JausAddress.h>
#include <iop_msgs_fkie/Node.h>
#include <iop_msgs_fkie/Service.h>
#include <iop_msgs_fkie/Subsystem.h>
#include <iop_msgs_fkie/System.h>
#include <std_srvs/Empty.h>
#include <iop_discovery_fkie/DiscoveryServiceDef.h>


namespace urn_jaus_jss_core_DiscoveryClient
{

const int TYPE_SYSTEM = 1;
const int TYPE_SUBSYSTEM = 2;
const int TYPE_NODE = 3;
const int TYPE_COMPONENT = 4;

using namespace urn_jaus_jss_core_Discovery;

//typedef RegisterServices::RegisterServicesBody::ServiceList::ServiceRec ServiceDef;

class DllExport DiscoveryClient_ReceiveFSM : public JTS::StateMachine
{
public:
	int TIMEOUT_DISCOVER;
	int TIMEOUT_STANDBY;
	DiscoveryClient_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_EventsClient::EventsClient_ReceiveFSM* pEventsClient_ReceiveFSM);
	virtual ~DiscoveryClient_ReceiveFSM();

	/// Handle notifications on parent state changes
	virtual void setupNotifications();

	/// Action Methods
	virtual void handleReportConfigurationAction(ReportConfiguration msg, Receive::Body::ReceiveRec transportData);
	virtual void handleReportIdentificationAction(ReportIdentification msg, Receive::Body::ReceiveRec transportData);
	virtual void handleReportServiceListAction(ReportServiceList msg, Receive::Body::ReceiveRec transportData);
	virtual void handleReportServicesAction(ReportServices msg, Receive::Body::ReceiveRec transportData);
	virtual void handleReportSubsystemListAction(ReportSubsystemList msg, Receive::Body::ReceiveRec transportData);
	virtual void registerAction();
	virtual void sendQueryIdentificationAction();
	virtual void sendRegisterServicesAction(ReportIdentification msg, Receive::Body::ReceiveRec transportData);


	/// Guard Methods
	virtual bool isRegistered();
	virtual bool onRegistration();

	void setDiscoveryFSM(Discovery_ReceiveFSM *discovery_fsm);
	void appendServiceUri(std::string service_uri, unsigned char major_version=1, unsigned char minor_version=0);
	template<class T>
	void discover(std::string service_uri, void(T::*handler)(const std::string &, JausAddress &), T*obj, unsigned char major_version=1, unsigned char minor_version=255, unsigned short subsystem=65535)
	{
		boost::function<void (const std::string &, JausAddress &)> callback = boost::bind(handler, obj, _1, _2);;
		iop::DiscoveryServiceDef service(service_uri, major_version, minor_version);
		p_discover_callbacks[service].push_back(callback);
		pDiscover(service_uri, major_version, minor_version, subsystem);
	}
	void cancel_discovery();
	void cancel_discovery(std::string service_uri, unsigned char major_version=1, unsigned char minor_version=255);
	void query_identification(int query_type, jUnsignedShortInteger subsystem=0xFFFF, jUnsignedByte node=0xFF, jUnsignedByte component=0xFF);
	template<class T>
	void set_discovery_handler(void(T::*handler)(const std::string &, JausAddress &), T*obj) {
		class_discovery_callback_ = boost::bind(handler, obj, _1, _2);
	}

	DiscoveryClient_ReceiveFSMContext *context;

protected:

	class DiscoverItem {
	public:
		iop::DiscoveryServiceDef service;
		int subsystem;
		std::set<unsigned short> discovered_in;

		bool discovered(unsigned short subsystem_id=65535) {
			if (subsystem_id == 65535) {
				return discovered_in.size() > 0;
			}
			return discovered_in.count(subsystem_id) > 0;
		}
	};

    /// References to parent FSMs
	urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM;
	urn_jaus_jss_core_EventsClient::EventsClient_ReceiveFSM* pEventsClient_ReceiveFSM;

	typedef boost::recursive_mutex mutex_type;
	typedef boost::unique_lock<mutex_type> lock_type;
	mutable mutex_type p_mutex;

	// ros parameter
	// 0: Reserved, 1: System Identification, 2: Subsystem Identification, 3: Node Identification, 4: Component Identification, 5 - 255: Reserved
	int system_id;
	bool register_own_services;
	Discovery_ReceiveFSM *p_discovery_fsm;
	int p_current_timeout;
	int p_force_component_update_after;
	bool p_first_ready;
	bool p_is_registered;
	bool p_on_registration;
	int p_count_discover_tries;
	std::vector<iop::DiscoveryServiceDef> p_own_uri_services;
	/** Variables used for registration by subsystem or node **/
	JausAddress p_addr_discovery_service;
	ros::WallTimer p_timeout_timer;
	JTS::InternalEvent *p_timeout_event;
	iop_msgs_fkie::System p_discovered_system;
	/// p_count_queries and p_count_service_lists are for compatibility to v1.0
	int p_count_queries;
	bool p_recved_service_lists;
	int p_timeout_discover_service;

	std::map <iop::DiscoveryServiceDef, std::vector<boost::function<void (const std::string &, JausAddress &)> > > p_discover_callbacks;  // Service to discover, list with callbacks requested this service
	boost::function<void (const std::string &, JausAddress &)> class_discovery_callback_;
	std::vector<DiscoverItem> p_discover_services;
	void pRegistrationFinished();
	void pCheckTimer();
	JausAddress pGetService(ReportServices &msg, iop::DiscoveryServiceDef service, unsigned short subsystem);
	JausAddress pGetService(ReportServiceList &msg, iop::DiscoveryServiceDef service, unsigned short subsystem);
	bool pIsKnownComponent(iop_msgs_fkie::JausAddress &addr);
	bool pEqualIdent(iop_msgs_fkie::Identification &ros_ident, iop_msgs_fkie::JausAddress &addr);
	void pUpdateSubsystemIdent(iop_msgs_fkie::JausAddress &addr, ReportIdentification &report_ident);
	void pUpdateNodeIdent(iop_msgs_fkie::JausAddress &addr, ReportIdentification &report_ident);
	void pUpdateComponent(iop_msgs_fkie::JausAddress &node_addr, iop_msgs_fkie::JausAddress &addr, ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list);
	void pUpdateComponent(iop_msgs_fkie::JausAddress &node_addr, iop_msgs_fkie::JausAddress &addr, ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list);
	void pTimeoutCallback(const ros::WallTimerEvent& event);
	bool pHasToDiscover(unsigned short subsystem_id);
	void pInformDiscoverCallbacks(iop::DiscoveryServiceDef &service, JausAddress &address);

	/** Parameter and functions for ROS interface to publish the IOP system.*/
	bool p_enable_ros_interface;
	ros::Publisher p_pub_identification;
	ros::Publisher p_pub_system;
	ros::ServiceServer p_srv_query_ident;
	ros::ServiceServer p_srv_update_system;
	std::map<unsigned short, unsigned int> p_discovery_srvs_stamps;  // subsystem ID, seconds of last update

	bool pQueryIdentificationSrv(iop_msgs_fkie::QueryIdentification::Request  &req, iop_msgs_fkie::QueryIdentification::Response &res);
	bool pUpdateSystemSrv(std_srvs::Empty::Request  &req, std_srvs::Empty::Response &res);
	void pDiscover(std::string service_uri, unsigned char major_version=1, unsigned char minor_version=0, unsigned short subsystem=65535);
	std::map<int, std::string> p_system_id_map();
	void p_check_for_timeout_discovery_service();

};

};

#endif // DISCOVERYCLIENT_RECEIVEFSM_H
