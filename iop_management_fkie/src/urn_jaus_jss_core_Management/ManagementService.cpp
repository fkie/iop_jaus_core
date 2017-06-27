

#include "urn_jaus_jss_core_Management/ManagementService.h"
#include <iostream>

using namespace JTS;

namespace urn_jaus_jss_core_Management
{

ManagementService::ManagementService(JTS::JausRouter* jausRouter , urn_jaus_jss_core_Transport::TransportService* pTransportService, urn_jaus_jss_core_Events::EventsService* pEventsService, urn_jaus_jss_core_AccessControl::AccessControlService* pAccessControlService) : Service()
{
	jausRouter->setTransportType(JausRouter::Version_1_1);

	pManagement_ReceiveFSM = new Management_ReceiveFSM(pTransportService->pTransport_ReceiveFSM, pEventsService->pEvents_ReceiveFSM, pAccessControlService->pAccessControl_ReceiveFSM);
	pManagement_ReceiveFSM->setHandlers(ieHandler, jausRouter);
	pManagement_ReceiveFSM->setupNotifications();
	pManagement_SendFSM = new Management_SendFSM(pTransportService->pTransport_SendFSM, pEventsService->pEvents_SendFSM, pAccessControlService->pAccessControl_SendFSM);
	pManagement_SendFSM->setHandlers(ieHandler, jausRouter);
	pManagement_SendFSM->setupNotifications();

}


ManagementService::~ManagementService()
{
	delete pManagement_ReceiveFSM;
	delete pManagement_SendFSM;

}

/**
 *	This is the function that will actually be run by the thread at start-up.
 *  We override it from EventReceiver in order to handle any entry
 *  actions defined by the initial state.
 */
void ManagementService::run()
{

	/// Perform any entry actions specified by the start state.


	/// Kick-off the receive loop...
	EventReceiver::run();
}

/**
 *	This is the function that will process an event either generated
 *  by the service, sent to it by another service on the same component,
 *  or as a message sent by a different component.
 */
bool ManagementService::processTransitions(InternalEvent* ie)
{
        bool done = false;

   // Since this function can be called from multiple threads,
   // we use a mutex to ensure only one state transition is
   // active at a time.
   mutex.lock();
   std::cout << "MANAGEMENT STATE: " << pManagement_ReceiveFSM->context->getState().getName() << std::endl;
   Receive* casted_ie = (Receive*) ie;
   std::cout << "MANAGEMENT PROCES: " << "  trans: " << ie->getName() << " source: " << ie->getSource() << std::endl;
	// Invoke the FSM transition for this event.
	try
	{
		if ((done == false) && (ie->getName().compare("Receive") == 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());

			if ((id != QueryStatus::ID) &&
				(id != SetEmergency::ID) &&
				(id != ClearEmergency::ID) &&
				(id != Shutdown::ID) &&
				(id != Reset::ID) &&
				(id != Resume::ID) &&
				(id != Standby::ID))
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == ClearEmergency::ID)
			{
				ClearEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Shutdown::ID)
			{
				Shutdown msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Reset::ID)
			{
				Reset msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Resume::ID)
			{
				Resume msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Standby::ID)
			{
				Standby msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Shutdown::ID)
			{
				Shutdown msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Reset::ID)
			{
				Reset msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == ClearEmergency::ID)
			{
				ClearEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				Receive::Body::ReceiveRec transportData = *(casted_ie->getBody()->getReceiveRec());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg, transportData);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Shutdown::ID)
			{
				Shutdown msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Reset::ID)
			{
				Reset msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ReceiveTransition(msg);
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Initialized") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Initialized* casted_ie = (Initialized*) ie;
				pManagement_ReceiveFSM->context->InitializedTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Ready* casted_ie = (Ready*) ie;
				pManagement_ReceiveFSM->context->ReadyTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Timedout") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Timedout* casted_ie = (Timedout*) ie;
				pManagement_ReceiveFSM->context->TimedoutTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Init") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_InitTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_FailureTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Shutdown") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_ShutdownTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_Emergency") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled_EmergencyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_NotControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Standby") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_StandbyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_Controlled") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready_ControlledTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving_Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_Receiving_ReadyTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("InternalStateChange_To_Management_ReceiveFSM_Receiving") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
				pManagement_ReceiveFSM->context->InternalStateChange_To_Management_ReceiveFSM_ReceivingTransition(ie);
				done = true;
				goto leave;
		}
	} catch (...) {}


leave:
mutex.unlock();
return done;
}


/**
*	This is the function that will check for default transitions if
*  no other transitions were satisfied.
*/
bool ManagementService::defaultTransitions(InternalEvent* ie)
{
bool done = false;

// Since this function can be called from multiple threads,
// we use a mutex to ensure only one state transition is
// active at a time.
mutex.lock();

	// Invoke the FSM transition for this event.
	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Shutdown::ID)
			{
				Shutdown msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ShutdownTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Standby::ID)
			{
				Standby msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->StandbyTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Resume::ID)
			{
				Resume msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ResumeTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Reset::ID)
			{
				Reset msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ResetTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->SetEmergencyTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == ClearEmergency::ID)
			{
				ClearEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ClearEmergencyTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->QueryStatusTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == RequestControl::ID)
			{
				RequestControl msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->RequestControlTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == ReleaseControl::ID)
			{
				ReleaseControl msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->ReleaseControlTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryControl::ID)
			{
				QueryControl msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->QueryControlTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryAuthority::ID)
			{
				QueryAuthority msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->QueryAuthorityTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetAuthority::ID)
			{
				SetAuthority msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->SetAuthorityTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryTimeout::ID)
			{
				QueryTimeout msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->QueryTimeoutTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == CreateEvent::ID)
			{
				CreateEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->CreateEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == CreateCommandEvent::ID)
			{
				CreateCommandEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->CreateCommandEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == UpdateEvent::ID)
			{
				UpdateEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->UpdateEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == CancelEvent::ID)
			{
				CancelEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->CancelEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryEvents::ID)
			{
				QueryEvents msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->QueryEventsTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryEventTimeout::ID)
			{
				QueryEventTimeout msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_ReceiveFSM->context->QueryEventTimeoutTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Initialized") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Initialized* casted_ie = (Initialized*) ie;
				pManagement_ReceiveFSM->context->InitializedTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Ready") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Ready* casted_ie = (Ready*) ie;
				pManagement_ReceiveFSM->context->ReadyTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_ReceiveFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Timedout") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Timedout* casted_ie = (Timedout*) ie;
				pManagement_ReceiveFSM->context->TimedoutTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("EventOccurred") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			EventOccurred* casted_ie = (EventOccurred*) ie;
				pManagement_ReceiveFSM->context->EventOccurredTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("EventError") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			EventError* casted_ie = (EventError*) ie;
				pManagement_ReceiveFSM->context->EventErrorTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Timeout") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Timeout* casted_ie = (Timeout*) ie;
				pManagement_ReceiveFSM->context->TimeoutTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("CommandCompleted") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			CommandCompleted* casted_ie = (CommandCompleted*) ie;
				pManagement_ReceiveFSM->context->CommandCompletedTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("CommandExpired") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			CommandExpired* casted_ie = (CommandExpired*) ie;
				pManagement_ReceiveFSM->context->CommandExpiredTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Send") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			Send* casted_ie = (Send*) ie;
				pManagement_ReceiveFSM->context->SendTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("BroadcastLocal") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			BroadcastLocal* casted_ie = (BroadcastLocal*) ie;
				pManagement_ReceiveFSM->context->BroadcastLocalTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("BroadcastGlobal") == 0 && (ie->getSource().compare("Management_ReceiveFSM") != 0))
		{
			BroadcastGlobal* casted_ie = (BroadcastGlobal*) ie;
				pManagement_ReceiveFSM->context->BroadcastGlobalTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Shutdown::ID)
			{
				Shutdown msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->ShutdownTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Standby::ID)
			{
				Standby msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->StandbyTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Resume::ID)
			{
				Resume msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->ResumeTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == Reset::ID)
			{
				Reset msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->ResetTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetEmergency::ID)
			{
				SetEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->SetEmergencyTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == ClearEmergency::ID)
			{
				ClearEmergency msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->ClearEmergencyTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryStatus::ID)
			{
				QueryStatus msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->QueryStatusTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == RequestControl::ID)
			{
				RequestControl msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->RequestControlTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == ReleaseControl::ID)
			{
				ReleaseControl msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->ReleaseControlTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryControl::ID)
			{
				QueryControl msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->QueryControlTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryAuthority::ID)
			{
				QueryAuthority msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->QueryAuthorityTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == SetAuthority::ID)
			{
				SetAuthority msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->SetAuthorityTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryTimeout::ID)
			{
				QueryTimeout msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->QueryTimeoutTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == CreateEvent::ID)
			{
				CreateEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->CreateEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == CreateCommandEvent::ID)
			{
				CreateCommandEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->CreateCommandEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == UpdateEvent::ID)
			{
				UpdateEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->UpdateEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == CancelEvent::ID)
			{
				CancelEvent msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->CancelEventTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryEvents::ID)
			{
				QueryEvents msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->QueryEventsTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Receive") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Receive* casted_ie = (Receive*) ie;
			unsigned short id = *((unsigned short*) casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
			id = JSIDL_v_1_0::correctEndianness(id);
			if ( id == QueryEventTimeout::ID)
			{
				QueryEventTimeout msg;
				msg.decode(casted_ie->getBody()->getReceiveRec()->getMessagePayload()->getData());
				pManagement_SendFSM->context->QueryEventTimeoutTransition();
				done = true;
				goto leave;
			}
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Initialized") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Initialized* casted_ie = (Initialized*) ie;
				pManagement_SendFSM->context->InitializedTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Ready") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Ready* casted_ie = (Ready*) ie;
				pManagement_SendFSM->context->ReadyTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Failure") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Failure* casted_ie = (Failure*) ie;
				pManagement_SendFSM->context->FailureTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Timedout") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Timedout* casted_ie = (Timedout*) ie;
				pManagement_SendFSM->context->TimedoutTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("EventOccurred") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			EventOccurred* casted_ie = (EventOccurred*) ie;
				pManagement_SendFSM->context->EventOccurredTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("EventError") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			EventError* casted_ie = (EventError*) ie;
				pManagement_SendFSM->context->EventErrorTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Timeout") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Timeout* casted_ie = (Timeout*) ie;
				pManagement_SendFSM->context->TimeoutTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("CommandCompleted") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			CommandCompleted* casted_ie = (CommandCompleted*) ie;
				pManagement_SendFSM->context->CommandCompletedTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("CommandExpired") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			CommandExpired* casted_ie = (CommandExpired*) ie;
				pManagement_SendFSM->context->CommandExpiredTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("Send") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			Send* casted_ie = (Send*) ie;
				pManagement_SendFSM->context->SendTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("BroadcastLocal") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			BroadcastLocal* casted_ie = (BroadcastLocal*) ie;
				pManagement_SendFSM->context->BroadcastLocalTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}

	try
	{
		if ((done == false) && ie->getName().compare("BroadcastGlobal") == 0 && (ie->getSource().compare("Management_SendFSM") != 0))
		{
			BroadcastGlobal* casted_ie = (BroadcastGlobal*) ie;
				pManagement_SendFSM->context->BroadcastGlobalTransition();
				done = true;
				goto leave;
		}
	} catch (...) {}


leave:
mutex.unlock();
return done;
}


};
