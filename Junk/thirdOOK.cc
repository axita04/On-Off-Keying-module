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
#include "ns3/on-off-keying-module-helper.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n4   n3   n2   n1 -------------- n0  
//                         OOK  

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int 
main (int argc, char *argv[])
{
  bool verbose = true;  //True for logging flase for no logging
    uint32_t nWifi = 3;
  CommandLine cmd; // Allow individuals to change the following values via command line.
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (nWifi > 18) // Makes sure the number of nodes do not exceed the max
    {
      std::cout << "Number of wifi nodes " << nWifi << 
                   " specified exceeds the mobility bounding box" << std::endl;
      exit (1);
    }

  if (verbose) //Sets up loggin if Verbose is true on the Applications
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
//Mades a node container with 2 nodes
  NodeContainer OOKNodes;
  OOKNodes.Create (2);
//Sets up the helper with Data rate and Delay values
  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//Sets up a Device Container and install net devices to the nodes
  NetDeviceContainer OOKDevices;
  OOKDevices = OOK.Install (OOKNodes);
//Makes a node container specific to the Sation nodes.
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  //Make a node for the Access Point nodes
  NodeContainer wifiApNode = OOKNodes.Get (1);
  // Sets up a channel Helper with the default setup 
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  //Creates the PHY layer with a Default set up and used the channel helper to make a channel 
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  //Creates a Wifi helper with default setting 
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
 //Creates a Non-Qos Wifi Mac layer helper
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  //Makes a new SSID and sets up the Mac layer settings with it
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  //Makes a Device container  and create Devices for every Station node with the PHY and MAC  
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  //Changes the Make type to Access point 
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
//Makes a container for the AP devices and insall the devices with the new mac and phy to the nodes
  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);
//set up Mobility for the wifi nodes.
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  //Sets up a model to randomly walk in a 2d plane
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  //Installs the Mobility model to the Station nodes
  mobility.Install (wifiStaNodes);
 //Sets up and install a constant position model to te Access point node
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
//Sets up an Internet Stack and install all of the nodes
  InternetStackHelper stack;
  stack.Install (OOKNodes.Get(0));
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

//Creates and Ipv4 helper and assignes Adresses  to all nodes
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer OOKInterfaces;
  OOKInterfaces = address.Assign (OOKDevices);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

//Sets up the UDP echo Server application and install thems
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (OOKNodes.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
//Sets up the UDP echo Client application and install thems
  UdpEchoClientHelper echoClient (OOKInterfaces.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));    //Sets up the numner of packets
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0))); //sets up interval between packets
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));  // sets up packet size

  ApplicationContainer clientApps = 
    echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
//Makes routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));
//Enbles Pcap logging for this simulation
  OOK.EnablePcapAll ("third");
  phy.EnablePcap ("third", apDevices.Get (0));
  OOK.EnablePcap ("third", OOKDevices.Get (0), true);

//Runs and Destorys Simulation
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
