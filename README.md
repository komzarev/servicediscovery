# ServiceDiscovery

## Description
Library implements service discovery via SSDP protocol (https://en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol). It contains code for both server and client.

Tested on Windows, Linux, QNX.

## Project structure
* servicediscovery - source code files
  * ssdp_qt.hpp/cpp - implementation on the Qt5 network stack
  * ssdp_asio.hpp/cpp - on boost::asio
* test - unit tests 
* examples - server and client examples on Qt and boost::asio
  
## Adding to a project
The library files are included in the project as source code.
See the examples project files for details.

## Description of operation

SSDP server is started as part of a user service or together with it and has parameters:
* type - service type, mandatory parameter.
* name, detail - name and the second part of the name - optional parameters.
* port - port on which the user service is launched.

SSDP client can find all SSDP services available in Ethernet network that have type,name,detail specified in the search. The server response will contain ip address on which it is launched and port value specified at startup.
