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
	TIMEOUT_DISCOVER = 5;
	TIMEOUT_STANDBY = 60;
	p_current_timeout = TIMEOUT_STANDBY;
	p_discovery_fsm = NULL;
	p_first_ready = true;
	p_is_registered = false;
	p_on_registration = false;
	p_count_discover_tries = 0;
	p_count_queries = 0;
	p_recved_service_lists = false;
	p_timeout_event = new InternalEvent("Timeout", "ControlTimeout");
	p_pnh = ros::NodeHandle("~");
	p_enable_ros_interface = false;
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
	p_discovery_config.update_ros_parameter();
	p_timeout_timer = p_nh.createWallTimer(ros::WallDuration(p_current_timeout), &DiscoveryClient_ReceiveFSM::pTimeoutCallback, this, false, false);
	if (!p_discovery_config.register_own_services) {
		pRegistrationFinished();
	} else {
		p_timeout_timer.start();
	}
	p_pnh.param("enable_ros_interface", p_enable_ros_interface, p_enable_ros_interface);
	ROS_INFO_STREAM("[DiscoveryClientService] enable_ros_interface: " << p_enable_ros_interface);
	if (p_enable_ros_interface) {
		p_pub_identification = p_nh.advertise<iop_msgs_fkie::Identification>("/iop_identification", 10);
		p_pub_system = p_nh.advertise<iop_msgs_fkie::System>("/iop_system", 10, true);
		p_srv_query_ident = p_nh.advertiseService("/iop_query_identification", &DiscoveryClient_ReceiveFSM::pQueryIdentificationSrv, this);
		p_srv_update_system = p_nh.advertiseService("/iop_update_discovery", &DiscoveryClient_ReceiveFSM::pUpdateSystemSrv, this);
	}
}

void DiscoveryClient_ReceiveFSM::setDiscoveryFSM(Discovery_ReceiveFSM *discovery_fsm)
{
	p_discovery_fsm = discovery_fsm;
	if (p_discovery_fsm != NULL) {
		if (p_discovery_fsm->getSystemID() == discovery_config::TYPE_SUBSYSTEM) {
			for (unsigned int i = 0; i < p_own_uri_services.size(); i++) {
				ServiceDef &service = p_own_uri_services[i];
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
	if (!p_is_registered || pHasToDiscover()) {
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
	this->query_identification(req.type, 0xFFFF, 0xFF, 0xFF);
	return true;
}

bool DiscoveryClient_ReceiveFSM::pUpdateSystemSrv(std_srvs::Empty::Request  &req, std_srvs::Empty::Response &res)
{
	this->query_identification(2, 0xFFFF, 0xFF, 0xFF);
	return true;
}

JausAddress DiscoveryClient_ReceiveFSM::pGetService(ReportServices &msg, ServiceDef service, int subsystem)
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
						&& service.minor_version == service_rec->getMinorVersionNumber()) {
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

JausAddress DiscoveryClient_ReceiveFSM::pGetService(ReportServiceList &msg, ServiceDef service, int subsystem)
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
								&& service.minor_version == service_rec->getMinorVersionNumber()) {
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

void DiscoveryClient_ReceiveFSM::appendServiceUri(std::string service_uri, int major_version, int minor_version)
{
	ServiceDef service;
	service.service_uri = service_uri;
	service.minor_version = minor_version;
	service.major_version = major_version;
	if ( std::find(p_own_uri_services.begin(), p_own_uri_services.end(), service) != p_own_uri_services.end() ) {
		ROS_INFO_NAMED("DiscoveryClient", "	%s already known", service_uri.c_str());
	} else {
		ROS_DEBUG_NAMED("DiscoveryClient", "appendServiceUri: %s", service_uri.c_str());
		p_own_uri_services.push_back(service);
		if (p_discovery_config.register_own_services) {
			p_is_registered = false;
		}
		pCheckTimer();
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
	mutex_.lock();
	ROS_DEBUG_NAMED("DiscoveryClient", "reportedIdentificationAction from sender: subsystem: %d, node: %d, component: %d",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	iop_msgs_fkie::Identification ros_msg;
	ros_msg.request_type = msg.getBody()->getReportIdentificationRec()->getQueryType();
	ros_msg.system_type = msg.getBody()->getReportIdentificationRec()->getType();
	ros_msg.name = msg.getBody()->getReportIdentificationRec()->getIdentification();
	ros_msg.address.subsystem_id = transportData.getSourceID()->getSubsystemID();
	ros_msg.address.node_id = transportData.getSourceID()->getNodeID();
	ros_msg.address.component_id = transportData.getSourceID()->getComponentID();
	if (ros_msg.request_type == 1) {
		// update the system identification
		p_discovered_system.name = ros_msg.name;
		p_discovered_system.address = ros_msg.address;
	} else if (ros_msg.request_type == 2) {
		// request nodes of the subsystem
		pUpdateSubsystemIdent(ros_msg.address, msg);
		query_identification(3, 0xFFFF, 0xFF, 0xFF);
	} else if (ros_msg.request_type == 3) {
		pUpdateNodeIdent(ros_msg.address, msg);
	}
	if (p_enable_ros_interface) {
		p_pub_identification.publish(ros_msg);
	}
	if (pHasToDiscover()) {
		// request configuration
		QueryConfiguration req_cfg_msg;
		JausAddress addr(transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
		req_cfg_msg.getBody()->getQueryConfigurationRec()->setQueryType(ros_msg.request_type);
		this->sendJausMessage(req_cfg_msg, addr);
		if (!p_recved_service_lists and p_count_queries > 2) {
			ROS_DEBUG_NAMED("DiscoveryClient", "send request for QueryServices for compatibility to v1.0");
			QueryServices req_srv_msg;
			this->sendJausMessage(req_srv_msg, addr);
		}
		QueryServiceList req_srvl_msg;
		this->sendJausMessage(req_srvl_msg, addr);
		p_count_queries += 1;

	}
	mutex_.unlock();
}

void DiscoveryClient_ReceiveFSM::handleReportServiceListAction(ReportServiceList msg, Receive::Body::ReceiveRec transportData)
{
	p_recved_service_lists = true;
	ROS_DEBUG_NAMED("DiscoveryClient", "handleReportServiceListAction from sender: subsystem: %d, node: %d, component: %d",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	/// Insert User Code HERE
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
		if (p_discovery_config.register_own_services) {
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
		mutex_.lock();
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
		mutex_.unlock();
	}
	// check for services to discover, but only if a discovery_handler is set
	mutex_.lock();
	for (int i = p_discover_services.size()-1; i >= 0; --i) {
		if (!p_discover_services[i].discovered) {
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
					ServiceDef service = p_discover_services[i].service;
					ROS_DEBUG_NAMED("DiscoveryClient", "service '%s' discovered @%d.%d.%d through service list", service.service_uri.c_str(), addr.getSubsystemID(), addr.getNodeID(), addr.getComponentID());
					p_discover_services[i].discovered = true;
					pInformDiscoverCallbacks(service, addr);
				}
			}
//		}
	}
	pCheckTimer();
	mutex_.unlock();
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
		if (p_discovery_config.register_own_services) {
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
		mutex_.lock();
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
		mutex_.unlock();
	}
	// check for services to discover, but only if a discovery_handler is set
	mutex_.lock();
	for (int i = p_discover_services.size()-1; i >= 0; --i) {
		if (!p_discover_services[i].discovered) {
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
					ServiceDef service = p_discover_services[i].service;
					ROS_DEBUG_NAMED("DiscoveryClient", "service '%s' discovered @%d.%d.%d through services old stile", service.service_uri.c_str(), addr.getSubsystemID(), addr.getNodeID(), addr.getComponentID());
					p_discover_services[i].discovered = true;
					pInformDiscoverCallbacks(service, addr);
				}
			}
//		}
	}
	pCheckTimer();
	mutex_.unlock();
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
	int query_type = discovery_config::TYPE_SUBSYSTEM;
	unsigned short subsystem_id = jausRouter->getJausAddress()->getSubsystemID();
	if (p_discovery_config.system_id == discovery_config::TYPE_SUBSYSTEM) {
		// we are subsystem -> discover system
		query_type = discovery_config::TYPE_SYSTEM;
		subsystem_id = 65535; //0xFFFF
	} else if (p_discovery_config.system_id == discovery_config::TYPE_NODE or
		// we are node -> find subsystem discovery service and register own services
		// if this discovery service is included in a component.
			p_discovery_config.system_id == discovery_config::TYPE_COMPONENT) {
		// IOP 5a: A QueryIdentification message shall be broadcast by
		// every JAUS component on the subsystem at a rate of at least once per 5 minutes for the
		// purpose of finding and registering services with the Discovery service.

		// is set by default...
	}
	query_identification(query_type, subsystem_id, 0xFF, 0xFF);
	// send query for services to discover, if these are not in the same subsystem
	if (p_discover_services.size() > 0) {
		std::set<int> subsystems;
		mutex_.lock();
		for (unsigned int i = 0; i < p_discover_services.size(); i++) {
			if (!p_discover_services[i].discovered) {
				subsystem_id = p_discover_services[i].subsystem;
				if (subsystems.count(subsystem_id) == 0) {
					subsystems.insert(subsystem_id);
					ROS_DEBUG_NAMED("DiscoveryClient", "send QueryServices to subsystem: %d for discover service: %s",
							subsystem_id, p_discover_services[i].service.service_uri.c_str());
					query_identification(query_type, subsystem_id, 0xFF, 0xFF);
	//				QueryIdentification msg;
	//				msg.getBody()->getQueryIdentificationRec()->setQueryType(discovery_config::TYPE_SUBSYSTEM);
	//				printf("[DiscoveryClient] send QueryIdentification to subsystem: %d of type: %d (SUBSYSTEM)\n", subsystem_id, discovery_config::TYPE_SUBSYSTEM);
	//				sendJausMessage(msg,JausAddress(subsystem_id, 0xFF, 0xFF)); //0xFFFF, 0xFF, 0xFF
				}
			}
		}
		mutex_.unlock();
	}
	if (p_enable_ros_interface and query_type != discovery_config::TYPE_SUBSYSTEM) {
		query_identification(discovery_config::TYPE_SUBSYSTEM, 0xFFFF, 0xFF, 0xFF);
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
	if (query_type == discovery_config::TYPE_SUBSYSTEM) {
		if (this->jausRouter->getJausAddress()->getSubsystemID() == transportData.getSourceID()->getSubsystemID()) {
			if (!onRegistration() && !isRegistered()) {
				p_on_registration = true;
				p_addr_discovery_service = JausAddress(transportData.getSourceID()->getSubsystemID(),
						transportData.getSourceID()->getNodeID(),
						transportData.getSourceID()->getComponentID());
				ROS_INFO_NAMED("DiscoveryClient", "register services by [%d.%d.%d]",
						transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
				ROS_DEBUG_NAMED("DiscoveryClient", "Discovery service info:");
				ROS_DEBUG_NAMED("DiscoveryClient", "  QueryType: %d", msg.getBody()->getReportIdentificationRec()->getQueryType());
				ROS_DEBUG_NAMED("DiscoveryClient", "  Type     : %d", msg.getBody()->getReportIdentificationRec()->getType());
				ROS_DEBUG_NAMED("DiscoveryClient", "  ID       : %s", msg.getBody()->getReportIdentificationRec()->getIdentification().c_str());
				RegisterServices msg_reg_services;
				RegisterServices::RegisterServicesBody::ServiceList list_services;
				for (unsigned int i = 0; i < p_own_uri_services.size(); i++) {
					ServiceDef &service = p_own_uri_services[i];
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
	} else if (query_type == discovery_config::TYPE_SYSTEM) {
		// add a discovery service for ReportSubsystemLIst
		if (p_discovery_fsm != NULL) {
			JausAddress addr(transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
			p_discovery_fsm->registerSubsystem(addr);
		}
	}
}



bool DiscoveryClient_ReceiveFSM::isRegistered()
{
	/// Insert User Code HERE
	return p_is_registered || (p_own_uri_services.size() == 0);
}

bool DiscoveryClient_ReceiveFSM::onRegistration()
{
	/// Insert User Code HERE
	return p_on_registration;
}

void DiscoveryClient_ReceiveFSM::pDiscover(std::string service_uri, int major_version, int minor_version, int subsystem)
{
	p_count_discover_tries = 0;
	ServiceDef service(service_uri, major_version, minor_version);
	mutex_.lock();
	for (unsigned int i = 0; i < p_discover_services.size(); i++) {
		if (p_discover_services[i].service == service
				&& p_discover_services[i].subsystem == subsystem) {
			ROS_DEBUG_NAMED("DiscoveryClient", "%s for subsystem: %d already in discover, skip", service_uri.c_str(), subsystem);
			p_discover_services[i].discovered = false;
			mutex_.unlock();
			return;
		}
	}
	DiscoverItem di;
	di.discovered = false;
	di.service = service;
	di.subsystem = subsystem;
	p_discover_services.push_back(di);
	mutex_.unlock();
	int query_type = discovery_config::TYPE_SUBSYSTEM;
	unsigned short subsystem_id = subsystem;
	QueryIdentification msg;
	msg.getBody()->getQueryIdentificationRec()->setQueryType(query_type);
	ROS_DEBUG_NAMED("DiscoveryClient", "discover, send QueryIdentification to subsystem: %d", subsystem_id);
	sendJausMessage(msg,JausAddress(subsystem_id, 0xFF, 0xFF)); //0xFFFF, 0xFF, 0xFF
	pCheckTimer();
}

void DiscoveryClient_ReceiveFSM::cancel_discovery()
{
	mutex_.lock();
	p_discover_services.clear();
	mutex_.unlock();
}

void DiscoveryClient_ReceiveFSM::cancel_discovery(std::string service_uri, int major_version, int minor_version)
{
	mutex_.lock();
	ServiceDef service(service_uri, major_version, minor_version);
	for (unsigned int i = 0; i < p_discover_services.size(); i++) {
		if (p_discover_services[i].service == service) {
			ROS_DEBUG_NAMED("DiscoveryClient", "remove %s from discover", service_uri.c_str());
			p_discover_services.erase(p_discover_services.begin() + i);
			mutex_.unlock();
			return;
		}
	}
	mutex_.unlock();
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
	mutex_.lock();
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
	mutex_.unlock();
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
	mutex_.lock();
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
	mutex_.unlock();
}

void DiscoveryClient_ReceiveFSM::pUpdateNodeIdent(iop_msgs_fkie::JausAddress &addr, ReportIdentification &report_ident)
{
	mutex_.lock();
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
	mutex_.unlock();
}

void DiscoveryClient_ReceiveFSM::pUpdateComponent(iop_msgs_fkie::JausAddress &node_addr, iop_msgs_fkie::JausAddress &addr, ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list)
{
	// update the service list for given component
	mutex_.lock();
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
	mutex_.unlock();
}

void DiscoveryClient_ReceiveFSM::pUpdateComponent(iop_msgs_fkie::JausAddress &node_addr, iop_msgs_fkie::JausAddress &addr, ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList *service_list)
{
	// update the service list for given component
	mutex_.lock();
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
	mutex_.unlock();
}

bool DiscoveryClient_ReceiveFSM::pHasToDiscover()
{
	for (int i = p_discover_services.size()-1; i >= 0; --i) {
		if (!p_discover_services[i].discovered) {
			return true;
		}
	}
	return false;
}

void DiscoveryClient_ReceiveFSM::pInformDiscoverCallbacks(ServiceDef &service, JausAddress &address)
{
	std::map <ServiceDef, std::vector<boost::function<void (const std::string &, JausAddress &)> > > ::iterator it;
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
