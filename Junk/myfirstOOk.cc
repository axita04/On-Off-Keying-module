/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/on-off-keying-module-helper.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  //Sets us some Log on the UDP Apps
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
//Creates a Container with 2 nodes
  NodeContainer nodes;
  nodes.Create (2);
//Sets up the OOK Help and sets the Channel Delay, and Data rate
  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
  //Sets up the Device Container and installs net devices to the nodes
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);
// Creates an Internetstack and installs nodes to it
  InternetStackHelper stack;
  stack.Install (nodes);
// Set up an IPV4 address Helper and sets the base for the nodes.
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
//Creates a Ipv4 Interface contains and put in the two adresses assoiated with the net devices in the container
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

//Creates a new UDP Echo Server and installs the server application on one of the nodes
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
 //Creates a UDP echo Client  and installs it on to the rest of the nodes that are clients
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));    //Sets umber of packets to send
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0))); // sets timer between sending packets
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));  // Sets size of packet

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0)); // installs nodes
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Simulator::Run ();     //Runs simulation
  Simulator::Destroy (); //Destorys Simulation
  return 0;  //End
}
