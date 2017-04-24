

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

	JTS::Service* get_iop_service();
	const std::type_info & get_iop_service_type();
	const std::type_info & get_base_service_type();
	void create_jts_service(JTS::JausRouter* jaus_router);
	virtual ~AccessControlPlugin_1_0();

	jVariableLengthString get_service_uri() { return "urn:jaus:jss:core:AccessControl"; }
	jUnsignedByte get_version_number_major() { return 1; }
	jUnsignedByte get_version_number_minor() { return 0; }

protected:
	urn_jaus_jss_core_AccessControl_1_0::AccessControlService *p_my_service;
	urn_jaus_jss_core_Transport_1_0::TransportService *p_transport_service;
	urn_jaus_jss_core_Events_1_0::EventsService *p_base_service;

};

};

#endif
