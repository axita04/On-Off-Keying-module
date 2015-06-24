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
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

 void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port);
 void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev);
 void srcSocketRecv (Ptr<Socket> socket);
 void dstSocketRecv (Ptr<Socket> socket);
 //static void RxEnd(Ptr<const Packet> p);
 
int
main (int argc, char *argv[])
{
  Ptr<Node> A = CreateObject<Node>();
  Ptr<Node> B = CreateObject<Node>();
  Ptr<Node> C = CreateObject<Node>();

  NodeContainer all = NodeContainer(A,B,C);
  
  InternetStackHelper stack;
  stack.Install (all);

  NodeContainer A_B = NodeContainer(A,B);
  NodeContainer B_C = NodeContainer(B,C);
  //NodeContainer C_A = NodeContainer(C,A);


  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer nd_A_B = p2p.Install(A_B);
  NetDeviceContainer nd_B_C = p2p.Install(B_C);
  //NetDeviceContainer nd_C_A = p2p.Install(C_A);  

  Ptr<NetDevice> fromA = nd_A_B.Get(0);  
  //Ptr<NetDevice> fromB1 = nd_A_B.Get(1);  
  //Ptr<NetDevice> fromB2 = nd_B_C.Get(0);  

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer inter_A_B = address.Assign (nd_A_B);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer inter_B_C = address.Assign (nd_B_C);
  //address.SetBase ("10.1.3.0", "255.255.255.0");
  //Ipv4InterfaceContainer inter_C_A = address.Assign (nd_C_A);
  
  Ptr<Ipv4> ipv4A = A->GetObject<Ipv4>();
  Ptr<Ipv4> ipv4B = B->GetObject<Ipv4>();
  Ptr<Ipv4> ipv4C = C->GetObject<Ipv4>();

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRoutingA = ipv4RoutingHelper.GetStaticRouting(ipv4A);
  Ptr<Ipv4StaticRouting> staticRoutingB = ipv4RoutingHelper.GetStaticRouting(ipv4B);
  Ptr<Ipv4StaticRouting> staticRoutingC = ipv4RoutingHelper.GetStaticRouting(ipv4C);
 

  staticRoutingA -> AddHostRouteTo(Ipv4Address("10.1.2.2"),Ipv4Address("10.1.2.1"),2);
  //staticRoutingC -> AddHostRouteTo(Ipv4Address("10.1.3.2"),Ipv4Address("10.1.1.2"),2);
  //staticRoutingB -> AddHostRouteTo(Ipv4Address("10.1.2.2"),Ipv4Address("0.0.0.0"),1);

  Ptr<Socket> Asocket = Socket::CreateSocket(A, TypeId::LookupByName("ns3::UdpSocketFactory"));
  Asocket->Bind();
  Asocket->SetRecvCallback(MakeCallback(&srcSocketRecv));

  Ptr<Socket> Csocket = Socket::CreateSocket(C, TypeId::LookupByName("ns3::UdpSocketFactory"));
  uint16_t Cport = 12345;
  Ipv4Address Caddr("10.1.2.2");
  InetSocketAddress Cdst = InetSocketAddress(Caddr, Cport);
  Csocket->Bind(Cdst);
  Csocket->SetRecvCallback(MakeCallback(&dstSocketRecv));

  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll(ascii.CreateFileStream("myfirst-tr"));
  p2p.EnablePcapAll("myfirst");

  LogComponentEnableAll(LOG_PREFIX_TIME);
  LogComponentEnable("FirstScriptExample", LOG_LEVEL_INFO);

  //fromB1->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

  //fromB2->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

  // First packet as normal (goes via Rtr1)
  Simulator::Schedule (Seconds (0.1),&SendStuff, Asocket, Caddr, Cport);
  // Second via Rtr1 explicitly
  Simulator::Schedule (Seconds (1.0),&BindSock, Asocket, fromA);
  Simulator::Schedule (Seconds ( 1.1),&SendStuff, Asocket, Caddr, Cport);

  
  // If you uncomment what's 

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
/*static void RxEnd (Ptr<const Packet> p)
{
 
  std::cout<<"SOMETHING IS AT B"<< std::endl;
 // NS_LOG_UNCOND ("Rx Received at " << Simulator::Now().GetSeconds());
}*/


void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port)
   {
     Ptr<Packet> p = Create<Packet> ();
     p->AddPaddingAtEnd (100);
     sock->SendTo (p, 0, InetSocketAddress (dstaddr,port));
     return;
   }
   
void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev)
   {
     sock->BindToNetDevice (netdev);
     return;
   }

void srcSocketRecv(Ptr<Socket> socket)
{
 Address from;
     Ptr<Packet> packet = socket->RecvFrom (from);
     packet->RemoveAllPacketTags ();
     packet->RemoveAllByteTags ();
     NS_LOG_INFO ("Source Received " << packet->GetSize () << " bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 ());
     if (socket->GetBoundNetDevice ())
       {
         NS_LOG_INFO ("Socket was bound");
       } 
     else
       {
         NS_LOG_INFO ("Socket was not bound");
       }

}
void dstSocketRecv(Ptr<Socket> socket)
{
  Address from;
    Ptr<Packet> packet = socket->RecvFrom (from);
    packet->RemoveAllPacketTags ();
    packet->RemoveAllByteTags ();
    InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
    NS_LOG_INFO ("Destination Received " << packet->GetSize () << " bytes from " << address.GetIpv4 ());
    NS_LOG_INFO ("Triggering packet back to source node's interface 1");
    SendStuff (socket, Ipv4Address ("10.1.1.1"), address.GetPort ());

}
