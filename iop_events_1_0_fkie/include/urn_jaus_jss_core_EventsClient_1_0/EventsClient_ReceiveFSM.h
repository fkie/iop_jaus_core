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


#ifndef EVENTSCLIENT_RECEIVEFSM_H
#define EVENTSCLIENT_RECEIVEFSM_H

#include "JausUtils.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JTSStateMachine.h"
#include "urn_jaus_jss_core_EventsClient_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_EventsClient_1_0/InternalEvents/InternalEventsSet.h"

typedef JTS::Receive_1_0 Receive;
typedef JTS::Send_1_0 Send;

#include "urn_jaus_jss_core_Transport_1_0/Transport_ReceiveFSM.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <ros/ros.h>
#include "EventsClient_ReceiveFSM_sm.h"

namespace urn_jaus_jss_core_EventsClient_1_0
{

class DllExport EventsClient_ReceiveFSM : public JTS::StateMachine
{
public:
	EventsClient_ReceiveFSM(urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM);
	virtual ~EventsClient_ReceiveFSM();

	/// Handle notifications on parent state changes
	virtual void setupNotifications();

	/// Action Methods
	virtual void handleConfirmEventRequestAction(ConfirmEventRequest msg, Receive::Body::ReceiveRec transportData);
	virtual void handleEventAction(Event msg, Receive::Body::ReceiveRec transportData);
	virtual void handleRejectEventRequestAction(RejectEventRequest msg, Receive::Body::ReceiveRec transportData);
	virtual void handleReportEventsAction(ReportEvents msg, Receive::Body::ReceiveRec transportData);


	/// Guard Methods



	EventsClient_ReceiveFSMContext *context;
	/** Create an event on given remote JAUS service. By default it is a periodic event.
	 *
	 * rate:
	 * MINIMUM_RATE = 0.1
	 * MAXIMUM_RATE = 25.0
	 *
	 * event_type:
	 * 0 = Periodic
	 * 1 = Every Change */
	template<class T>
	void create_event(void(T::*handler)(Receive::Body::ReceiveRec &transport_data, Event &msg), T*obj, JausAddress address, JTS::Message &query_msg, double rate=1.0, jUnsignedByte event_type=0)
	{
		boost::function<void (Receive::Body::ReceiveRec &transport_data, Event &msg)> callback = boost::bind(handler, obj, _1, _2);
		p_callbacks_requests[request_id_+1] = callback;
		p_create_event(address, query_msg, rate, event_type);
	}

	void cancel_event(JausAddress address, JTS::Message &query_msg);
	/** You can register a handler to be informed about the creation/cancelation status of your events. To register the handler you
	 * have to define a function in your class:
	 * void my_handler(JausAddress &addr, jUnsignedShortInteger query_msg_id, bool accepted, jByte event_id, jUnsignedByte reject_code) {}
	 * and then register:
	 * pEventsClientService->set_events_reply_handler(&MyClass::my_handler, this);
	 *
	 * reject_code:
	 * 1: Periodic events not supported
	 * 2: Change based events not supported
	 * 3: Connection refused
	 * 4: Invalid event setup
	 * 5: Message not supported
	 * 6: Invalid event ID for update event request */
	template<class T>
	void set_events_reply_handler(void(T::*handler)(JausAddress &, jUnsignedShortInteger query_msg_id, bool accepted, jByte event_id, jUnsignedByte reject_code), T*obj) {
		p_class_events_reply_callback = boost::bind(handler, obj, _1, _2, _3, _4, _5);
	}

protected:

    /// References to parent FSMs
	urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM;
	boost::function<void (JausAddress &, jUnsignedShortInteger query_msg_id, bool accepted, jByte event_id, jUnsignedByte reject_code)> p_class_events_reply_callback;
	std::map <jUnsignedByte, boost::function<void (Receive::Body::ReceiveRec &transport_data, Event &msg)> > p_callbacks_requests;  // request_id, callback
	std::map <std::pair<jUnsignedByte, unsigned int>, boost::function<void (Receive::Body::ReceiveRec &transport_data, Event &msg)> > p_callbacks_confirmed;  // <event_id, unsigned int -> JausAddress::get()>, callback
	std::set <std::pair<jUnsignedShortInteger, unsigned int> > p_events;  // unsigned int -> JausAddress::get()
	std::map <jUnsignedByte, std::pair<jUnsignedShortInteger, unsigned int> > p_event_requests;  // unsigned int -> JausAddress::get()
	std::map <std::pair<jUnsignedShortInteger, unsigned int>, jByte> p_events_confirmed;  // unsigned int -> JausAddress::get()
	jUnsignedByte request_id_;
	DeVivo::Junior::JrMutex mutex;
	void p_create_event(JausAddress address, JTS::Message &query_msg, double rate=1.0, jUnsignedByte event_type=0);
	void pRemoveEventRequestHandler(jUnsignedByte request_id);
	void pRemoveEventConfirmedHandler(JausAddress &address, jUnsignedByte event_id);
	void pMoveToConfirmedHandler(jUnsignedByte request_id, JausAddress &address, jUnsignedByte event_id);
	void pInformEventCallbacks(Receive::Body::ReceiveRec &transport_data, Event &msg);
};

};

#endif // EVENTSCLIENT_RECEIVEFSM_H
