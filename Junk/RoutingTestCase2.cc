/*
		p2p
	WifiAp ------ relayAp
		       ^   |
		       |   |
		  wifi |   | VLC
		       |   v
	WifiMt ------ relayMt
		p2p
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/on-off-keying-module-helper.h"
#include "ns3/applications-module.h"
#include "ns3/OOK-error-model.h"
#include "ns3/vlc-propagation-loss-model.h"
#include "ns3/packet-sink.h"
#include "ns3/gnuplot.h"
#include "ns3/double.h"
#include "ns3/mobility-module.h"
#include "ns3/VLC-Mobility-Model.h"
#include "ns3/wifi-module.h"
#include <vector>
#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RoutingTestCase");

void SendStuff(Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port);
void BindSock(Ptr<Socket> sock, Ptr<NetDevice> netdev);
void srcSocketRecv(Ptr<Socket> Socket);
void dstSocketRecv(Ptr<Socket> Socket);
/*
static void
Trace (Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("Sent Mt");
}
*/

int main (int argc, char *argv[])
{


Ptr<Node> wifiAp = CreateObject<Node>();
Ptr<Node> relayAp = CreateObject<Node>();
Ptr<Node> relayMt = CreateObject<Node>();
Ptr<Node> wifiMt = CreateObject<Node>();

NodeContainer c = NodeContainer(wifiAp,relayAp,relayMt,wifiMt);

InternetStackHelper internet;
internet.Install(c);

PointToPointHelper p2p;
p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
p2p.SetChannelAttribute("Delay", StringValue("2ms"));
NetDeviceContainer ndAp_Relay = p2p.Install(wifiAp, relayAp);
//VLC---------------------------------------------------------
 OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer ndRelayAp_RelayMt2 = OOK.Install(relayAp, relayMt);
  
  Ptr<VlcMobilityModel> a = CreateObject<VlcMobilityModel> ();
  Ptr<VlcMobilityModel> b = CreateObject<VlcMobilityModel> ();  

  a -> SetPosition (Vector (0.0,0.0,5.0));
  b -> SetPosition (Vector (0.0,0.0,0.0));
  a ->SetAzimuth(0.0);
  b ->SetAzimuth(0.0);
  a ->SetElevation(0.0);
  b ->SetElevation(0.0);

  OOKErrorModel *em2 ;
  OOKErrorModel x;
  em2 = &x;

  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(48.573);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);

  em2->setRes(0.28);
  em2->setNo(1.0e-11);
  em2->setRb(1.0e6);
  em2->setWavelengths(380,380);
  em2->setTemperature(5000);
  em2->setRx(VPLM.GetRxPower(a,b));

  ndRelayAp_RelayMt2.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));

//------------------------------------------------------------
//Wifi--------------------------------------------------------
  std::string phyMode ("DsssRate1Mbps");
  double rss = -80;  // -dBm

NodeContainer cont = NodeContainer(relayMt, relayAp);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
 
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (0) ); 
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer ndRelayAp_RelayMt3 = wifi.Install (wifiPhy, wifiMac, cont);

  // Note that with FixedRssLossModel, the positions below are not 
  // used for received signal strength. 
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (cont);  

//NetDeviceContainer ndRelayAp_RelayMt3 = p2p.Install(relayMt,relayAp);
//-------------------------------------------------------------
NetDeviceContainer ndRelay_Mt = p2p.Install(relayMt, wifiMt);

Ipv4AddressHelper ipv4;
ipv4.SetBase("10.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer iAp = ipv4.Assign(ndAp_Relay);

ipv4.SetBase("10.1.2.0", "255.255.255.0");
Ipv4InterfaceContainer iRelayApMt = ipv4.Assign(ndRelayAp_RelayMt2);

ipv4.SetBase("10.1.3.0", "255.255.255.0");
Ipv4InterfaceContainer iRelayMtAp = ipv4.Assign(ndRelayAp_RelayMt3);

ipv4.SetBase("10.1.4.0", "255.255.255.0");
Ipv4InterfaceContainer iMt = ipv4.Assign(ndRelay_Mt);

Ptr<Ipv4> ipv4Ap = wifiAp->GetObject<Ipv4>();
Ptr<Ipv4> ipv4RelayAp = relayAp->GetObject<Ipv4>();
Ptr<Ipv4> ipv4RelayMt = relayMt->GetObject<Ipv4>();
Ptr<Ipv4> ipv4Mt = wifiMt->GetObject<Ipv4>();

Ipv4StaticRoutingHelper ipv4RoutingHelper;

Ptr<Ipv4StaticRouting> staticRoutingAp = ipv4RoutingHelper.GetStaticRouting(ipv4Ap);

Ptr<Ipv4StaticRouting> staticRoutingRelayAp = ipv4RoutingHelper.GetStaticRouting(ipv4RelayAp);

Ptr<Ipv4StaticRouting> staticRoutingRelayMt = ipv4RoutingHelper.GetStaticRouting(ipv4RelayMt);

Ptr<Ipv4StaticRouting> staticRoutingMt = ipv4RoutingHelper.GetStaticRouting(ipv4Mt);



staticRoutingAp->AddHostRouteTo(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.1.2"), 1,1);
staticRoutingRelayAp->AddHostRouteTo(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.2.2"), 2,1);
staticRoutingRelayMt->AddHostRouteTo(Ipv4Address("10.1.4.2"), Ipv4Address("10.1.4.2"), 3,1);

staticRoutingMt->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.4.1"), 1,1);
staticRoutingRelayMt->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.3.2"), 2,1);
staticRoutingRelayAp->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.1"), 1,1);


Ptr<Socket> ApSocket = Socket::CreateSocket(wifiAp,TypeId::LookupByName("ns3::UdpSocketFactory"));
ApSocket->Bind();
ApSocket->SetRecvCallback (MakeCallback(&srcSocketRecv));

Ptr<Socket> MtSocket = Socket::CreateSocket(wifiMt, TypeId::LookupByName("ns3::UdpSocketFactory"));
uint16_t dstport = 12345;
Ipv4Address dstaddr ("10.1.4.2");
InetSocketAddress dstsa = InetSocketAddress(dstaddr, dstport);
MtSocket->Bind(dstsa);
MtSocket->SetRecvCallback(MakeCallback(&dstSocketRecv));

AsciiTraceHelper ascii;
p2p.EnableAsciiAll(ascii.CreateFileStream ("RoutingTestCase.tr"));
p2p.EnablePcapAll("RoutingTestCase");

LogComponentEnableAll(LOG_PREFIX_TIME);
LogComponentEnable("RoutingTestCase", LOG_LEVEL_INFO);

Ptr<OutputStreamWrapper> stream1 = Create<OutputStreamWrapper> ("Table5", std::ios::out);
ipv4RoutingHelper.PrintRoutingTableAllAt(Seconds(2.0), stream1);

//ndAp_Relay.Get (1)->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback (&Trace));


Simulator::Schedule(Seconds(0.1), &SendStuff,ApSocket, dstaddr, dstport);
Simulator::Run();
Simulator::Destroy();

return 0;
}


void SendStuff (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port)
   {
    // std::cout<<"SEND"<<std::endl;
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
