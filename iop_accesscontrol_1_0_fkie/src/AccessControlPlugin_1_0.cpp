#include <AccessControlPlugin_1_0.h>
#include <pluginlib/class_list_macros.h>

/** \author Alexander Tiderko */

using namespace iop;
using namespace urn_jaus_jss_core_AccessControl_1_0;
using namespace urn_jaus_jss_core_Events_1_0;
using namespace urn_jaus_jss_core_Transport_1_0;

AccessControlPlugin_1_0::AccessControlPlugin_1_0()
{
	p_my_service = NULL;
	p_transport_service = NULL;
	p_base_service = NULL;
}

AccessControlPlugin_1_0::~AccessControlPlugin_1_0()
{

}

JTS::Service* AccessControlPlugin_1_0::get_iop_service()
{
	return p_my_service;
}

const std::type_info & AccessControlPlugin_1_0::get_iop_service_type()
{
	return typeid(AccessControlService);
}

const std::type_info & AccessControlPlugin_1_0::get_base_service_type()
{
	return typeid(EventsService);
}


void AccessControlPlugin_1_0::create_jts_service(JTS::JausRouter* jaus_router)
{
	p_base_service = dynamic_cast<EventsService *>(get_base_service());
	p_transport_service = dynamic_cast<TransportService *>(get_base_service(2));
	p_my_service = new AccessControlService(jaus_router, p_transport_service, p_base_service);
}

PLUGINLIB_EXPORT_CLASS(iop::AccessControlPlugin_1_0, iop::PluginInterface)
