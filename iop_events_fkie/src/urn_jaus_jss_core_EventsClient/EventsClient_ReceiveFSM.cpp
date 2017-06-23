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


#include "urn_jaus_jss_core_EventsClient/EventsClient_ReceiveFSM.h"

#include <ros/console.h>


using namespace JTS;

namespace urn_jaus_jss_core_EventsClient
{



EventsClient_ReceiveFSM::EventsClient_ReceiveFSM(urn_jaus_jss_core_Transport::Transport_ReceiveFSM* pTransport_ReceiveFSM)
{

	/*
	 * If there are other variables, context must be constructed last so that all
	 * class variables are available if an EntryAction of the InitialState of the
	 * statemachine needs them.
	 */
	context = new EventsClient_ReceiveFSMContext(*this);

	this->pTransport_ReceiveFSM = pTransport_ReceiveFSM;
	request_id_ = 0;
}



EventsClient_ReceiveFSM::~EventsClient_ReceiveFSM()
{
	delete context;
}

void EventsClient_ReceiveFSM::setupNotifications()
{
	pTransport_ReceiveFSM->registerNotification("Receiving", ieHandler, "InternalStateChange_To_EventsClient_ReceiveFSM_Receiving_Ready", "Transport_ReceiveFSM");
	registerNotification("Receiving_Ready", pTransport_ReceiveFSM->getHandler(), "InternalStateChange_To_Transport_ReceiveFSM_Receiving", "EventsClient_ReceiveFSM");
	registerNotification("Receiving", pTransport_ReceiveFSM->getHandler(), "InternalStateChange_To_Transport_ReceiveFSM_Receiving", "EventsClient_ReceiveFSM");

}

void EventsClient_ReceiveFSM::handleCommandEventAction(CommandEvent msg, Receive::Body::ReceiveRec transportData)
{
	//TODO
}

void EventsClient_ReceiveFSM::handleConfirmEventRequestAction(ConfirmEventRequest msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	jUnsignedByte request_id = msg.getBody()->getConfirmEventRequestRec()->getRequestID();
	ROS_DEBUG_NAMED("EventsClient", "Confirmed for request_id: %d to %d.%d.%d, rate: %f", request_id,
			sender.getSubsystemID(), sender.getNodeID(), sender.getComponentID(), msg.getBody()->getConfirmEventRequestRec()->getConfirmedPeriodicRate());
	mutex.lock();
	std::map <jUnsignedByte, std::pair<jUnsignedShortInteger, unsigned int> >::iterator it = p_event_requests.find(request_id);
	if (it != p_event_requests.end()) {
		if (p_events.find(it->second) == p_events.end()) {
			p_events.insert(it->second);
			p_events_confirmed[it->second] = msg.getBody()->getConfirmEventRequestRec()->getEventID();
			// copy callbacks
			pMoveToConfirmedHandler(request_id, sender, msg.getBody()->getConfirmEventRequestRec()->getEventID());
		} else {
			p_events.erase(it->second);
			p_events_confirmed.erase(it->second);
			pRemoveEventConfirmedHandler(sender, msg.getBody()->getConfirmEventRequestRec()->getEventID());
		}
		p_event_requests.erase(it);
	}
	mutex.unlock();

	if (p_class_events_reply_callback) {
		jUnsignedShortInteger query_msg_id = 0;
		p_class_events_reply_callback(sender, query_msg_id, true, msg.getBody()->getConfirmEventRequestRec()->getEventID(), 0);
	}
}

void EventsClient_ReceiveFSM::handleEventAction(Event msg, Receive::Body::ReceiveRec transportData)
{
	pInformEventCallbacks(transportData, msg);

}

void EventsClient_ReceiveFSM::handleRejectEventRequestAction(RejectEventRequest msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
	uint16_t subsystem_id = transportData.getSourceID()->getSubsystemID();
	uint8_t node_id = transportData.getSourceID()->getNodeID();
	uint8_t component_id = transportData.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	jUnsignedByte request_id = msg.getBody()->getRejectEventRequestRec()->getRequestID();
	ROS_DEBUG_NAMED("EventsClient", "Rejected for request_id: %d to %d.%d.%d, code: %d", request_id,
			sender.getSubsystemID(), sender.getNodeID(), sender.getComponentID(), msg.getBody()->getRejectEventRequestRec()->getResponseCode());
	if (p_class_events_reply_callback) {
		jUnsignedShortInteger query_id = 0;
		mutex.lock();
		std::map <jUnsignedByte, std::pair<jUnsignedShortInteger, unsigned int> >::iterator it = p_event_requests.find(request_id);
		if (it != p_event_requests.end()) {
			query_id = it->second.first;
			if (p_events.find(it->second) != p_events.end()) {
				p_events.erase(it->second);
			}
			p_event_requests.erase(it);
		}
		mutex.unlock();
		p_class_events_reply_callback(sender, query_id, false, 0, msg.getBody()->getRejectEventRequestRec()->getResponseCode());
	}
}

void EventsClient_ReceiveFSM::handleReportEventTimeoutAction(ReportEventTimeout msg, Receive::Body::ReceiveRec transportData)
{
	//TODO
}

void EventsClient_ReceiveFSM::handleReportEventsAction(ReportEvents msg, Receive::Body::ReceiveRec transportData)
{
	/// Insert User Code HERE
}

void EventsClient_ReceiveFSM::p_create_event(JausAddress address, JTS::Message &query_msg, double rate, jUnsignedByte event_type)
{
	std::pair<jUnsignedShortInteger, unsigned int> event_pair = std::make_pair(query_msg.getID(), address.get());
	bool send_msg = false;
	CreateEvent create_event;
	mutex.lock();
	//  if (p_events.find(event_pair) == p_events.end()) {
	request_id_++;
	ROS_DEBUG_NAMED("EventsClient", "Send create event for query=%#x to %d.%d.%d, rate: %f, request_id: %d", query_msg.getID(),
			address.getSubsystemID(), address.getNodeID(), address.getComponentID(), rate, request_id_);
	jUnsignedInteger len = query_msg.getSize();
	unsigned char bytes[len];
	query_msg.encode(bytes);
	create_event.getBody()->getCreateEventRec()->setRequestID(request_id_);
	create_event.getBody()->getCreateEventRec()->setEventType(event_type);
	create_event.getBody()->getCreateEventRec()->setRequestedPeriodicRate(rate);
	create_event.getBody()->getCreateEventRec()->getQueryMessage()->set(len, bytes);
	p_event_requests[request_id_] = event_pair;
	send_msg = true;
	//  }
	mutex.unlock();
	if (send_msg) {
		this->sendJausMessage(create_event, address);
	}
}

void EventsClient_ReceiveFSM::cancel_event(JausAddress address, JTS::Message &query_msg)
{
	std::pair<jUnsignedShortInteger, unsigned int> event_pair = std::make_pair(query_msg.getID(), address.get());
	bool send_msg = false;
	CancelEvent cancel_event;
	mutex.lock();
	request_id_++;
	ROS_DEBUG_NAMED("EventsClient", "Send cancel event for query=%#x to %d.%d.%d, request_id: %d", query_msg.getID(),
			address.getSubsystemID(), address.getNodeID(), address.getComponentID(), request_id_);
	std::map <std::pair<jUnsignedShortInteger, unsigned int>, jByte>::iterator it = p_events_confirmed.find(event_pair);
	if (it != p_events_confirmed.end()) {
		cancel_event.getBody()->getCancelEventRec()->setEventID(it->second);
		cancel_event.getBody()->getCancelEventRec()->setRequestID(request_id_);
		p_event_requests[request_id_] = event_pair;
		send_msg = true;
	}
	mutex.unlock();
	if (send_msg) {
		this->sendJausMessage(cancel_event, address);
	}
}

void EventsClient_ReceiveFSM::pRemoveEventRequestHandler(jUnsignedByte request_id)
{
	std::map <jUnsignedByte, boost::function<void (JausAddress &, unsigned int datalen, const unsigned char* data)> >::iterator it;
	it = p_callbacks_requests.find(request_id);
	if (it != p_callbacks_requests.end()) {
		p_callbacks_requests.erase(it);
	}
}

void EventsClient_ReceiveFSM::pRemoveEventConfirmedHandler(JausAddress &address, jUnsignedByte event_id)
{
	std::pair<jUnsignedByte, unsigned int> mp = std::make_pair(event_id, address.get());
	std::map <std::pair<jUnsignedByte, unsigned int>, boost::function<void (JausAddress &, unsigned int datalen, const unsigned char* data)> >::iterator it;
	it = p_callbacks_confirmed.find(mp);
	if (it != p_callbacks_confirmed.end()) {
		p_callbacks_confirmed.erase(it);
	}
}

void EventsClient_ReceiveFSM::pMoveToConfirmedHandler(jUnsignedByte request_id, JausAddress &address, jUnsignedByte event_id)
{
	std::pair<jUnsignedByte, unsigned int> mp = std::make_pair(event_id, address.get());
	std::map <jUnsignedByte, boost::function<void (JausAddress &, unsigned int datalen, const unsigned char* data)> >::iterator it;
	it = p_callbacks_requests.find(request_id);
	if (it != p_callbacks_requests.end()) {
		p_callbacks_confirmed[mp] = it->second;
		p_callbacks_requests.erase(it);
	}
}

void EventsClient_ReceiveFSM::pInformEventCallbacks(Receive::Body::ReceiveRec &transport_data, Event &msg)
{
	uint16_t subsystem_id = transport_data.getSourceID()->getSubsystemID();
	uint8_t node_id = transport_data.getSourceID()->getNodeID();
	uint8_t component_id = transport_data.getSourceID()->getComponentID();
	JausAddress sender(subsystem_id, node_id, component_id);
	std::map <std::pair<jUnsignedByte, unsigned int>, boost::function<void (JausAddress &, unsigned int datalen, const unsigned char* data)> >::iterator it;
	it = p_callbacks_confirmed.find(std::make_pair(msg.getBody()->getEventRec()->getEventID(), sender.get()));
	if (it != p_callbacks_confirmed.end()) {
		it->second(sender, msg.getBody()->getEventRec()->getReportMessage()->getLength(), msg.getBody()->getEventRec()->getReportMessage()->getData());
	}
}


};
