

#ifndef ACCESSCONTROLPLUGIN_1_0_H
#define ACCESSCONTROLPLUGIN_1_0_H

#include "urn_jaus_jss_core_AccessControl_1_0/AccessControlService.h"
#include "urn_jaus_jss_core_Events_1_0/EventsService.h"
#include "urn_jaus_jss_core_Transport_1_0/TransportService.h"

#include <iop_component_fkie/iop_plugin_interface.h>

namespace iop
{

class DllExport AccessControlPlugin_1_0 : public PluginInterface
{
public:
	AccessControlPlugin_1_0();

	JTS::Service* get_service();
	void create_service(JTS::JausRouter* jaus_router);

protected:
	urn_jaus_jss_core_AccessControl_1_0::AccessControlService *p_my_service;
	urn_jaus_jss_core_Transport_1_0::TransportService *p_transport_service;
	urn_jaus_jss_core_Events_1_0::EventsService *p_base_service;

};

};

#endif
