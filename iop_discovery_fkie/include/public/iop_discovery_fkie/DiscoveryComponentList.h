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


#ifndef DISCOVERY_COMPONENT_LIST_H
#define DISCOVERY_COMPONENT_LIST_H

#include "Transport/JausTransport.h"
#include "DiscoveryComponent.h"

namespace iop
{

class DiscoveryComponentList {
public:
	/**
	 * :param own_address: own services can not be expired. We need this address to identify own services.
	 * :param timeout: after this timeout a component will be removed on access or update. Zero disables timeout. */
	DiscoveryComponentList(JausAddress own_address=JausAddress(0), unsigned int timeout=60);
	void set_own_address(JausAddress own_address);
	void set_timeout(unsigned int timeout);

	bool add_service(JausAddress component, std::string service_uri, unsigned char major_version, unsigned char minor_version=255);
	/** Returns false if component not in the list or was expired and removed since last update. */
	bool update_ts(JausAddress component);
	std::vector<DiscoveryComponent> get_components(unsigned short subsystem=65535, unsigned char node=255, unsigned char component=255);

protected:
	std::vector<DiscoveryComponent> p_components;
	JausAddress p_own_address;
	unsigned int p_timeout;

	bool p_expired(unsigned int ts, unsigned int now=0);
};

};

#endif // DISCOVERY_CONFIG_H
