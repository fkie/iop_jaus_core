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


#include "urn_jaus_jss_core_Discovery/Discovery_ReceiveFSM.h"
#include <ros/ros.h>
#include <ros/console.h>
#include <string.h>
#include <algorithm>


using namespace JTS;

namespace urn_jaus_jss_core_Discovery
{

Discovery_ReceiveFSM::Discovery_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM, urn_jaus_jss_core_Events::Events_ReceiveFSM* pEvents_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new Discovery_ReceiveFSMContext(*this);
	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	this->pEvents_ReceiveFSM = pEvents_ReceiveFSM;
}

Discovery_ReceiveFSM::~Discovery_ReceiveFSM()
{
	delete context;
}

void Discovery_ReceiveFSM::registerService(int minver, int maxver, std::string serviceuri, JausAddress address)
{
	RegisterServices::RegisterServicesBody::ServiceList::ServiceRec service;
	service.setMinorVersionNumber(minver);
	service.setMajorVersionNumber(maxver);
	service.setURI(serviceuri);
	bool isnew = true;
	for (unsigned int j = 0; j < uri_services.size(); j++) {
		if (uri_services[j].first.getURI() == service.getURI()
				&& uri_services[j].second.getSubsystemID() == address.getSubsystemID()
				&& uri_services[j].second.getNodeID() == address.getNodeID()
				&& uri_services[j].second.getComponentID() == address.getComponentID()) {
			isnew = false;
			ROS_WARN_NAMED("Discovery", "service '%s' [%d.%d.%d] already exists, ignore",
					service.getURI().c_str(), address.getSubsystemID(), address.getNodeID(), address.getComponentID());
		}
	}
	if (isnew) {
		ROS_INFO_NAMED("Discovery", "register '%s' [%d.%d.%d]",
				service.getURI().c_str(), address.getSubsystemID(), address.getNodeID(), address.getComponentID());
		uri_services.push_back(std::make_pair(service, address));
	}
}

void Discovery_ReceiveFSM::registerSubsystem(JausAddress address)
{
	bool discovered = false;
	for (int i=1; i<p_subsystems.size(); i++) {
		if (address.getSubsystemID() == p_subsystems[i].getSubsystemID()) {
			discovered = true;
			break;
		}
	}
	if (!discovered) {
		p_subsystems.push_back(address);
	}
}

void Discovery_ReceiveFSM::setupNotifications()
{
	pEvents_ReceiveFSM->registerNotification("Receiving_Ready", ieHandler, "InternalStateChange_To_Discovery_ReceiveFSM_Receiving_Ready", "Events_ReceiveFSM");
	pEvents_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_Discovery_ReceiveFSM_Receiving_Ready", "Events_ReceiveFSM");
	registerNotification("Receiving_Ready", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving_Ready", "Discovery_ReceiveFSM");
	registerNotification("Receiving", pEvents_ReceiveFSM->getHandler(), "InternalStateChange_To_Events_ReceiveFSM_Receiving", "Discovery_ReceiveFSM");
	p_discovery_config.update_ros_parameter();
}

void Discovery_ReceiveFSM::publishServicesAction(RegisterServices msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	JausAddress addr_discovery_service(transportData.getSourceID()->getSubsystemID(),
			transportData.getSourceID()->getNodeID(),
			transportData.getSourceID()->getComponentID());
	RegisterServices::RegisterServicesBody::ServiceList *services = msg.getRegisterServicesBody()->getServiceList();
	ROS_DEBUG_NAMED("Discovery", "Register %u new services...", services->getNumberOfElements());
	for (unsigned int i = 0; i < services->getNumberOfElements(); i++) {
		RegisterServices::RegisterServicesBody::ServiceList::ServiceRec *service = msg.getRegisterServicesBody()->getServiceList()->getElement(i);
		registerService(service->getMinorVersionNumber(), service->getMajorVersionNumber(), service->getURI(), addr_discovery_service);
	}
}


void Discovery_ReceiveFSM::sendReportConfigurationAction(QueryConfiguration msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	int query_type = msg.getBody()->getQueryConfigurationRec()->getQueryType();
	ROS_DEBUG_NAMED("Discovery", "sendReportConfiguration query_type: %d, sender: subsystem: %d, node: %d,	component: %d",
			query_type, transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	// Extract the sender information
	JausAddress sender(transportData.getSourceID()->getSubsystemID(),
			transportData.getSourceID()->getNodeID(),
			transportData.getSourceID()->getComponentID());

	ReportConfiguration report_msg;
	int cnt_nodes = 0;
	int cnt_cmps = 0;
	for (int si=0; si < uri_services.size(); si++) {
		bool service_added = false;
		ReportConfiguration::Body::NodeList *node_list = report_msg.getBody()->getNodeList();
		for (unsigned int jn = 0; jn < node_list->getNumberOfElements(); jn++) {
			ReportConfiguration::Body::NodeList::NodeSeq *node_seq = node_list->getElement(jn);
			if (uri_services[si].second.getNodeID() == node_seq->getNodeRec()->getNodeID()) {
				ReportConfiguration::Body::NodeList::NodeSeq::ComponentList *cmp_list = node_seq->getComponentList();
				for (unsigned int jc = 0; jc < cmp_list->getNumberOfElements(); jc++) {
					ReportConfiguration::Body::NodeList::NodeSeq::ComponentList::ComponentRec *cmp_rec = cmp_list->getElement(jc);
					if (uri_services[si].second.getComponentID() == cmp_rec->getComponentID()) {
						service_added = true;
					}
				}
				if (!service_added) {
					ReportConfiguration::Body::NodeList::NodeSeq::ComponentList::ComponentRec cmp;
					cmp.setComponentID(uri_services[si].second.getComponentID());
					node_seq->getComponentList()->addElement(cmp);
					service_added = true;
					cnt_cmps++;
				}
			}
		}
		if (!service_added) {
			ReportConfiguration::Body::NodeList::NodeSeq::ComponentList::ComponentRec cmp;
			cmp.setComponentID(uri_services[si].second.getComponentID());
			ReportConfiguration::Body::NodeList::NodeSeq node;
			node.getNodeRec()->setNodeID(uri_services[si].second.getNodeID());
			node.getComponentList()->addElement(cmp);
			node_list->addElement(node);
			service_added = true;
			cnt_nodes++;
			cnt_cmps++;
		}
	}
	ROS_DEBUG_NAMED("Discovery", "	report configuration with %d nodes and %d components", cnt_nodes, cnt_cmps);
	sendJausMessage( report_msg, sender);
}

void Discovery_ReceiveFSM::sendReportIdentificationAction(QueryIdentification msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	int query_type = msg.getBody()->getQueryIdentificationRec()->getQueryType();
//	printf("[Discovery] sendReportIdentification query_type: %d, sender: subsystem: %d, node: %d,	component: %d\n",
//				 query_type,
//				 transportData.getSrcSubsystemID(),
//				 transportData.getSrcNodeID(), transportData.getSrcComponentID());
	if (p_discovery_config.system_id <= query_type) {
		JausAddress sender(transportData.getSourceID()->getSubsystemID(),
				transportData.getSourceID()->getNodeID(),
				transportData.getSourceID()->getComponentID());
		ReportIdentification report_msg;
		std::string name = "InvalidName";
		if (query_type == discovery_config::TYPE_COMPONENT) {
			name = ros::this_node::getName();
		} else if (query_type == discovery_config::TYPE_NODE)	{
			name = p_discovery_config.name_node;
		} else if (query_type == discovery_config::TYPE_SUBSYSTEM) {
			name = p_discovery_config.name_subsystem;
		} else if (query_type == discovery_config::TYPE_SYSTEM) {
			name = "System";
		}
		ROS_DEBUG_NAMED("Discovery", "sendReportIdentification own identification: query_type: %d, system_type: %d, name: %s",
				query_type, p_discovery_config.system_type, name.c_str());
		report_msg.getBody()->getReportIdentificationRec()->setQueryType(query_type);
		report_msg.getBody()->getReportIdentificationRec()->setType(p_discovery_config.system_type);
		report_msg.getBody()->getReportIdentificationRec()->setIdentification(name);
		sendJausMessage(report_msg, sender);
	} else {
//		printf("[Discovery] sendReportIdentification own system_id_type: %d>%d (query_type), do not response\n", p_discovery_config.system_id, query_type);
	}
}

void Discovery_ReceiveFSM::sendReportServiceListAction(QueryServiceList msg, Receive::Body::ReceiveRec transportData)
{
	ROS_DEBUG_NAMED("Discovery", "sendReportServiceList to: subsystem: %d, node: %d,	component: %d",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	JausAddress sender(transportData.getSourceID()->getSubsystemID(),
			transportData.getSourceID()->getNodeID(),
			transportData.getSourceID()->getComponentID());
	if (p_discovery_config.system_id == discovery_config::TYPE_SUBSYSTEM) {
		ReportServiceList report_msg;
		int cnt_services = 0;
		for (unsigned int si=0; si < uri_services.size(); si++) {
			JausAddress addr = uri_services[si].second;
			if (isComponentRequested(msg, addr.getSubsystemID(), addr.getNodeID(), addr.getComponentID())) {
				bool service_added = false;
				ReportServiceList::Body::SubsystemList *sslist = report_msg.getBody()->getSubsystemList();
				ReportServiceList::Body::SubsystemList::SubsystemSeq *ssrec = p_add_subsystem(sslist, addr.getSubsystemID());
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq *nrec = p_add_node(ssrec->getNodeList(), addr.getNodeID());
				ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq *crec = p_add_component(nrec->getComponentList(), addr.getComponentID());
				crec->getServiceList()->addElement((ReportServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec &) uri_services[si].first);
				cnt_services++;
			}
		}
		ROS_DEBUG_NAMED("Discovery", "	report services with %d services", cnt_services);
		sendJausMessage( report_msg, sender );
	}

}

bool Discovery_ReceiveFSM::isComponentRequested(QueryServiceList &msg, unsigned int subsystemid, unsigned int nodeid, unsigned int compid)
{
	QueryServiceList::Body::SubsystemList *sslist = msg.getBody()->getSubsystemList();
	unsigned int count_ssystems = sslist->getNumberOfElements();
	if (count_ssystems < 1) {
		return true;
	}
	for (unsigned int s = 0; s < count_ssystems; s++) {
		QueryServiceList::Body::SubsystemList::SubsystemSeq *ssreq = sslist->getElement(s);
		unsigned int ssid = ssreq->getSubsystemRec()->getSubsystemID();
		// check subsystem ID: 65535 -> all subsystem IDs
		if (ssid == 65535 || ssid == subsystemid) {
			QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList *nodelist = ssreq->getNodeList();
			// check node ID: 255 -> all node IDs
			unsigned int count_nodes = nodelist->getNumberOfElements();
			if (count_nodes < 1) {
				return true;
			}
			for (unsigned int n = 0; n < count_nodes; n++) {
				QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq *nodesec = nodelist->getElement(n);
				unsigned int nid = nodesec->getNodeRec()->getNodeID();
				if (nid == 255 or nid == nodeid) {
					// check the component ID; 255 -> all components of the node
					QueryServiceList::Body::SubsystemList::SubsystemSeq::NodeList::NodeSeq::ComponentList *complist = nodesec->getComponentList();
					unsigned int count_comps = complist->getNumberOfElements();
					if (count_comps < 1) {
						return true;
					}
					for (unsigned int c = 0; c < count_comps; c++) {
						unsigned int cid = complist->getElement(c)->getComponentID();
						if (cid == 255 or cid == compid) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

RS_SSList::SubsystemSeq *Discovery_ReceiveFSM::p_add_subsystem(RS_SSList *list, unsigned int id)
{
	RS_SSList::SubsystemSeq *result;
	for (unsigned int i = 0; i < list->getNumberOfElements(); i++) {
		result = list->getElement(i);
		if (result->getSubsystemRec()->getSubsystemID() == id) {
			return result;
		}
	}
	RS_SSList::SubsystemSeq ssseq;
	ssseq.getSubsystemRec()->setSubsystemID(id);
	list->addElement(ssseq);
	return list->getElement(list->getNumberOfElements()-1);
}

RS_NList::NodeSeq *Discovery_ReceiveFSM::p_add_node(RS_NList *list, unsigned int id)
{
	RS_NList::NodeSeq *result;
	for (unsigned int i = 0; i < list->getNumberOfElements(); i++) {
		result = list->getElement(i);
		if (result->getNodeRec()->getNodeID() == id) {
			return result;
		}
	}
	RS_NList::NodeSeq nseq;
	nseq.getNodeRec()->setNodeID(id);
	list->addElement(nseq);
	return list->getElement(list->getNumberOfElements()-1);
}

RS_CList::ComponentSeq *Discovery_ReceiveFSM::p_add_component(RS_CList *list, unsigned int id)
{
	RS_CList::ComponentSeq *result;
	for (unsigned int i = 0; i < list->getNumberOfElements(); i++) {
		result = list->getElement(i);
		if (result->getComponentRec()->getComponentID() == id) {
			return result;
		}
	}
	RS_CList::ComponentSeq cseq;
	cseq.getComponentRec()->setComponentID(id);
	list->addElement(cseq);
	return list->getElement(list->getNumberOfElements()-1);
}

void Discovery_ReceiveFSM::sendReportServicesAction(QueryServices msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	ROS_DEBUG_NAMED("Discovery", "sendReportServices sender: subsystem: %d, node: %d,	component: %d",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	JausAddress sender(transportData.getSourceID()->getSubsystemID(),
			transportData.getSourceID()->getNodeID(),
			transportData.getSourceID()->getComponentID());
//	msg.getBody()->getNodeList()->getElement(0)->getNodeRec()->getNodeID()
	if (p_discovery_config.system_id == discovery_config::TYPE_SUBSYSTEM) {
		ReportServices report_msg;
		int cnt_nodes = 0;
		int cnt_cmps = 0;
		for (unsigned int si=0; si < uri_services.size(); si++) {
			JausAddress addr = uri_services[si].second;
			if (isComponentRequested(msg, addr.getNodeID(), addr.getComponentID())) {
				bool service_added = false;
				ReportServices::Body::NodeList *node_list = report_msg.getBody()->getNodeList();
				for (unsigned int jn = 0; jn < node_list->getNumberOfElements(); jn++) {
					ReportServices::Body::NodeList::NodeSeq *node_seq = node_list->getElement(jn);
					if (uri_services[si].second.getNodeID() == node_seq->getNodeRec()->getNodeID()) {
						ReportServices::Body::NodeList::NodeSeq::ComponentList *cmp_list = node_seq->getComponentList();
						for (unsigned int jc = 0; jc < cmp_list->getNumberOfElements(); jc++) {
							ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq *cmp_seq = cmp_list->getElement(jc);
							if (uri_services[si].second.getComponentID() == cmp_seq->getComponentRec()->getComponentID()) {
								cmp_seq->getServiceList()->addElement((ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec &) uri_services[si].first);
								service_added = true;
							}
						}
						if (!service_added) {
							ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq cmp;
							cmp.getComponentRec()->setComponentID(uri_services[si].second.getComponentID());
							cmp.getServiceList()->addElement((ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec &) uri_services[si].first);
							node_seq->getComponentList()->addElement(cmp);
							service_added = true;
							cnt_cmps++;
						}
					}
				}
				if (!service_added) {
					ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq cmp;
					cmp.getComponentRec()->setComponentID(uri_services[si].second.getComponentID());
					cmp.getServiceList()->addElement((ReportServices::Body::NodeList::NodeSeq::ComponentList::ComponentSeq::ServiceList::ServiceRec &) uri_services[si].first);
					ReportServices::Body::NodeList::NodeSeq node;
					node.getNodeRec()->setNodeID(uri_services[si].second.getNodeID());
					node.getComponentList()->addElement(cmp);
					node_list->addElement(node);
					service_added = true;
					cnt_nodes++;
					cnt_cmps++;
				}
			}
		}
		ROS_DEBUG_NAMED("Discovery", "	report services with %d nodes, %d components and %lu services", cnt_nodes, cnt_cmps, uri_services.size());
		sendJausMessage( report_msg, sender );

	}
}

bool Discovery_ReceiveFSM::isComponentRequested(QueryServices &msg, unsigned int nodeid, unsigned int compid)
{
	QueryServices::Body::NodeList *nodelist = msg.getBody()->getNodeList();
	// check node ID: 255 -> all node IDs
	unsigned int count_nodes = nodelist->getNumberOfElements();
	if (count_nodes < 1) {
		return true;
	}
	for (unsigned int n = 0; n < count_nodes; n++) {
		QueryServices::Body::NodeList::NodeSeq *nodesec = nodelist->getElement(n);
		unsigned int nid = nodesec->getNodeRec()->getNodeID();
		if (nid == 255 or nid == nodeid) {
			// check the component ID; 255 -> all components of the node
			QueryServices::Body::NodeList::NodeSeq::ComponentList *complist = nodesec->getComponentList();
			unsigned int count_comps = complist->getNumberOfElements();
			if (count_comps < 1) {
				return true;
			}
			for (unsigned int c = 0; c < count_comps; c++) {
				unsigned int cid = complist->getElement(c)->getComponentID();
				if (cid == 255 or cid == compid) {
					return true;
				}
			}
		}
	}
	return false;
}

void Discovery_ReceiveFSM::sendReportSubsystemListAction(QuerySubsystemList msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	ROS_DEBUG_NAMED("Discovery", "sendReportSubsystemList sender: subsystem: %d, node: %d,	component: %d",
			transportData.getSourceID()->getSubsystemID(), transportData.getSourceID()->getNodeID(), transportData.getSourceID()->getComponentID());
	JausAddress sender(transportData.getSourceID()->getSubsystemID(),
			transportData.getSourceID()->getNodeID(),
			transportData.getSourceID()->getComponentID());
	ReportSubsystemList report_msg;
	ROS_DEBUG_NAMED("Discovery", "	known subsystems: %lu", p_subsystems.size());
	if (p_discovery_config.system_id == discovery_config::TYPE_SYSTEM) {
		// add only the discovery services of a subsystems
		for (int i=0; i < p_subsystems.size(); i++) {
			ReportSubsystemList::Body::SubsystemList::SubsystemRec comp;
			comp.setSubsystemID(p_subsystems[i].getSubsystemID());
			comp.setNodeID(p_subsystems[i].getNodeID());
			comp.setComponentID(p_subsystems[i].getComponentID());
			report_msg.getBody()->getSubsystemList()->addElement(comp);
		}
	}
	sendJausMessage( report_msg, sender );
}

};
