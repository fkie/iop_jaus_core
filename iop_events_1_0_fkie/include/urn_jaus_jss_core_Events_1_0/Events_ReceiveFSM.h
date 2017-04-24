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


#ifndef EVENTS_RECEIVEFSM_H
#define EVENTS_RECEIVEFSM_H

#include "JausUtils.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JTSStateMachine.h"
#include "urn_jaus_jss_core_Events_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_Events_1_0/InternalEvents/InternalEventsSet.h"

typedef JTS::Receive_1_0 Receive;
typedef JTS::Send_1_0 Send;

#include "urn_jaus_jss_core_Transport_1_0/Transport_ReceiveFSM.h"


#include "Events_ReceiveFSM_sm.h"

#include <ros/ros.h>

#if __GNUC__ > 5
 #define GNU_CONST_STATIC_FLOAT_DECLARATION constexpr
#else
 #define GNU_CONST_STATIC_FLOAT_DECLARATION const
#endif

namespace urn_jaus_jss_core_Events_1_0
{

class DllExport Events_ReceiveFSM : public JTS::StateMachine
{
public:
	static GNU_CONST_STATIC_FLOAT_DECLARATION float MINIMUM_RATE;
	static GNU_CONST_STATIC_FLOAT_DECLARATION float MAXIMUM_RATE;
	Events_ReceiveFSM(urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM);
	virtual ~Events_ReceiveFSM();

	/// Handle notifications on parent state changes
	virtual void setupNotifications();

	/// Action Methods
	virtual void cancelAndConfirmEventRequestAction(CancelEvent msg, Receive::Body::ReceiveRec transportData);
	virtual void createAndConfirmEventRequestAction(CreateEvent msg, Receive::Body::ReceiveRec transportData);
	virtual void sendEventAction();
	virtual void sendRejectEventRequestAction();
	virtual void sendRejectEventRequestAction(CancelEvent msg, Receive::Body::ReceiveRec transportData);
	virtual void sendRejectEventRequestAction(CreateEvent msg, Receive::Body::ReceiveRec transportData);
	virtual void sendRejectEventRequestAction(UpdateEvent msg, Receive::Body::ReceiveRec transportData);
	virtual void sendReportEventsAction(QueryEvents msg, Receive::Body::ReceiveRec transportData);
	virtual void updateAndConfirmEventRequestAction(CreateEvent msg, Receive::Body::ReceiveRec transportData);
	virtual void updateAndConfirmEventRequestAction(UpdateEvent msg, Receive::Body::ReceiveRec transportData);


	/// Guard Methods
	virtual bool eventExists();
	virtual bool eventExists(CancelEvent msg, Receive::Body::ReceiveRec transportData);
	virtual bool eventExists(CreateEvent msg, Receive::Body::ReceiveRec transportData);
	virtual bool eventExists(UpdateEvent msg, Receive::Body::ReceiveRec transportData);
	virtual bool isSupported(CreateEvent msg);
	virtual bool isSupported(UpdateEvent msg);

	/// User Methods
	void eventOccurred(void* arg);
	/** Register/update the supported event. The caller has to store the report, because only the given reference to the
	 * report will be stored and used to send the report to a client, which create an event.
	 * If on change is supported you have to call this method each time the report was changed.
	 * **/
	void set_event_report(jUnsignedShortInteger query_msg_id, JTS::Message &report, bool on_change_supported=true);

	Events_ReceiveFSMContext *context;

protected:

	class EventData {
	public:
		jUnsignedByte eventid;
		Event event;
		jUnsignedShortInteger query_msg_id;
		jUnsignedByte event_type;
		double rate;
		jUnsignedByte seq_nr;
		JausAddress sender;
		EventData() {eventid = 255; query_msg_id = 0; event_type = 0; rate = 0.; seq_nr = 0;}
		EventData(jUnsignedShortInteger query_msg_id, Receive::Body::ReceiveRec transportData);
		bool operator==(EventData &value);
		bool operator!=(EventData &value);
	};
    /// References to parent FSMs
	urn_jaus_jss_core_Transport_1_0::Transport_ReceiveFSM* pTransport_ReceiveFSM;
	std::map<jUnsignedShortInteger, JTS::Message *> p_registered_events;
	std::set<jUnsignedByte> p_registered_events_onchange_;
	std::map<jUnsignedByte, EventData> p_current_events;
	std::deque<std::pair<double, jUnsignedByte> > p_timeout_events;
	DeVivo::Junior::JrTimer *p_timer;
	JTS::InternalEvent *p_event_occurred_event;
	JTS::InternalEvent *p_event_trigger_event;
	void startTimer(int ms);
	jUnsignedShortInteger pMessageIDFromData(const unsigned char *data);
	jUnsignedByte pGetFreeEventID();
	bool hasEvent(const unsigned char *data, Receive::Body::ReceiveRec transportData);
	void append2timeout(jUnsignedByte eventid);
};

};

#endif // EVENTS_RECEIVEFSM_H
