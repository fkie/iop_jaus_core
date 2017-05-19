

#ifndef ACCESSCONTROLCLIENTPLUGIN_1_0_H
#define ACCESSCONTROLCLIENTPLUGIN_1_0_H

#include "urn_jaus_jss_core_AccessControlClient_1_0/AccessControlClientService.h"
#include "urn_jaus_jss_core_EventsClient_1_0/EventsClientService.h"
#include "urn_jaus_jss_core_Transport_1_0/TransportService.h"

#include <iop_component_fkie/iop_plugin_interface.h>

namespace iop
{

class DllExport AccessControlClientPlugin_1_0 : public PluginInterface
{
public:
	AccessControlClientPlugin_1_0();

	JTS::Service* get_service();
	void create_service(JTS::JausRouter* jaus_router);

protected:
	urn_jaus_jss_core_AccessControlClient_1_0::AccessControlClientService *p_my_service;
	urn_jaus_jss_core_EventsClient_1_0::EventsClientService *p_base_service;
	urn_jaus_jss_core_Transport_1_0::TransportService *p_transport_service;

};

};

#endif
