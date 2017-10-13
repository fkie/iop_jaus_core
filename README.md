See [iop_core](https://github.com/fkie/iop_core/blob/master/README.md) for use instructions.

```
```

# Interfaces

List of services in this repository:
```
iop_accesscontrol_fkie: AccessControl
iop_accesscontrol_fkie: AccessControlClient
iop_discovery_fkie: Discovery
iop_discovery_fkie: DiscoveryClient
iop_events_fkie: Events
iop_events_fkie: EventsClient
iop_liveness_fkie: Liveness
iop_management_fkie: Management
iop_management_fkie: ManagementClient
iop_transport_fkie: Transport
```

## _iop_accesscontrol_fkie:_ AccessControl

Implements the functionality for exclusive control. By default a timeout of 10 seconds is active. You have to send your requests within this time as not to lose the control.

#### Parameter:

_access_timeout_ (int_, (Default: 10)

> Time period in seconds after which the exclusive control goes lost. Zero disables the timeout.


#### Publisher:

_is_controlled (std_msgs::Bool)_, latched

> Publishes True or False if the service is controlled by someone or not

#### Subscriber:

> None

## _iop_accesscontrol_fkie:_ AccessControlClient

The client service for AccessControl. It offers the functionality to send access requests in periodic time specified by AccessControl service.



## _iop_discovery_fkie:_ Discovery

The discovery service holds information about System, Subsystem, Node or Component depend on the configuration parameter _system_id_. It should by only one service with type __Subsystem__ in your subsystem. Each component can include this service to provide a name of the component.

#### Parameter:

_system_id_ (int_, (Default: 4)

> The ID of the service: 0: Reserved, 1: System Identification, 2: Subsystem Identification, 3: Node Identification, 4: Component Identification, 5 – 255: Reserved

_system_type_ (int_, (Default: 60001)

> 10001: VEHICLE, 20001: OCU, 30001: OTHER_SUBSYSTEM, 40001: NODE, 50001: PAYLOAD, 60001: COMPONENT

_name_subsystem_ (str_, (Default: Robotname)

> The name of the robot. Only used if the _system_id_ is set to 2 (subsystem).

_name_node_ (str_, (Default: Componentname)

> The name of the component which includes this service.


#### Publisher:

> None

#### Subscriber:

> None


## _iop_discovery_fkie:_ DiscoveryClient

A client service to discover other IOP services and register own services by Discovery service of _subsystem_ type. It provide also a ROS publisher with all discovered services.

#### Parameter:

_system_id_ (int_, (Default: 4)

> This parameter influence the discover update behaviour only if ROS interface is disabled and all IOP services are already discovered. The same type as in Discovery.

_register_own_services_ (bool_, (Default: true)

> You can prevent the registration of own services by discovery service. It is useful in case of OCU services.

_force_component_update_after_ (int_, (Default: 300)

> Force the request of service list for all known robot.

_enable_ros_interface_ (bool_, (Default: false)

> Publish the discovered services to the ROS network. On false the publisher are not created.


#### Publisher:

_/iop_identification (iop_msgs_fkie::Identification)_

> Publishes the identification report to the ROS network.

_/iop_system (iop_msgs_fkie::System)_, latched

> Publishes all services of all discovered subsystems on every detected change.

#### Subscriber:

> None

#### Services:

_/iop_query_identification (iop_msgs_fkie::QueryIdentification)_

> Request an identification for provided system id.

_/iop_update_discovery (std_srvs::Empty)_

> Tries to update the whole system.


#To be continued...
