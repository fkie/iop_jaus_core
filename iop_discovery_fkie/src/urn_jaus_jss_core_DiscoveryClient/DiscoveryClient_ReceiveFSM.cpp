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


#include "urn_jaus_jss_core_DiscoveryClient/DiscoveryClient_ReceiveFSM.h"

#include <algorithm>
#include <ros/console.h>
#include <iop_component_fkie/iop_config.h>


using namespace JTS;
using namespace urn_jaus_jss_core_Discovery;

namespace urn_jaus_jss_core_DiscoveryClient
{

DiscoveryClient_ReceiveFSM::DiscoveryClient_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_EventsClient::EventsClient_ReceiveFSM* pEventsClient_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new DiscoveryClient_ReceiveFSMContext(*this);
	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	this->pEventsClient_ReceiveFSM = pEventsClient_ReceiveFSM;
	system_id = 4;
	register_own_services = true;
	p_force_component_update_after = 300;
	TIMEOUT_DISCOVER = 5;
	TIMEOUT_STANDBY = 10;
	p_current_timeout = TIMEOUT_STANDBY;
	p_discovery_fsm = NULL;
	p_first_ready = true;
	p_is_registered = false;
	p_on_registration = false;
	p_count_discover_tries = 0;
	p_count_queries = 0;
	p_recved_service_lists = false;
	p_timeout_event = new InternalEvent("Timeout", "ControlTimeout");
	p_enable_ros_interface = false;
	p_timeout_discover_service = 60;
}



DiscoveryClient_ReceiveFSM::~DiscoveryClient_ReceiveFSM()
{

	p_timeout_timer.stop();
//	p_ident_timer.stop();
	delete context;
	delete p_timeout_event;
}

void DiscoveryClient_ReceiveFSM::setupNotifications()
{
	pEventsClient_ReceiveFSM->registerNotification("Receiving_Ready", ieHandler, "InternalStateChange_To_DiscoveryClient_ReceiveFSM_Receiving_Ready", "EventsClient_ReceiveFSM");
	pEventsClient_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_DiscoveryClient_ReceiveFSM_Receiving_Ready", "EventsClient_ReceiveFSM");
	registerNotification("Receiving_Ready", pEventsClient_ReceiveFSM->getHandler(), "InternalStateChange_To_EventsClient_ReceiveFSM_Receiving_Ready", "DiscoveryClient_ReceiveFSM");
	registerNotification("Receiving", pEventsClient_ReceiveFSM->getHandler(), "InternalStateChange_To_EventsClient_ReceiveFSM_Receiving", "DiscoveryClient_ReceiveFSM");
	iop::Config cfg("~DiscoveryClient");
	cfg.param("system_id", system_id, system_id, true, true, "", p_system_id_map());
	cfg.param("register_own_services", register_own_services, register_own_services, true, false);
	cfg.param("force_component_update_after", p_force_component_update_after, p_force_component_update_after);
	cfg.param("enable_ros_interface", p_enable_ros_interface, p_enable_ros_interface);
	cfg.param("timeout_discover_service", p_timeout_discover_service, p_timeout_discover_service);

	ros::NodeHandle nh;
	p_timeout_timer = nh.createWallTimer(ros::WallDuration(p_current_timeout), &DiscoveryClient_ReceiveFSM::pTimeoutCallback, this, false, false);
	p_timeout_timer.start();
	if (!register_own_services) {
		pRegistrationFinished();
	}
	if (p_enable_ros_interface) {
		p_pub_identification = cfg.advertise<iop_msgs_fkie::Identification>("/iop_identification", 10);
		p_pub_system = cfg.advertise<iop_msgs_fkie::System>("/iop_system", 10, true);
		p_srv_query_ident = cfg.advertiseService("/iop_query_identification", &DiscoveryClient_ReceiveFSM::pQueryIdentificationSrv, this);
		p_srv_update_system = cfg.advertiseService("/iop_update_discovery", &DiscoveryClient_ReceiveFSM::pUpdateSystemSrv, this);
	}
}

std::map<int, std::string> DiscoveryClient_ReceiveFSM::p_system_id_map()
{
	std::map<int, std::string> result;
	result[1] = "System";
	result[2] = "Subsystem";
	result[3] = "Node";
	result[4] = "Component";
	return result;
}

void DiscoveryClient_ReceiveFSM::setDiscoveryFSM(Discovery_ReceiveFSM *discovery_fsm)
{
	p_discovery_fsm = discovery_fsm;
	if (p_discovery_fsm != NULL) {
		if (p_discovery_fsm->getSystemID() == TYPE_SUBSYSTEM) {
			for (unsigned int i = 0; i < p_own_uri_services.size(); i++) {
				iop::DiscoveryServiceDef &service = p_own_uri_services[i];
				p_discovery_fsm->registerService(service.minor_version,
						service.major_version,
						service.service_uri,
						*jausRouter->getJausAddress());
			}
			pRegistrationFinished();
		}
	}
}

void DiscoveryClient_ReceiveFSM::pRegistrationFinished()
{
	p_is_registered = true;
	ROS_INFO_NAMED("DiscoveryClient", "Service registration by discovery service finished!");
	pCheckTimer();
}

void DiscoveryClient_ReceiveFSM::pCheckTimer()
{
	int timeoutts = TIMEOUT_STANDBY;
	if (!p_is_registered || pHasToDiscover(65535)) {
		timeoutts = TIMEOUT_DISCOVER;
		if (p_count_discover_tries > TIMEOUT_STANDBY / TIMEOUT_DISCOVER) {
			if (timeoutts != TIMEOUT_STANDBY) {
				ROS_DEBUG_NAMED("DiscoveryClient", "max tries for discovery services reached, increase timeout");
				timeoutts = TIMEOUT_STANDBY;
			}
		}
	}
	if (p_current_timeout != timeoutts) {
		if (p_current_timeout > timeoutts) {
			ROS_INFO_NAMED("DiscoveryClient", "reduce timeout to %d sec", timeoutts);
		} else {
			ROS_INFO_NAMED("DiscoveryClient", "increase timeout to %d sec", timeoutts);
		}
		p_current_timeout = timeoutts;
		p_timeout_timer.setPeriod(ros::WallDuration(p_current_timeout), true);
		p_timeout_timer.start();
	}
}

bool DiscoveryClient_ReceiveFSM::pQueryIdentificationSrv(iop_msgs_fkie::QueryIdentification::Request  &req,
		iop_msgs_fkie::QueryIdentification::Response &res)
{
	p_check_for_timeout_discovery_service();
	this->query_identification(req.type, 0xFFFF, 0xFF, 0xFF);
	return true;
}

bool DiscoveryClient_ReceiveFSM::pUpdateSystemSrv(std_srvs::Empty::Request  &req, std_srvs::Empty::Response &res)
{
//	p_discovery_srvs_stamps.clear();
	p_check_for_timeout_discovery_service();
	this->query_identification(2, 0xFFFF, 0xFF, 0xFF);
	return true;
}

JausAddress DiscoveryClient_ReceiveFSM::pGetService(ReportServices &msg, iop::DiscoveryServiceDef service, unsigned short subsystem)
{
	ReportServices::Body::NodeList *node_list = msg.getBody()->getNodeList();
	ROS_DEBUG_NAMED("DiscoveryClient", "pGetService search: %s, %d", service.service_uri.c_str(), subsystem);
	for (unsigned int n = 0; n < node_list->getNumberOfElements(); n++) {
		ReportServices::Body::NodeList::NodeSeq *nodes = node_list->getElement(n);
		ReportServices::Body::NodeList::NodeSeq::NodeRec *node = nodes->getNodeRec();
		ReportServices::Body::NodeList::NodeSeq::ComponentList *component_list = nodes->getComponentList();
		for (unsigned int c = 0; c < component_list->getNumberOfElements(); c++) {
			ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq *component_seq = component_list->getElement(c);
			ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ComponentRec *component = component_seq->getComponentRec();
			ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list = component_seq->getServiceList();
			for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
				ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service_rec = service_list->getElement(s);
				ROS_DEBUG_NAMED("DiscoveryClient", "pGetService: %s", service_rec->getURI().c_str());
				if (service.service_uri.compare(service_rec->getURI()) == 0
						&& service.major_version == service_rec->getMajorVersionNumber()
						&& (service.minor_version == service_rec->getMinorVersionNumber() || service.minor_version == 255)) {
					JausAddress address(0);
					address.setSubsystemID(subsystem);
					address.setNodeID(node->getNodeID());
					address.setComponentID(component->getComponentID());
					return address;
				}
			}
		}
	}
	return JausAddress(0);
}

JausAddress DiscoveryClient_ReceiveFSM::pGetService(ReportServiceList &msg, iop::DiscoveryServiceDef service, unsigned short subsystem)
{
	ReportServiceList::Body::SubsystemList *ssys_list = msg.getBody()->getSubsystemList();
	for (unsigned int s = 0; s < ssys_list->getNumberOfElements(); s++) {
		ReportServiceList::Body::SubsystemList::SubsystemSeq *ssystems = ssys_list->getElement(s);
		ReportServiceList::Body::SubsystemList::SubsystemSeq::SubsystemRec *ssystem = ssystems->getSubsystemRec();
		if (ssystem->getSubsystemID() == subsystem) {
			ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList *node_list = ssystems->getNodeList();
			for (unsigned int n = 0; n < node_list->getNumberOfElements(); n++) {
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq *nodes = node_list->getElement(n);
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::NodeRec *node = nodes->getNodeRec();
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList *component_list = nodes->getComponentList();
				for (unsigned int c = 0; c < component_list->getNumberOfElements(); c++) {
					ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq *component_seq = component_list->getElement(c);
					ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ComponentRec *component = component_seq->getComponentRec();
					ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list = component_seq->getServiceList();
					for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
						ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service_rec = service_list->getElement(s);
						if (service.service_uri.compare(service_rec->getURI()) == 0
								&& service.major_version == service_rec->getMajorVersionNumber()
								&& (service.minor_version == service_rec->getMinorVersionNumber() || service.minor_version == 255)) {
							JausAddress address(0);
							address.setSubsystemID(subsystem);
							address.setNodeID(node->getNodeID());
							address.setComponentID(component->getComponentID());
							return address;
						}
					}
				}
			}
		}
	}
	return JausAddress(0);
}

void DiscoveryClient_ReceiveFSM::appendServiceUri(std::string service_uri, unsigned char major_version, unsigned char minor_version)
{
	iop::DiscoveryServiceDef service;
	service.service_uri = service_uri;
	service.minor_version = minor_version;
	service.major_version = major_version;
	if ( std::find(p_own_uri_services.begin(), p_own_uri_services.end(), service) != p_own_uri_services.end() ) {
		ROS_INFO_NAMED("DiscoveryClient", "	%s already known", service_uri.c_str());
	} else {
		ROS_DEBUG_NAMED("DiscoveryClient", "appendServiceUri: %s for registration", service_uri.c_str());
		p_own_uri_services.push_back(service);
		if (register_own_services) {
			p_is_registered = false;
		}
		pCheckTimer();
	}
	for (int i = 0; i < p_discover_services.size(); i++) {
		unsigned short ssid = p_discover_services[i].subsystem;
		JausAddress addr = *(this->jausRouter->getJausAddress());
		if (ssid == 65535) {
			ssid = addr.getSubsystemID();
		}
		if (ssid == addr.getSubsystemID()) {
			if (p_discover_services[i].service.service_uri.compare(service_uri) == 0) {
				// the service was found, forward the address to the callback
				iop::DiscoveryServiceDef service = p_discover_services[i].service;
				ROS_DEBUG_NAMED("DiscoveryClient", "local service '%s' discovered @%s", service.service_uri.c_str(), addr.str().c_str());
				p_discover_services[i].discovered_in.insert(ssid);
				pInformDiscoverCallbacks(service, addr);
			}
		}
	}
}

void DiscoveryClient_ReceiveFSM::pTimeoutCallback(const ros::WallTimerEvent& event)
{
	this->getHandler()->invoke(p_timeout_event);
	// create a new event, since the InternalEventHandler deletes the given.
	p_timeout_event = new InternalEvent("Timeout", "ControlTimeout");
}

void DiscoveryClient_ReceiveFSM::registerAction()
{
	/// Insert User Code HERE
	if (p_first_ready) {
		sendQueryIdentificationAction();
		p_first_ready = false;
	}
}

void DiscoveryClient_ReceiveFSM::handleReportConfigurationAction(ReportConfiguration msg, Receive::Body::ReceiveRec transportData)
{
	ROS_DEBUG_NAMED("DiscoveryClient", "reportedConfigurationAction from %d.%d.%d, not implemented",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
//	if (!class_system_callback_.empty()) {
//		// update current components
//		ROS_INFO_NAMED("DiscoveryClient", "1");
//		mutex_.lock();
//		ROS_INFO_NAMED("DiscoveryClient", " 2");
//		ReportConfiguration::Body::NodeList *node_list = msg.getBody()->getNodeList();
//		ROS_INFO_NAMED("DiscoveryClient", " 3->%d", node_list->getNumberOfElements());
//		for (unsigned int n = 0; n < node_list->getNumberOfElements(); n++) {
//			ReportConfiguration::Body::NodeList::NodeSeq *nodes = node_list->getElement(n);
//			ReportConfiguration::Body::NodeList::NodeSeq::NodeRec *node = nodes->getNodeRec();
//			ReportConfiguration::Body::NodeList::NodeSeq::ComponentList *component_list = nodes->getComponentList();
//			for (unsigned int c = 0; c < component_list->getNumberOfElements(); c++) {
//				ReportConfiguration::Body::NodeList::NodeSeq::ComponentList::ComponentRec *component = component_list->getElement(c);
//				iop_msgs_fkie::JausAddress addr;
//				addr.subsystem_id = transportData.getSrcSubsystemID();
//				addr.node_id = node->getNodeID();
//				addr.component_id = component->getComponentID();
//				if (!pIsKnownComponent(addr)) {
//					ROS_INFO_NAMED("DiscoveryClient", "->%d.%d.%d not known, add", addr.subsystem_id, addr.node_id, addr.component_id);
//					iop_msgs_fkie::Component ros_cmp;
//					ros_cmp.address = addr;
//					p_discovered_components.components.push_back(ros_cmp);
//				} else {
//					ROS_INFO_NAMED("DiscoveryClient", "->%d.%d.%d known!", addr.subsystem_id, addr.node_id, addr.component_id);
//				}
//			}
//		}
//		class_system_callback_(p_discovered_system);
//		mutex_.unlock();
//	}
}

void DiscoveryClient_ReceiveFSM::handleReportIdentificationAction(ReportIdentification msg, Receive::Body::ReceiveRec transportData)
{
	lock_type lock(p_mutex);
	iop_msgs_fkie::Identification ros_msg;
	ros_msg.request_type = msg.getBody()->getReportIdentificationRec()->getQueryType();
	ros_msg.system_type = msg.getBody()->getReportIdentificationRec()->getType();
	ros_msg.name = msg.getBody()->getReportIdentificationRec()->getIdentification();
	unsigned short subsystem_id = transportData.getSourceID()->getSubsystemID();
	JausAddress sender(subsystem_id, transportData.getSourceID()->getNodeID(),transportData.getSourceID()->getComponentID());
	ros_msg.address.subsystem_id = subsystem_id;
	ros_msg.address.node_id = transportData.getSourceID()->getNodeID();
	ros_msg.address.component_id = transportData.getSourceID()->getComponentID();
	ROS_DEBUG_NAMED("DiscoveryClient", "reportedIdentificationAction from sender: %s, request_type: %d, system_type: %d, name: %s",
			sender.str().c_str(), (int)ros_msg.request_type, (int)ros_msg.system_type, ros_msg.name.c_str());
	if (ros_msg.request_type == 1) {
		// update the system identification
		p_discovered_system.name = ros_msg.name;
		p_discovered_system.address = ros_msg.address;
	} else if (ros_msg.request_type == 2) {
		// request nodes of the subsystem
		pUpdateSubsystemIdent(ros_msg.address, msg);
		query_identification(3, subsystem_id, 0xFF, 0xFF);
	} else if (ros_msg.request_type == 3) {
		pUpdateNodeIdent(ros_msg.address, msg);
	}
	if (p_enable_ros_interface) {
		p_pub_identification.publish(ros_msg);
	}
	// doupdate if 1: no service list was received or
	//             2: last update is more then p_force_component_update_after ago
	//             3: max count of ties is not reached
	bool found = p_discovery_srvs_stamps.find(sender.getSubsystemID()) != p_discovery_srvs_stamps.end();
	bool doupdate = not found || ros::WallTime::now().sec - p_discovery_srvs_stamps[sender.getSubsystemID()] > p_force_component_update_after;
	doupdate = doupdate || p_count_discover_tries <= TIMEOUT_STANDBY / TIMEOUT_DISCOVER;
	if ((pHasToDiscover(subsystem_id) || p_enable_ros_interface) && doupdate) {
		// remove the subsystem from discovered systems
		for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
			iop_msgs_fkie::Subsystem &ss= p_discovered_system.subsystems[ssi];
			if (ss.ident.address.subsystem_id == ros_msg.address.subsystem_id) {
				for (unsigned int ni = 0; ni < ss.nodes.size(); ni++) {
					ss.nodes[ni].components.clear();
				}
				//p_discovered_system.subsystems.erase(p_discovered_system.subsystems.begin() + ssi);
				break;
			}
		}
		// request configuration
		QueryConfiguration req_cfg_msg;
		req_cfg_msg.getBody()->getQueryConfigurationRec()->setQueryType(ros_msg.request_type);
		this->sendJausMessage(req_cfg_msg, sender);
		if (!p_recved_service_lists and p_count_queries > 2) {
			ROS_DEBUG_NAMED("DiscoveryClient", "send request for QueryServices for compatibility to v1.0  to %d.%d.%d", sender.getSubsystemID(), sender.getNodeID(), sender.getComponentID());
			QueryServices req_srv_msg;
			this->sendJausMessage(req_srv_msg, sender);
		}
		ROS_DEBUG_NAMED("DiscoveryClient", "send request for QueryServiceList to %d.%d.%d", sender.getSubsystemID(), sender.getNodeID(), sender.getComponentID());
		QueryServiceList req_srvl_msg;
		QueryServiceList::Body::SubsystemList *sslist = req_srvl_msg.getBody()->getSubsystemList();
		QueryServiceList::Body::SubsystemList::SubsystemSeq ssr;
		ssr.getSubsystemRec()->setSubsystemID(65535);
		sslist->addElement(ssr);
		QueryServiceList::Body::SubsystemList::SubsystemSeq *ssrec = sslist->getElement(sslist->getNumberOfElements()-1);
		QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList *nlist = ssrec->getNodeList();
		QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq nr;
		nr.getNodeRec()->setNodeID(255);
		nlist->addElement(nr);
		QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq *nrseq = nlist->getElement(nlist->getNumberOfElements()-1);
		QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList *clist = nrseq->getComponentList();
		QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentRec cr;
		cr.setComponentID(255);
		clist->addElement(cr);
		this->sendJausMessage(req_srvl_msg, sender);
		p_count_queries += 1;
	}
	p_discovery_srvs_stamps[sender.getSubsystemID()] = ros::WallTime::now().sec;
}

void DiscoveryClient_ReceiveFSM::handleReportServiceListAction(ReportServiceList msg, Receive::Body::ReceiveRec transportData)
{
	p_recved_service_lists = true;
	p_count_queries = 0;
	JausAddress sender = transportData.getAddress();
	ROS_DEBUG_NAMED("DiscoveryClient", "handleReportServiceListAction from sender: %s", sender.str().c_str());
	p_discovery_srvs_stamps[sender.getSubsystemID()] = ros::WallTime::now().sec;
	// This message is received after own services are registered -> test is service registered?
	// or after QueryServices was send
	if (p_on_registration) {
		// -> test is service registered?
		std::set<std::string> services_registered;
		// create a list with all service URI's associated with own JAUS address
		ReportServiceList::Body::SubsystemList *ssys_list = msg.getBody()->getSubsystemList();
		for (unsigned int s = 0; s < ssys_list->getNumberOfElements(); s++) {
			ReportServiceList::Body::SubsystemList::SubsystemSeq *ssystems = ssys_list->getElement(s);
			ReportServiceList::Body::SubsystemList::SubsystemSeq::SubsystemRec *ssystem = ssystems->getSubsystemRec();
			if (ssystem->getSubsystemID() == jausRouter->getJausAddress()->getSubsystemID()) {
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList *node_list = ssystems->getNodeList();
				for (unsigned int n = 0; n < node_list->getNumberOfElements(); n++) {
					ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq *nodes = node_list->getElement(n);
					ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::NodeRec *node = nodes->getNodeRec();
					if (node->getNodeID() == jausRouter->getJausAddress()->getNodeID()) {
						ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList *component_list = nodes->getComponentList();
						for (unsigned int c = 0; c < component_list->getNumberOfElements(); c++) {
							ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq *component_seq = component_list->getElement(c);
							ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ComponentRec *component = component_seq->getComponentRec();
							if (component->getComponentID() == jausRouter->getJausAddress()->getComponentID()) {
								ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list = component_seq->getServiceList();
								for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
									ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service = service_list->getElement(s);
									services_registered.insert(service->getURI());
								}
							}
						}
					}
				}
			}
		}
		p_on_registration = false;
		p_is_registered = true;
		if (register_own_services) {
			// test all own URI's are in the registered list
			for (unsigned int i = 0; i < p_own_uri_services.size(); i++) {
				if (std::find(services_registered.begin(), services_registered.end(), p_own_uri_services[i].service_uri) == services_registered.end()) {
					ROS_WARN_NAMED("DiscoveryClient", "own service '%s' not found in reported services! retry registration...",
							p_own_uri_services[i].service_uri.c_str());
					p_is_registered = false;
					p_on_registration = true;
				}
			}
			if (p_is_registered) {
				ROS_INFO_NAMED("DiscoveryClient", "all services are registered!");
			}
		}
		if (p_is_registered) {
			pRegistrationFinished();
		}
	}

	// Create ROS message from components
	if (p_enable_ros_interface) {
		// update the object with discovered system and publish it to ROS
		lock_type lock(p_mutex);
		iop_msgs_fkie::JausAddress node_addr;
		node_addr.subsystem_id = transportData.getSourceID()->getSubsystemID();
		node_addr.node_id = transportData.getSourceID()->getNodeID();
		node_addr.component_id = transportData.getSourceID()->getComponentID();
		ReportServiceList::Body::SubsystemList *ssys_list = msg.getBody()->getSubsystemList();
		for (unsigned int s = 0; s < ssys_list->getNumberOfElements(); s++) {
			ReportServiceList::Body::SubsystemList::SubsystemSeq *ssystems = ssys_list->getElement(s);
			ReportServiceList::Body::SubsystemList::SubsystemSeq::SubsystemRec *ssystem = ssystems->getSubsystemRec();
			ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList *node_list = ssystems->getNodeList();
			for (unsigned int n = 0; n < node_list->getNumberOfElements(); n++) {
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq *nodes = node_list->getElement(n);
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::NodeRec *node = nodes->getNodeRec();
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList *component_list = nodes->getComponentList();
				for (unsigned int c = 0; c < component_list->getNumberOfElements(); c++) {
					ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq *component_seq = component_list->getElement(c);
					ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ComponentRec *component = component_seq->getComponentRec();
					iop_msgs_fkie::JausAddress addr;
					addr.subsystem_id = transportData.getSourceID()->getSubsystemID();
					addr.node_id = node->getNodeID();
					addr.component_id = component->getComponentID();
					pUpdateComponent(node_addr, addr, component_seq->getServiceList());
				}
			}
		}
		p_pub_system.publish(p_discovered_system);
	}
	// check for services to discover, but only if a discovery_handler is set
	lock_type lock(p_mutex);
	for (int i = p_discover_services.size()-1; i >= 0; --i) {
		if (!p_discover_services[i].discovered(transportData.getSourceID()->getSubsystemID())) {
			ROS_DEBUG_NAMED("DiscoveryClient", "  discover %s, subsystem: %d", p_discover_services[i].service.service_uri.c_str(), p_discover_services[i].subsystem);
		}
	}
	for (int i = 0; i < p_discover_services.size(); i++) {
//		if (!p_discover_services[i].discovered) {
			unsigned short ssid = p_discover_services[i].subsystem;
			if (ssid == 65535) {
				ssid = transportData.getSourceID()->getSubsystemID();
			}
			if (ssid == transportData.getSourceID()->getSubsystemID()) {
				JausAddress addr = this->pGetService(msg, p_discover_services[i].service, ssid);
				if (addr.get() != 0) {
					// the service was found, forward the address to the callback
					iop::DiscoveryServiceDef service = p_discover_services[i].service;
					ROS_DEBUG_NAMED("DiscoveryClient", "service '%s' discovered @%d.%d.%d through service list", service.service_uri.c_str(), addr.getSubsystemID(), addr.getNodeID(), addr.getComponentID());
					p_discover_services[i].discovered_in.insert(transportData.getSourceID()->getSubsystemID());
					pInformDiscoverCallbacks(service, addr);
				} else {
					p_discover_services[i].discovered_in.erase(transportData.getSourceID()->getSubsystemID());
				}
			}
//		}
	}
	pCheckTimer();
}

void DiscoveryClient_ReceiveFSM::handleReportServicesAction(ReportServices msg, Receive::Body::ReceiveRec transportData)
{
	ROS_DEBUG_NAMED("DiscoveryClient", "handleReportServicesAction from sender: subsystem: %d, node: %d, component: %d",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	/// Insert User Code HERE
	// This message is received after own services are registered -> test is service registered?
	// or after QueryServices was send
	if (p_on_registration) {
		// -> test is service registered?
		std::set<std::string> services_registered;
		// create a list with all service URI's associated with own JAUS address
		ReportServices::Body::NodeList *node_list = msg.getBody()->getNodeList();
		for (unsigned int n = 0; n < node_list->getNumberOfElements(); n++) {
			ReportServices::Body::NodeList::NodeSeq *nodes = node_list->getElement(n);
			ReportServices::Body::NodeList::NodeSeq::NodeRec *node = nodes->getNodeRec();
			if (node->getNodeID() == jausRouter->getJausAddress()->getNodeID()) {
				ReportServices::Body::NodeList::NodeSeq::ComponentList *component_list = nodes->getComponentList();
				for (unsigned int c = 0; c < component_list->getNumberOfElements(); c++) {
					ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq *component_seq = component_list->getElement(c);
					ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ComponentRec *component = component_seq->getComponentRec();
					if (component->getComponentID() == jausRouter->getJausAddress()->getComponentID()) {
						ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list = component_seq->getServiceList();
						for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
							ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service = service_list->getElement(s);
							services_registered.insert(service->getURI());
						}
					}
				}
			}
		}
		p_on_registration = false;
		p_is_registered = true;
		if (register_own_services) {
			// test all own URI's are in the registered list
			for (unsigned int i = 0; i < p_own_uri_services.size(); i++) {
				if (std::find(services_registered.begin(), services_registered.end(), p_own_uri_services[i].service_uri) == services_registered.end()) {
					ROS_WARN_NAMED("DiscoveryClient", "own service '%s' not found in reported services! retry registration...",
							p_own_uri_services[i].service_uri.c_str());
					p_is_registered = false;
					p_on_registration = true;
				}
			}
			if (p_is_registered) {
				ROS_INFO_NAMED("DiscoveryClient", "all services are registered!");
			}
		}
		if (p_is_registered) {
			pRegistrationFinished();
		}
	}

	// Create ROS message from components
	if (p_enable_ros_interface) {
		// update the object with discovered system and publish it to ROS
		lock_type lock(p_mutex);
		iop_msgs_fkie::JausAddress node_addr;
		node_addr.subsystem_id = transportData.getSourceID()->getSubsystemID();
		node_addr.node_id = transportData.getSourceID()->getNodeID();
		node_addr.component_id = transportData.getSourceID()->getComponentID();
		ReportServices::Body::NodeList *node_list = msg.getBody()->getNodeList();
		for (unsigned int n = 0; n < node_list->getNumberOfElements(); n++) {
			ReportServices::Body::NodeList::NodeSeq *nodes = node_list->getElement(n);
			ReportServices::Body::NodeList::NodeSeq::NodeRec *node = nodes->getNodeRec();
			ReportServices::Body::NodeList::NodeSeq::ComponentList *component_list = nodes->getComponentList();
			for (unsigned int c = 0; c < component_list->getNumberOfElements(); c++) {
				ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq *component_seq = component_list->getElement(c);
				ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ComponentRec *component = component_seq->getComponentRec();
				iop_msgs_fkie::JausAddress addr;
				addr.subsystem_id = transportData.getSourceID()->getSubsystemID();
				addr.node_id = node->getNodeID();
				addr.component_id = component->getComponentID();
				pUpdateComponent(node_addr, addr, component_seq->getServiceList());
			}
		}
		p_pub_system.publish(p_discovered_system);
	}
	// check for services to discover, but only if a discovery_handler is set
	lock_type lock(p_mutex);
	for (int i = p_discover_services.size()-1; i >= 0; --i) {
		if (!p_discover_services[i].discovered(transportData.getSourceID()->getSubsystemID())) {
			ROS_DEBUG_NAMED("DiscoveryClient", "  discover %s, subsystem: %d", p_discover_services[i].service.service_uri.c_str(), p_discover_services[i].subsystem);
		}
	}
	for (int i = 0; i < p_discover_services.size(); i++) {
//		if (!p_discover_services[i].discovered) {
			unsigned short ssid = p_discover_services[i].subsystem;
			if (ssid == 65535) {
				ssid = transportData.getSourceID()->getSubsystemID();
			}
			if (ssid == transportData.getSourceID()->getSubsystemID()) {
				JausAddress addr = this->pGetService(msg, p_discover_services[i].service, ssid);
				if (addr.get() != 0) {
					// the service was found, forward the address to the callback
					iop::DiscoveryServiceDef service = p_discover_services[i].service;
					ROS_DEBUG_NAMED("DiscoveryClient", "service '%s' discovered @%d.%d.%d through services old stile, using QueryServices", service.service_uri.c_str(), addr.getSubsystemID(), addr.getNodeID(), addr.getComponentID());
					p_discover_services[i].discovered_in.insert(transportData.getSourceID()->getSubsystemID());
					pInformDiscoverCallbacks(service, addr);
				} else {
					p_discover_services[i].discovered_in.erase(transportData.getSourceID()->getSubsystemID());
				}
			}
//		}
	}
	pCheckTimer();
}

void DiscoveryClient_ReceiveFSM::handleReportSubsystemListAction(ReportSubsystemList msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	ROS_DEBUG_NAMED("DiscoveryClient", "reportedSubsystemList from sender: subsystem: %d, node: %d,	component: %d",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	ReportSubsystemList::Body::SubsystemList *dis_subsystems = msg.getBody()->getSubsystemList();
	ROS_DEBUG_NAMED("DiscoveryClient", "  known subsystems: %u", dis_subsystems->getNumberOfElements());
	for (int i=0; i < dis_subsystems->getNumberOfElements(); i++) {
		ReportSubsystemList::Body::SubsystemList::SubsystemRec *item = dis_subsystems->getElement(i);
		ROS_DEBUG_NAMED("DiscoveryClient", "  subsystem: %d, node: %d, component: %d",
				item->getSubsystemID(),item->getNodeID(), item->getComponentID());
	}
}

void DiscoveryClient_ReceiveFSM::sendQueryIdentificationAction()
{
	// remove expired subsystems (not responding to query identification messages)
	p_check_for_timeout_discovery_service();
	if (!(pHasToDiscover(65535) || p_enable_ros_interface)) {
		// if we descovered all services and have no enabled ROS interface we send only subsystem queries
		query_identification(TYPE_SUBSYSTEM, 0xFFFF, 0xFF, 0xFF);
		return;
	}
	int query_type = TYPE_SUBSYSTEM;
	unsigned short subsystem_id = jausRouter->getJausAddress()->getSubsystemID();
	if (system_id == TYPE_SUBSYSTEM) {
		// we are subsystem -> discover system
		query_type = TYPE_SYSTEM;
		subsystem_id = 65535; //0xFFFF
	} else if (system_id == TYPE_NODE or
		// we are node -> find subsystem discovery service and register own services
		// if this discovery service is included in a component.
			system_id == TYPE_COMPONENT) {
		// IOP 5a: A QueryIdentification message shall be broadcast by
		// every JAUS component on the subsystem at a rate of at least once per 5 minutes for the
		// purpose of finding and registering services with the Discovery service.

		// is set by default...
	}
	query_identification(TYPE_SUBSYSTEM, 0xFFFF, 0xFF, 0xFF);
	if (query_type != TYPE_SUBSYSTEM) {
		query_identification(query_type, subsystem_id, 0xFF, 0xFF);
		// send query for services to discover, if these are not in the same subsystem
		if (p_discover_services.size() > 0) {
			std::set<int> subsystems;
			subsystems.insert(subsystem_id);
			lock_type lock(p_mutex);
			for (unsigned int i = 0; i < p_discover_services.size(); i++) {
				if (!p_discover_services[i].discovered()) {
					subsystem_id = p_discover_services[i].subsystem;
					if (subsystems.count(subsystem_id) == 0) {
						subsystems.insert(subsystem_id);
						ROS_DEBUG_NAMED("DiscoveryClient", "send QueryServices to subsystem: %d for discover service: %s",
								subsystem_id, p_discover_services[i].service.service_uri.c_str());
						query_identification(query_type, subsystem_id, 0xFF, 0xFF);
		//				QueryIdentification msg;
		//				msg.getBody()->getQueryIdentificationRec()->setQueryType(TYPE_SUBSYSTEM);
		//				printf("[DiscoveryClient] send QueryIdentification to subsystem: %d of type: %d (SUBSYSTEM)\n", subsystem_id, TYPE_SUBSYSTEM);
		//				sendJausMessage(msg,JausAddress(subsystem_id, 0xFF, 0xFF)); //0xFFFF, 0xFF, 0xFF
					}
				}
			}
		}
	}
	if (p_count_discover_tries <= TIMEOUT_STANDBY / TIMEOUT_DISCOVER) {
		p_count_discover_tries++;
	}

	p_timeout_timer.start();
}

void DiscoveryClient_ReceiveFSM::sendRegisterServicesAction(ReportIdentification msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	int query_type = msg.getBody()->getReportIdentificationRec()->getQueryType();
	ROS_DEBUG_NAMED("DiscoveryClient", "sendRegisterServices query_type: %d, sender: subsystem: %d, node: %d,	component: %d",
			query_type, transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	if (p_own_uri_services.size() == 0) {
		// do not register, if we have no services to register setted by appendServiceUri()
		handleReportIdentificationAction(msg, transportData);
		return;
	}
	if (query_type == TYPE_SUBSYSTEM) {
		if (this->jausRouter->getJausAddress()->getSubsystemID() == transportData.getSourceID()->getSubsystemID()) {
			if (!onRegistration() && !isRegistered()) {
				p_on_registration = true;
				p_addr_discovery_service = transportData.getAddress();
				ROS_INFO_NAMED("DiscoveryClient", "register services by [%s]", p_addr_discovery_service.str().c_str());
				ROS_DEBUG_NAMED("DiscoveryClient", "Discovery service info:");
				ROS_DEBUG_NAMED("DiscoveryClient", "  QueryType: %d", msg.getBody()->getReportIdentificationRec()->getQueryType());
				ROS_DEBUG_NAMED("DiscoveryClient", "  Type     : %d", msg.getBody()->getReportIdentificationRec()->getType());
				ROS_DEBUG_NAMED("DiscoveryClient", "  ID       : %s", msg.getBody()->getReportIdentificationRec()->getIdentification().c_str());
				RegisterServices msg_reg_services;
				RegisterServices::RegisterServicesBody::ServiceList list_services;
				for (unsigned int i = 0; i < p_own_uri_services.size(); i++) {
					iop::DiscoveryServiceDef &service = p_own_uri_services[i];
					RegisterServices::RegisterServicesBody::ServiceList::ServiceRec service_rec;
					service_rec.setMinorVersionNumber(service.minor_version);
					service_rec.setMajorVersionNumber(service.major_version);
					service_rec.setURI(service.service_uri);
					list_services.addElement(service_rec);
				}
				msg_reg_services.getRegisterServicesBody()->setServiceList(list_services);
				sendJausMessage(msg_reg_services, p_addr_discovery_service);

				ROS_DEBUG_NAMED("DiscoveryClient", "send QueryServices to validate the registration");
				QueryServices msg;
				QueryServices::Body::NodeList nlist;
				QueryServices::Body::NodeList::NodeSeq nseq;
				QueryServices::Body::NodeList::NodeSeq::NodeRec nreq;
				QueryServices::Body::NodeList::NodeSeq::ComponentList clist;
				QueryServices::Body::NodeList::NodeSeq::ComponentList::ComponentRec creq;

				nreq.setNodeID(jausRouter->getJausAddress()->getNodeID());
				creq.setComponentID(jausRouter->getJausAddress()->getComponentID());
				clist.addElement(creq);
				nseq.setComponentList(clist);
				nseq.setNodeRec(nreq);
				nlist.addElement(nseq);
				msg.getBody()->setNodeList(nlist);
				sendJausMessage(msg, p_addr_discovery_service);
			}
		} else {
			ROS_DEBUG_NAMED("DiscoveryClient", "  skip registration on this discovery service because of different subsystem!");
		}
	} else if (query_type == TYPE_SYSTEM) {
		// add a discovery service for ReportSubsystemLIst
		if (p_discovery_fsm != NULL) {
			JausAddress addr(transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
			p_discovery_fsm->registerSubsystem(addr);
		}
	}
}

void DiscoveryClient_ReceiveFSM::p_check_for_timeout_discovery_service()
{
	if (p_timeout_discover_service > 0) {
		std::vector<unsigned short> to_remove;
		std::map<unsigned short, unsigned int>::iterator it;
		unsigned int now = ros::WallTime::now().sec;
		for (it = p_discovery_srvs_stamps.begin(); it != p_discovery_srvs_stamps.end(); it++) {
			if (it->second > 0) {
				if (now - it->second > p_timeout_discover_service) {
					// remove the subsystem from our system published to ROS
					if (p_enable_ros_interface){
						bool changed = false;
						iop_msgs_fkie::System::_subsystems_type::iterator itsubs;
						for (itsubs = p_discovered_system.subsystems.begin(); itsubs != p_discovered_system.subsystems.end(); itsubs++) {
							if (itsubs->ident.address.subsystem_id == it->first) {
								p_discovered_system.subsystems.erase(itsubs);
								changed = true;
								break;
							}
						}
						if (changed) {
							p_pub_system.publish(p_discovered_system);
						}
					}
					// it was the discovery service where we register our services. We need to register again.
					if (it->first == p_addr_discovery_service.getSubsystemID()) {
						if (p_is_registered) {
							ROS_WARN_NAMED("DiscoveryClient", "discovery service timeout, set service to not registered!");
						}
						p_is_registered = false;
					}
					to_remove.push_back(it->first);
				}
			}
		}
		// remove expired subsystems
		for (unsigned int i = 0; i < to_remove.size(); i++) {
			p_discovery_srvs_stamps.erase(p_discovery_srvs_stamps.find(to_remove[i]));
		}
	}
}

bool DiscoveryClient_ReceiveFSM::isRegistered()
{
	p_check_for_timeout_discovery_service();
	return p_is_registered || (p_own_uri_services.size() == 0);
}

bool DiscoveryClient_ReceiveFSM::onRegistration()
{
	/// Insert User Code HERE
	return p_on_registration;
}

void DiscoveryClient_ReceiveFSM::pDiscover(std::string service_uri, unsigned char major_version, unsigned char minor_version, unsigned short subsystem)
{
	p_count_discover_tries = 0;
	iop::DiscoveryServiceDef service(service_uri, major_version, minor_version);
	JausAddress own_addr = *(this->jausRouter->getJausAddress());
	bool own_found = false;
	lock_type lock(p_mutex);
	for (unsigned int i = 0; i < p_discover_services.size(); i++) {
		if (p_discover_services[i].service == service
				&& p_discover_services[i].subsystem == subsystem) {
			ROS_DEBUG_NAMED("DiscoveryClient", "%s %d.%d for subsystem: %d already in discover, skip", service_uri.c_str(), major_version, minor_version, subsystem);
			p_discover_services[i].discovered_in.clear();
			return;
		}
	}
	ROS_DEBUG_NAMED("DiscoveryClient", "added %s %d.%d for subsystem: %d to discovery", service_uri.c_str(), major_version, minor_version, subsystem);
	DiscoverItem di;
	di.service = service;
	di.subsystem = subsystem;
	// search in own services
	unsigned short ssid = subsystem;
	if (ssid == 65535) {
		ssid = own_addr.getSubsystemID();
	}
	if (ssid == own_addr.getSubsystemID()) {
		if ( std::find(p_own_uri_services.begin(), p_own_uri_services.end(), service) != p_own_uri_services.end() ) {
			ROS_DEBUG_NAMED("DiscoveryClient", "	discovered own service %s", service_uri.c_str());
			di.discovered_in.insert(ssid);
			own_found = true;
		}
	}
	p_discover_services.push_back(di);
	if (own_found) {
		pInformDiscoverCallbacks(service, own_addr);
	}
//	int query_type = TYPE_SUBSYSTEM;
//	unsigned short subsystem_id = subsystem;
//	QueryIdentification msg;
//	msg.getBody()->getQueryIdentificationRec()->setQueryType(query_type);
//	ROS_DEBUG_NAMED("DiscoveryClient", "discover, send QueryIdentification to subsystem: %d", subsystem_id);
//	sendJausMessage(msg,JausAddress(subsystem_id, 0xFF, 0xFF)); //0xFFFF, 0xFF, 0xFF
	pCheckTimer();
}

void DiscoveryClient_ReceiveFSM::cancel_discovery()
{
	lock_type lock(p_mutex);
	p_discover_services.clear();
}

void DiscoveryClient_ReceiveFSM::cancel_discovery(std::string service_uri, unsigned char major_version, unsigned char minor_version)
{
	lock_type lock(p_mutex);
	iop::DiscoveryServiceDef service(service_uri, major_version, minor_version);
	for (unsigned int i = 0; i < p_discover_services.size(); i++) {
		if (p_discover_services[i].service == service) {
			ROS_DEBUG_NAMED("DiscoveryClient", "remove %s from discover", service_uri.c_str());
			p_discover_services.erase(p_discover_services.begin() + i);
			return;
		}
	}
}

void DiscoveryClient_ReceiveFSM::query_identification(int query_type, jUnsignedShortInteger subsystem, jUnsignedByte node, jUnsignedByte component)
{
	QueryIdentification msg;
	msg.getBody()->getQueryIdentificationRec()->setQueryType(query_type);
	ROS_DEBUG_NAMED("DiscoveryClient", "send QueryIdentification to subsystem.node.comp: %i.%d.%d of type %d, next query in %i sec",
			subsystem, (int)node, (int)component, query_type, p_current_timeout);
	sendJausMessage(msg,JausAddress(subsystem, node, component)); //0xFFFF, 0xFF, 0xFF
}

bool DiscoveryClient_ReceiveFSM::pIsKnownComponent(iop_msgs_fkie::JausAddress &addr)
{
	lock_type lock(p_mutex);
	for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
		for (unsigned int ni = 0; ni < p_discovered_system.subsystems[ssi].nodes.size(); ni++) {
			for (unsigned int ci = 0; ci < p_discovered_system.subsystems[ssi].nodes[ni].components.size(); ci++) {
				iop_msgs_fkie::JausAddress kaddr = p_discovered_system.subsystems[ssi].nodes[ni].components[ci].address;
				ROS_INFO_NAMED("DiscoveryClient", " compare: %d.%d.%d -- %d.%d.%d", kaddr.subsystem_id, kaddr.node_id, kaddr.component_id, addr.subsystem_id, addr.node_id, addr.component_id);
				if (kaddr.subsystem_id == addr.subsystem_id
						&& kaddr.node_id == addr.node_id
						&& kaddr.component_id == addr.component_id) {
					ROS_INFO_NAMED("DiscoveryClient", " ==");
					return true;
				}
			}
		}
	}
	return false;
}

bool DiscoveryClient_ReceiveFSM::pEqualIdent(iop_msgs_fkie::Identification &ros_ident, iop_msgs_fkie::JausAddress &addr)
{
	if (ros_ident.address.subsystem_id == addr.subsystem_id
			&& ros_ident.address.node_id == addr.node_id
			&& ros_ident.address.component_id == addr.component_id) {
		return true;
	}
	return false;
}

void DiscoveryClient_ReceiveFSM::pUpdateSubsystemIdent(iop_msgs_fkie::JausAddress &addr, ReportIdentification &report_ident)
{
	lock_type lock(p_mutex);
	// update the node identification
	bool ss_updated = false;
	for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
		iop_msgs_fkie::Subsystem &ss= p_discovered_system.subsystems[ssi];
		if (pEqualIdent(ss.ident, addr)) {
			ss_updated = true;
			ss.ident.name = report_ident.getBody()->getReportIdentificationRec()->getIdentification();
		}
	}
	if (! ss_updated) {
		iop_msgs_fkie::Subsystem ss_new;
		ss_new.ident.name = report_ident.getBody()->getReportIdentificationRec()->getIdentification();
		ss_new.ident.request_type = report_ident.getBody()->getReportIdentificationRec()->getQueryType();
		ss_new.ident.system_type = report_ident.getBody()->getReportIdentificationRec()->getType();
		ss_new.ident.address.subsystem_id = addr.subsystem_id;
		ss_new.ident.address.node_id = addr.node_id;
		ss_new.ident.address.component_id = addr.component_id;
		p_discovered_system.subsystems.push_back(ss_new);
	}
}

void DiscoveryClient_ReceiveFSM::pUpdateNodeIdent(iop_msgs_fkie::JausAddress &addr, ReportIdentification &report_ident)
{
	lock_type lock(p_mutex);
	// update the node identification
	for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
		iop_msgs_fkie::Subsystem &ss= p_discovered_system.subsystems[ssi];
		if (ss.ident.address.subsystem_id == addr.subsystem_id) {
			bool node_updated = false;
			for (unsigned int ni = 0; ni < p_discovered_system.subsystems[ssi].nodes.size(); ni++) {
				iop_msgs_fkie::Node &node= ss.nodes[ni];
				if (pEqualIdent(node.ident, addr)) {
					node_updated = true;
					node.ident.name = report_ident.getBody()->getReportIdentificationRec()->getIdentification();
				}
			}
			if (! node_updated) {
				iop_msgs_fkie::Node node_new;
				node_new.ident.name = report_ident.getBody()->getReportIdentificationRec()->getIdentification();
				node_new.ident.request_type = report_ident.getBody()->getReportIdentificationRec()->getQueryType();
				node_new.ident.system_type = report_ident.getBody()->getReportIdentificationRec()->getType();
				node_new.ident.address.subsystem_id = addr.subsystem_id;
				node_new.ident.address.node_id = addr.node_id;
				node_new.ident.address.component_id = addr.component_id;
				ss.nodes.push_back(node_new);
			}
		}
	}
}

void DiscoveryClient_ReceiveFSM::pUpdateComponent(iop_msgs_fkie::JausAddress &node_addr, iop_msgs_fkie::JausAddress &addr, ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list)
{
	// update the service list for given component
	lock_type lock(p_mutex);
	bool cmp_updated = false;
	for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
		if (p_discovered_system.subsystems[ssi].ident.address.subsystem_id == addr.subsystem_id) {
			for (unsigned int ni = 0; ni < p_discovered_system.subsystems[ssi].nodes.size(); ni++) {
				if (p_discovered_system.subsystems[ssi].nodes[ni].ident.address.node_id == addr.node_id) {
					for (unsigned int ci = 0; ci < p_discovered_system.subsystems[ssi].nodes[ni].components.size(); ci++) {
						iop_msgs_fkie::Component &cmp = p_discovered_system.subsystems[ssi].nodes[ni].components[ci];
						iop_msgs_fkie::JausAddress kaddr = cmp.address;
						if (kaddr.subsystem_id == addr.subsystem_id
								&& kaddr.node_id == addr.node_id
								&& kaddr.component_id == addr.component_id) {
							// update service
							cmp.services.clear();
							for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
								ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service = service_list->getElement(s);
								iop_msgs_fkie::Service srv;
								srv.uri = service->getURI();
								srv.major_version = service->getMajorVersionNumber();
								srv.minor_version = service->getMinorVersionNumber();
								cmp.services.push_back(srv);
							}
							cmp_updated = true;
						}
					}
				}
			}
		}
	}
	if (! cmp_updated) {
		// add the service to new component. The node must exists.
		for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
			iop_msgs_fkie::Subsystem &ss= p_discovered_system.subsystems[ssi];
			bool node_updated = false;
			for (unsigned int ni = 0; ni < p_discovered_system.subsystems[ssi].nodes.size(); ni++) {
				iop_msgs_fkie::Node &node= ss.nodes[ni];
				if (pEqualIdent(node.ident, node_addr)) {
					iop_msgs_fkie::Component ros_cmp;
					ros_cmp.address = addr;
					for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
						ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service = service_list->getElement(s);
						iop_msgs_fkie::Service srv;
						srv.uri = service->getURI();
						srv.major_version = service->getMajorVersionNumber();
						srv.minor_version = service->getMinorVersionNumber();
						ros_cmp.services.push_back(srv);
					}
					node.components.push_back(ros_cmp);
				}
			}
		}
	}
}

void DiscoveryClient_ReceiveFSM::pUpdateComponent(iop_msgs_fkie::JausAddress &node_addr, iop_msgs_fkie::JausAddress &addr, ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list)
{
	// update the service list for given component
	lock_type lock(p_mutex);
	bool cmp_updated = false;
	for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
		for (unsigned int ni = 0; ni < p_discovered_system.subsystems[ssi].nodes.size(); ni++) {
			for (unsigned int ci = 0; ci < p_discovered_system.subsystems[ssi].nodes[ni].components.size(); ci++) {
				iop_msgs_fkie::Component &cmp = p_discovered_system.subsystems[ssi].nodes[ni].components[ci];
				iop_msgs_fkie::JausAddress kaddr = cmp.address;
				if (kaddr.subsystem_id == addr.subsystem_id
						&& kaddr.node_id == addr.node_id
						&& kaddr.component_id == addr.component_id) {
					// update service
					cmp.services.clear();
					for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
						ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service = service_list->getElement(s);
						iop_msgs_fkie::Service srv;
						srv.uri = service->getURI();
						srv.major_version = service->getMajorVersionNumber();
						srv.minor_version = service->getMinorVersionNumber();
						cmp.services.push_back(srv);
					}
					cmp_updated = true;
				}
			}
		}
	}
	if (! cmp_updated) {
		// add the service to new component. The node must exists.
		for (unsigned int ssi = 0; ssi < p_discovered_system.subsystems.size(); ssi++) {
			iop_msgs_fkie::Subsystem &ss= p_discovered_system.subsystems[ssi];
			bool node_updated = false;
			for (unsigned int ni = 0; ni < p_discovered_system.subsystems[ssi].nodes.size(); ni++) {
				iop_msgs_fkie::Node &node= ss.nodes[ni];
				if (pEqualIdent(node.ident, node_addr)) {
					iop_msgs_fkie::Component ros_cmp;
					ros_cmp.address = addr;
					for (unsigned int s = 0; s < service_list->getNumberOfElements(); s++) {
						ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec *service = service_list->getElement(s);
						iop_msgs_fkie::Service srv;
						srv.uri = service->getURI();
						srv.major_version = service->getMajorVersionNumber();
						srv.minor_version = service->getMinorVersionNumber();
						ros_cmp.services.push_back(srv);
					}
					node.components.push_back(ros_cmp);
				}
			}
		}
	}
}

bool DiscoveryClient_ReceiveFSM::pHasToDiscover(unsigned short subsystem_id)
{
	for (int i = p_discover_services.size()-1; i >= 0; --i) {
		if (subsystem_id == 65535) {
			// go through all discovered robots
			for (unsigned int s = 0; s < p_discovered_system.subsystems.size(); s++) {
				unsigned short sid = p_discovered_system.subsystems[s].ident.address.subsystem_id;
				if (!p_discover_services[i].discovered(sid)) {
					return true;
				}
			}
		} else {
			if (!p_discover_services[i].discovered(subsystem_id)) {
				return true;
			}
		}
	}
	return false;
}

void DiscoveryClient_ReceiveFSM::pInformDiscoverCallbacks(iop::DiscoveryServiceDef &service, JausAddress &address)
{
	std::map <iop::DiscoveryServiceDef, std::vector<boost::function<void (const std::string &, JausAddress &)> > > ::iterator it;
	it = p_discover_callbacks.find(service);
	if (it != p_discover_callbacks.end()) {
		for (unsigned int i = 0; i < it->second.size(); i++) {
			it->second[i](service.service_uri, address);
		}
//		it->second.clear();
//		p_discover_callbacks.erase(it);
	}
	if (!class_discovery_callback_.empty()) {
		class_discovery_callback_(service.service_uri, address);
	}

}

};
