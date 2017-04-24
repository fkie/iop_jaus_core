

#ifndef TRANSPORTSERVICE_H
#define TRANSPORTSERVICE_H

#include "Service.h"
#include "InternalEvents/InternalEventHandler.h"
#include "Transport/JausTransport.h"
#include "JConstants.h"
#include "urn_jaus_jss_core_Transport_1_0/Messages/MessageSet.h"
#include "urn_jaus_jss_core_Transport_1_0/InternalEvents/InternalEventsSet.h"
#include "Transport/OS.h"

#include "Transport_ReceiveFSM.h"
#include "Transport_SendFSM.h"


namespace urn_jaus_jss_core_Transport_1_0
{

class DllExport TransportService : public JTS::Service
{
public:
	TransportService( JTS::JausRouter* jausRouter );
	virtual ~TransportService();

	virtual bool processTransitions(JTS::InternalEvent* ie);
	virtual bool defaultTransitions(JTS::InternalEvent* ie);

	// FSMs are public so that children services can access them
	Transport_ReceiveFSM* pTransport_ReceiveFSM;
	Transport_SendFSM* pTransport_SendFSM;

        jVariableLengthString getUri() { return "urn:jaus:jss:core:Transport"; }
        jUnsignedByte getMajorVersionNumber() { return 1; }
        jUnsignedByte getMinorVersionNumber() { return 0; }



protected:
	virtual void run();

};

};

#endif // TRANSPORTSERVICE_H
