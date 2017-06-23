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


#include "urn_jaus_jss_core_Events/Events_ReceiveFSM.h"

#include <ros/console.h>


using namespace JTS;

namespace urn_jaus_jss_core_Events
{

GNU_CONST_STATIC_FLOAT_DECLARATION float Events_ReceiveFSM::MINIMUM_RATE = 0.1f;
GNU_CONST_STATIC_FLOAT_DECLARATION float Events_ReceiveFSM::MAXIMUM_RATE = 25.0f;

// Helper function.  This needs to be static so we can
// call it from pthread_create.
static void EventOccurred(void* arg)
{
	((Events_ReceiveFSM*)arg)->eventOccurred(arg);
}


Events_ReceiveFSM::Events_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new Events_ReceiveFSMContext(*this);
	std::cout << "CREATE EVENTS 1.1" << std::endl;
	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	p_timer = NULL;
	p_event_occurred_event = new InternalEvent("EventOccurred", "ControlEventOccurred");
	p_event_trigger_event = NULL;
}

Events_ReceiveFSM::~Events_ReceiveFSM()
{
	if (p_timer != NULL) {
		p_timer->stop();
		delete p_timer;
	}
	delete p_event_occurred_event;
	delete context;
}

void Events_ReceiveFSM::setupNotifications()
{
	pTransport_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_Events_ReceiveFSM_Receiving_Ready", "Transport_ReceiveFSM");
	registerNotification("Receiving_Ready", pTransport_ReceiveFSM->getHandler(), "InternalStateChange_To_Transport_ReceiveFSM_Receiving", "Events_ReceiveFSM");
	registerNotification("Receiving", pTransport_ReceiveFSM->getHandler(), "InternalStateChange_To_Transport_ReceiveFSM_Receiving", "Events_ReceiveFSM");
	p_registered_events.size();

}

void Events_ReceiveFSM::startTimer(int ms)
{
	ROS_DEBUG_NAMED("Events", "start timer for %d ms", ms);
	if (p_timer != NULL) {
		p_timer->stop();
		delete p_timer;
	}
	p_timer = new DeVivo::Junior::JrTimer(EventOccurred, this, ms);
	p_timer->start();
}

void Events_ReceiveFSM::eventOccurred(void* arg)
{
  ((Events_ReceiveFSM*)arg)->getHandler()->invoke(p_event_occurred_event);
  // create a new event, since the InternalEventHandler deletes the given.
  p_event_occurred_event = new InternalEvent("EventOccurred", "ControlEventOccurred");
}

void Events_ReceiveFSM::set_event_report(jUnsignedShortInteger query_msg_id, JTS::Message &report, bool on_change_supported)
{
	if (p_registered_events.find(query_msg_id) == p_registered_events.end()) {
		// if it is a new message id inform the user
		ROS_INFO_NAMED("Events", "register supported event: query=%#x, on_change_supported=%d", query_msg_id, on_change_supported);
	}
	p_registered_events[query_msg_id] = &report;
	if (on_change_supported) {
		p_registered_events_onchange_.insert(query_msg_id);
	}
	typedef std::map<jUnsignedByte, EventData>::iterator it_type;
	for(it_type it = p_current_events.begin(); it != p_current_events.end(); it++) {
//		std::cout << "CUREREP" << std::hex << it->second.query_msg_id << " added: " << query_msg_id << std::dec << ", count: " << p_current_events.size()<< std::endl;
		if (it->second.query_msg_id == query_msg_id) {
			if (it->second.event_type == 1) {
				// send the report to event receiver
				it->second.event.getBody()->getEventRec()->setSequenceNumber(it->second.event.getBody()->getEventRec()->getSequenceNumber()+1);
				ROS_DEBUG_NAMED("Events", "send changed_report, query_msg_id: %#x", query_msg_id);
				jUnsignedInteger len = report.getSize();
				if (len > 65536) {
					ROS_WARN_NAMED("Events", "large message detected, size: %d", len);
				}
				unsigned char bytes[len];
				report.encode(bytes);
				Event event;
				event.getBody()->getEventRec()->setSequenceNumber(it->second.seq_nr);
				event.getBody()->getEventRec()->setEventID(it->second.eventid);
				event.getBody()->getEventRec()->getReportMessage()->set(len, bytes);
				it->second.seq_nr++;
				sendJausMessage(event, it->second.sender);
			//				unsigned char bytes[len];
			//				report.encode(bytes);
			//				it->second.event.getBody()->getEventRec()->getReportMessage()->set(len, bytes);
			//				sendJausMessage(it->second.event, it->second.sender);
			// needs to send also a not encapsulated report?
				//sendJausMessage(report, it->second.sender);
			} else {
				// we found the query message ID, but it does not report on change
				break;
			}
		}
	}
}

void Events_ReceiveFSM::cancelAndConfirmEventRequestAction(CancelEvent msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	jUnsignedByte request_id = msg.getBody()->getCancelEventRec()->getRequestID();
	jUnsignedByte event_id = msg.getBody()->getCancelEventRec()->getEventID();
	ROS_DEBUG_NAMED("Events", "cancelEventAction, eventid: %d", event_id);
	std::map<jUnsignedByte, EventData>::iterator it = p_current_events.find(event_id);
	if (it != p_current_events.end()) {
		p_current_events.erase(it);
	}
	ConfirmEventRequest response;
	response.getBody()->getConfirmEventRequestRec()->setEventID(event_id);
	response.getBody()->getConfirmEventRequestRec()->setRequestID(request_id);
	response.getBody()->getConfirmEventRequestRec()->setConfirmedPeriodicRate(0);
	sendJausMessage( response, sender );
}

void Events_ReceiveFSM::cancelEventAction()
{
	//TODO
}

void Events_ReceiveFSM::createAndConfirmEventRequestAction(CreateEvent msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	EventData event;
	event.sender = sender;
	jUnsignedByte request_id = msg.getBody()->getCreateEventRec()->getRequestID();
	event.event_type = msg.getBody()->getCreateEventRec()->getEventType();
	event.rate = msg.getBody()->getCreateEventRec()->getRequestedPeriodicRate();
	event.eventid = pGetFreeEventID();
	event.query_msg_id = pMessageIDFromData(msg.getBody()->getCreateEventRec()->getQueryMessage()->getData());
	ROS_DEBUG_NAMED("Events", "createEventAction event id: %d, query_id: %#x, request_id: %d, event type: %s, rate: %f, sender %d.%d.%d",
			event.eventid, event.query_msg_id, request_id, (event.event_type == 0) ? "Periodic" : "Every change", event.rate, subsystem_id, node_id, component_id);
	event.event.getBody()->getEventRec()->setEventID(event.eventid);
	//	jUnsignedInteger len = 2;
	//	jUnsignedShortInteger reportid = p_registered_events[event.query_msg_id][0];
	//	event.event.getBody()->getEventRec()->getReportMessage()->set(len, (const unsigned char *)&reportid);
	event.event.getBody()->getEventRec()->setSequenceNumber(0);
	std::cout << "!!!!ADD EVEND" << (int)event.eventid << std::endl;
	p_current_events[event.eventid] = event;
	// start the timer
	append2timeout(event.eventid);
	// send confirm message
	ConfirmEventRequest response;
	response.getBody()->getConfirmEventRequestRec()->setEventID(event.eventid);
	response.getBody()->getConfirmEventRequestRec()->setRequestID(request_id);
	response.getBody()->getConfirmEventRequestRec()->setConfirmedPeriodicRate(event.rate);
	sendJausMessage( response, sender );
}

void Events_ReceiveFSM::sendRejectCommandEventRequestAction(CreateCommandEvent msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	CreateCommandEvent::Body::CreateEventRec *msg_event = msg.getBody()->getCreateEventRec();
	RejectEventRequest revent;
	RejectEventRequest::Body::RejectEventRequestRec *rvent_rec = revent.getBody()->getRejectEventRequestRec();
	jUnsignedByte requestid = msg_event->getRequestID();
	rvent_rec->setRequestID(requestid);
	jUnsignedByte rcode = 5;
	std::string rmsg("message not supported");
	rvent_rec->setResponseCode(rcode);
	rvent_rec->setErrorMessage(rmsg);
	ROS_DEBUG_NAMED("Events", "sendRejectCommandEventRequestAction to %d.%d.%d, CreateEvent: %d, rcode: %d, msg: %s",
				subsystem_id, node_id, component_id, requestid, rcode, rmsg.c_str());
	sendJausMessage( revent, sender );
}

void Events_ReceiveFSM::sendEventAction()
{
	ROS_DEBUG_NAMED("Events", "sendEventAction");
	double current_ts = ros::WallTime::now().toSec();
	std::vector<jUnsignedByte> readd;
	while (p_timeout_events.size() > 0 && current_ts - p_timeout_events.begin()->first < 0.004) {
		jUnsignedByte eventid = p_timeout_events.begin()->second;
		// send event
		std::map<jUnsignedByte, EventData>::iterator itev = p_current_events.find(eventid);
		if (itev != p_current_events.end()) {
			itev->second.event.getBody()->getEventRec()->setSequenceNumber(itev->second.event.getBody()->getEventRec()->getSequenceNumber()+1);
			ROS_DEBUG_NAMED("Events", "sendEventAction, eventid: %d", eventid);
			JTS::Message *report = p_registered_events[itev->second.query_msg_id];
			jUnsignedInteger len = report->getSize();
			if (len > 65536) {
				 ROS_WARN_NAMED("Events", "large message detected, size: %d", len);
			}
			unsigned char bytes[len];
			report->encode(bytes);
			Event event;
			event.getBody()->getEventRec()->setSequenceNumber(itev->second.seq_nr);
			event.getBody()->getEventRec()->setEventID(itev->second.eventid);
			event.getBody()->getEventRec()->getReportMessage()->set(len, bytes);
			itev->second.seq_nr++;
			sendJausMessage(event, itev->second.sender);
//			unsigned char bytes[len];
//			report->encode(bytes);
//			itev->second.event.getBody()->getEventRec()->getReportMessage()->set(len, bytes);
//			sendJausMessage(itev->second.event, itev->second.sender);
			// needs to send also a not encapsulated report?
//			sendJausMessage(*report, itev->second.sender);
			// if periodic, insert in timeout queue
			if (itev->second.event_type == 0) {
				readd.push_back(eventid);
			}
		}
		// remove from the queue
		p_timeout_events.erase(p_timeout_events.begin());
		current_ts = ros::WallTime::now().toSec();
	}
	for (unsigned int i = 0; i < readd.size(); i++) {
		append2timeout(readd[i]);
	}
}

void Events_ReceiveFSM::sendRejectEventRequestAction()
{
	ROS_WARN_NAMED("Events", "sendRejectEventRequestAction, not implemented");
}

void Events_ReceiveFSM::sendRejectEventRequestAction(CancelEvent msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	RejectEventRequest revent;
	RejectEventRequest::Body::RejectEventRequestRec *rvent_rec = revent.getBody()->getRejectEventRequestRec();
	jUnsignedByte requestid = msg.getBody()->getCancelEventRec()->getRequestID();
	rvent_rec->setRequestID(requestid);
	rvent_rec->setResponseCode(6);
//	rvent_rec->setErrorMessage("error, invalid event ID for cancel event");
	ROS_DEBUG_NAMED("Events", "sendRejectEventRequestAction to %d.%d.%d, CancelEvent: %d",
			subsystem_id, node_id, component_id, requestid);
	sendJausMessage( revent, sender );
}

void Events_ReceiveFSM::sendRejectEventRequestAction(CreateEvent msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	CreateEvent::Body::CreateEventRec *msg_event = msg.getBody()->getCreateEventRec();
	RejectEventRequest revent;
	RejectEventRequest::Body::RejectEventRequestRec *rvent_rec = revent.getBody()->getRejectEventRequestRec();
	jUnsignedByte requestid = msg_event->getRequestID();
	rvent_rec->setRequestID(requestid);
	jUnsignedByte rcode = 5;
	std::string rmsg("message not supported");
	if (p_registered_events.size() > 254) {
		rcode = 3;
		rmsg = "connection refused";
	} else if (msg_event->getRequestedPeriodicRate() < MINIMUM_RATE || msg_event->getRequestedPeriodicRate() > MAXIMUM_RATE) {
		rcode = 4;
		rmsg = "invalid event setup";
	} else if (msg_event->getEventType() == 1) {
		rcode = 2;
		rmsg = "change based events not supported";
	}

	rvent_rec->setResponseCode(rcode);
//	rvent_rec->setErrorMessage(rmsg);
	ROS_DEBUG_NAMED("Events", "sendRejectEventRequestAction to %d.%d.%d, CreateEvent: %d, rcode: %d, msg: %s",
			subsystem_id, node_id, component_id, requestid, rcode, rmsg.c_str());
	sendJausMessage( revent, sender );
}

void Events_ReceiveFSM::sendRejectEventRequestAction(UpdateEvent msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	UpdateEvent::Body::UpdateEventRec *msg_event = msg.getBody()->getUpdateEventRec();
	RejectEventRequest revent;
	RejectEventRequest::Body::RejectEventRequestRec *rvent_rec = revent.getBody()->getRejectEventRequestRec();
	jUnsignedByte requestid = msg_event->getRequestID();
	rvent_rec->setRequestID(requestid);
	jUnsignedByte rcode = 5;
	std::string rmsg("message not supported");
	if (p_current_events.find(msg_event->getEventID()) == p_current_events.end()) {
		rcode = 6;
		rmsg = "error, invalid event ID for update event";
	} else if (p_registered_events.size() > 254) {
		rcode = 3;
		rmsg = "connection refused";
	} else if (msg_event->getRequestedPeriodicRate() < MINIMUM_RATE || msg_event->getRequestedPeriodicRate() > MAXIMUM_RATE) {
		rcode = 4;
		rmsg = "invalid event setup";
	} else if (msg_event->getEventType() == 1) {
		rcode = 2;
		rmsg = "change based events not supported";
	}
	rvent_rec->setResponseCode(rcode);
//	rvent_rec->setErrorMessage(rmsg);
	ROS_DEBUG_NAMED("Events", "sendRejectEventRequestAction to %d.%d.%d, UpdateEvent: %d, rcode: %d, msg: %s",
			subsystem_id, node_id, component_id, requestid, rcode, rmsg.c_str());
	sendJausMessage( revent, sender );
}

void Events_ReceiveFSM::sendReportEventTimeoutAction(QueryEventTimeout msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	ReportEventTimeout response;
	// currently no timeout for events supported
	response.getBody()->getReportTimoutRec()->setTimeout(0);
	sendJausMessage( response, sender );
}

void Events_ReceiveFSM::sendReportEventsAction(QueryEvents msg, Receive::Body::ReceiveRec transportData)
{
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	ReportEvents response;
	try {
		QueryEvents::Body::QueryEventsVar *evrec = msg.getBody()->getQueryEventsVar();
		jUnsignedByte filter = evrec->getFieldValue();
		for (std::map<jUnsignedByte, EventData>::iterator it=p_current_events.begin(); it!=p_current_events.end(); it++) {
			bool doadd = false;
			// add filter support specified in QueryEvents message
			switch(filter) {
				case 0:
					doadd = (evrec->getMessageIDRec()->getMessageCode() == it->second.query_msg_id);
					break;
				case 1:
					doadd = (evrec->getEventTypeRec()->getEventType() == it->second.event_type);
					break;
				case 2:
					doadd = (evrec->getEventIDRec()->getEventID() == it->first);
					break;
				case 3:
					doadd = true;
					break;
			}
			if (doadd) {
				ReportEvents::Body::EventList::ReportEventRec event;
				event.setEventID(it->first);
				event.setEventType(it->second.event_type);
				const unsigned char *data = (const unsigned char *)&(it->second.query_msg_id);
				event.getQueryMessage()->set(sizeof(jUnsignedShortInteger), data);
				response.getBody()->getEventList()->addElement(event);
			}
		}
	} catch (std::exception &e) {
		ROS_WARN_NAMED("Events", "ERR %s", e.what());
	}
	ROS_DEBUG_NAMED("Events", "sendReportEventsAction to %d.%d.%d, event count: %d",
									subsystem_id, node_id, component_id, response.getBody()->getEventList()->getNumberOfElements());
	sendJausMessage( response, sender );
}

void Events_ReceiveFSM::stopEventTimerAction()
{
	//TODO
	ROS_WARN_NAMED("Events", "stopEventTimerAction, not implemented");
}

void Events_ReceiveFSM::timeoutConfirmEventRequestAction()
{
	//TODO
	ROS_WARN_NAMED("Events", "timeoutConfirmEventRequestAction, not implemented");
}

void Events_ReceiveFSM::updateAndConfirmEventRequestAction(CreateEvent msg, Receive::Body::ReceiveRec transportData)
{
	CreateEvent::Body::CreateEventRec *event = msg.getBody()->getCreateEventRec();
	jUnsignedShortInteger query_msg_id = pMessageIDFromData(event->getQueryMessage()->getData());
	EventData event_data(query_msg_id, transportData);
	for (std::map<jUnsignedByte, EventData>::iterator it=p_current_events.begin(); it!=p_current_events.end(); it++) {
		if (it->second == event_data) {
			it->second.event_type = event->getEventType();
			it->second.rate = event->getRequestedPeriodicRate();
			it->second.query_msg_id = query_msg_id;
//			jUnsignedInteger len = 2;
//			jUnsignedShortInteger reportid = p_registered_events[it->second.query_msg_id][0];
//			it->second.event.getBody()->getEventRec()->getReportMessage()->set(len, (const unsigned char *)&reportid);

			uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
			uint8_t node_id = transportData.getSourceID()->getNodeID();
			uint8_t component_id = transportData.getSourceID()->getComponentID();
			JausAddress sender(subsystem_id, node_id, component_id);
			ROS_DEBUG_NAMED("Events", "updateAndConfirmEventRequestAction while create event id: %d, query_id: %#x, request_id: %d, event type: %s, rate: %f, sender %d.%d.%d",
					it->second.eventid, it->second.query_msg_id, event->getRequestID(), (it->second.event_type == 0) ? "Periodic" : "Every change", it->second.rate, subsystem_id, node_id, component_id);
			ConfirmEventRequest response;
			response.getBody()->getConfirmEventRequestRec()->setEventID(it->second.eventid);
			response.getBody()->getConfirmEventRequestRec()->setRequestID(event->getRequestID());
			response.getBody()->getConfirmEventRequestRec()->setConfirmedPeriodicRate(it->second.rate);
			sendJausMessage( response, sender );
		}
	}
}

void Events_ReceiveFSM::updateAndConfirmEventRequestAction(UpdateEvent msg, Receive::Body::ReceiveRec transportData)
{
	UpdateEvent::Body::UpdateEventRec *event = msg.getBody()->getUpdateEventRec();
	std::map<jUnsignedByte, EventData>::iterator iter = p_current_events.find(event->getEventID());
	if (iter != p_current_events.end()) {
		iter->second.event_type = event->getEventType();
		iter->second.rate = event->getRequestedPeriodicRate();
		iter->second.query_msg_id = pMessageIDFromData(event->getQueryMessage()->getData());
//		jUnsignedInteger len = 2;
//		jUnsignedShortInteger reportid = p_registered_events[iter->second.query_msg_id][0];
//		JTS::Message *report = p_registered_events[iter->second.query_msg_id][1](reportid)
//		iter->second.event.getBody()->getEventRec()->getReportMessage()->set(len, (const unsigned char *)&reportid);

		uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
		uint8_t node_id = transportData.getSourceID()->getNodeID();
		uint8_t component_id = transportData.getSourceID()->getComponentID();
		JausAddress sender(subsystem_id, node_id, component_id);
		ROS_DEBUG_NAMED("Events", "updateAndConfirmEventRequestAction event id: %d, query_id: %#x, request_id: %d,"
				"event type: %s, rate: %f, sender %d.%d.%d",
				iter->second.eventid, iter->second.query_msg_id, event->getRequestID(),
				(iter->second.event_type == 0) ? "Periodic" : "Every change",
						iter->second.rate, subsystem_id, node_id, component_id);
		ConfirmEventRequest response;
		response.getBody()->getConfirmEventRequestRec()->setEventID(iter->second.eventid);
		response.getBody()->getConfirmEventRequestRec()->setRequestID(event->getRequestID());
		response.getBody()->getConfirmEventRequestRec()->setConfirmedPeriodicRate(iter->second.rate);
		sendJausMessage( response, sender );
	}
}

bool Events_ReceiveFSM::eventExists()
{
	/// Insert User Code HERE
	bool has_events = (p_timeout_events.size() > 0);
	return has_events;
}

bool Events_ReceiveFSM::eventExists(CancelEvent msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	return (p_current_events.find(msg.getBody()->getCancelEventRec()->getEventID()) != p_current_events.end());
}

bool Events_ReceiveFSM::eventExists(CreateEvent msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	return hasEvent(msg.getBody()->getCreateEventRec()->getQueryMessage()->getData(), transportData);
}

bool Events_ReceiveFSM::eventExists(UpdateEvent msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	return (p_current_events.find(msg.getBody()->getUpdateEventRec()->getEventID()) != p_current_events.end());
}

bool Events_ReceiveFSM::isSupported(CreateEvent msg)
{
	/// Insert User Code HERE
	CreateEvent::Body::CreateEventRec *event = msg.getBody()->getCreateEventRec();
	jUnsignedShortInteger messageid = pMessageIDFromData(event->getQueryMessage()->getData());
	bool supported = (p_registered_events.find(messageid) != p_registered_events.end());
//	printf("[Events] query message %#x is %ssupported\n", messageid, (supported) ? "" : "not " );
	if (event->getEventType() == 1) {
		supported = (p_registered_events_onchange_.find(messageid) != p_registered_events_onchange_.end());
		if (!supported) {
			ROS_WARN_NAMED("Events", "change based events not supported, requestid: %d",
					msg.getBody()->getCreateEventRec()->getRequestID() );
		}
	} else if (event->getRequestedPeriodicRate() < MINIMUM_RATE || event->getRequestedPeriodicRate() > MAXIMUM_RATE) {
		supported = false;
		ROS_WARN_NAMED("Events", "periodic rate %f is not supported, requestid: %d",
				event->getRequestedPeriodicRate(), event->getRequestID() );
	}
	return supported;
}

bool Events_ReceiveFSM::isSupported(UpdateEvent msg)
{
	/// Insert User Code HERE
	UpdateEvent::Body::UpdateEventRec *event = msg.getBody()->getUpdateEventRec();
	jUnsignedShortInteger messageid = pMessageIDFromData(event->getQueryMessage()->getData());
	bool supported = (p_registered_events.find(messageid) != p_registered_events.end());
//	printf("[Events] query message %#x for update is %ssupported\n", messageid, (supported) ? "" : "not " );
	if (event->getEventType() == 1) {
		supported = (p_registered_events_onchange_.find(messageid) != p_registered_events_onchange_.end());
		if (!supported) {
			ROS_WARN_NAMED("Events", "change based events not supported, update requestid: %d",
					msg.getBody()->getUpdateEventRec()->getRequestID() );
		}
	} else if (event->getRequestedPeriodicRate() < MINIMUM_RATE || event->getRequestedPeriodicRate() > MAXIMUM_RATE) {
		supported = false;
		ROS_WARN_NAMED("Events", "periodic rate %f is not supported, update requestid: %d",
				event->getRequestedPeriodicRate(), event->getRequestID() );
	}
	return supported;
}

jUnsignedShortInteger Events_ReceiveFSM::pMessageIDFromData(const unsigned char *data)
{
	jUnsignedShortInteger result = 0;
	try {
		memcpy(&result, data, sizeof(jUnsignedShortInteger));
		result = JSIDL_v_1_0::correctEndianness(result);
	} catch (...) {
	}
	return result;
}

jUnsignedByte Events_ReceiveFSM::pGetFreeEventID()
{
	jUnsignedByte result = 0;
	while (result < 255) {
		if (p_current_events.find(result) != p_current_events.end()) {
			std::cout << "!!!!nn ++" << (int)result << std::endl;
			result++;
		} else {
			std::cout << "!!!!break" << (int)result << std::endl;
			break;
		}
	}
	return result;
}

bool Events_ReceiveFSM::hasEvent(const unsigned char *data, Receive::Body::ReceiveRec transportData)
{
	EventData event(pMessageIDFromData(data), transportData);
	for (std::map<jUnsignedByte, EventData>::iterator it=p_current_events.begin(); it!=p_current_events.end(); it++) {
		if (it->second == event) {
			return true;
		}
	}
	return false;
}

void Events_ReceiveFSM::append2timeout(jUnsignedByte eventid)
{
	double current_ts = ros::WallTime::now().toSec();
	std::map<jUnsignedByte, EventData>::iterator it = p_current_events.find(eventid);
	if (it != p_current_events.end() && it->second.event_type == 0) {
		double rate_int = 1.0 / it->second.rate;
		double timeout_ts = current_ts + rate_int;
		int timeout_pos = 0;
		for (unsigned int i = 0; i < p_timeout_events.size(); i++) {
			if (p_timeout_events[i].first < timeout_ts) {
				timeout_pos++;
			} else {
				break;
			}
		}
			// insert the next timeout for this event
		p_timeout_events.insert(p_timeout_events.begin()+timeout_pos, std::make_pair(timeout_ts, eventid));
		if (timeout_pos == 0) {
			// the next timeout is the new event, restart timer
			startTimer((int)(rate_int*1000));
		}
	}
}

Events_ReceiveFSM::EventData::EventData(jUnsignedShortInteger query_msg_id, Receive::Body::ReceiveRec transportData)
{
	eventid = 255;
	event_type = 0;
	rate = 0;
	this->query_msg_id = query_msg_id;
	sender.setSubsystemID(transportData.getSourceID()->getSubsystemID());
	sender.setNodeID(transportData.getSourceID()->getNodeID());
	sender.setComponentID(transportData.getSourceID()->getComponentID());
	seq_nr = 0;
}

bool Events_ReceiveFSM::EventData::operator==(EventData &value)
{
	if (query_msg_id != value.query_msg_id) {
//		printf("operator== %d, %d FALSE\n", query_msg_id, value.query_msg_id);
		return false;
	}
	if (sender != value.sender) {
//		printf("operator== %d.%d.%d != %d.%d.%d\n", sender.getSubsystemID(), sender.getNodeID(), sender.getComponentID(),
//					 value.sender.getSubsystemID(), value.sender.getNodeID(), value.sender.getComponentID());
		return false;
	}
	return true;
}

bool Events_ReceiveFSM::EventData::operator!=(EventData &value)
{
	return !((*this) == value);
}

};
