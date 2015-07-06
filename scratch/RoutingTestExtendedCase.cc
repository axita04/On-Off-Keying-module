#include <iostream>
#include <fstream>
#include <ostream>
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
#include "ns3/Aerror-model.h"
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
static const uint32_t totalTxBytes = 1000000; //The simulation with send 1000000 bytes in data packets (not including overhead)
static uint32_t currentTxBytes = 0;
static const uint32_t writeSize = 1040;// How big each packet will be, default for TCP is 536 w/out headers
uint8_t data[writeSize];
void StartFlow (Ptr<Socket>, Ipv4Address, uint16_t); //send data
void WriteUntilBufferFull (Ptr<Socket>, uint32_t);


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
//Sinks nd packet sinks used for tracing on the various client nodes
Ptr<PacketSink> sink1;
Ptr<PacketSink> sink2;
Ptr<PacketSink> sink3;
std::vector<double> Received (1,0);
std::vector<double> Received2 (1,0);
std::vector<double> Received3 (1,0);
std::vector<double> ReceivedT (1,0);
std::vector<double> theTime (1,0);
std::vector<double> theTime2 (1,0);
std::vector<double> theTime3 (1,0);
std::vector<double> theTimeT (1,0);

static void
RxEnd (Ptr<const Packet> p)//used for tracing and calculating throughput
{
  Received.push_back(Received.back() + p->GetSize());//appends on the received packet to the received data up until that packet and adds that total to the end of the vector
  theTime.push_back(Simulator::Now().GetSeconds());// keeps track of the time during simulation that a packet is received


   ReceivedT.push_back(ReceivedT.back() + p->GetSize());// same as above except for the fact that this is used for the total throughput
  theTimeT.push_back(Simulator::Now().GetSeconds());// rather than throughput for a specific client
//  NS_LOG_UNCOND ("Received : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
}

static void
TxEnd (Ptr<const Packet> p)//also used as a trace and for calculating throughput
{
 //NS_LOG_UNCOND ("Sent : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
  Received.push_back(Received.back() + p->GetSize());//same as for RX end trace
  theTime.push_back(Simulator::Now().GetSeconds());//
 
     ReceivedT.push_back(ReceivedT.back() + p->GetSize());//same as for RX end trace
  theTimeT.push_back(Simulator::Now().GetSeconds());//
   
}

static void
RxEnd2 (Ptr<const Packet> p)//used for second node
{

  Received2.push_back(Received2.back() + p->GetSize());
  theTime2.push_back(Simulator::Now().GetSeconds());

     ReceivedT.push_back(ReceivedT.back() + p->GetSize());
  theTimeT.push_back(Simulator::Now().GetSeconds());

  //NS_LOG_UNCOND ("Received : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
}

static void
TxEnd2 (Ptr<const Packet> p)//used for second node
{
 //NS_LOG_UNCOND ("Sent : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
  Received2.push_back(Received2.back() + p->GetSize());
  theTime2.push_back(Simulator::Now().GetSeconds());
 

      ReceivedT.push_back(ReceivedT.back() + p->GetSize());
  theTimeT.push_back(Simulator::Now().GetSeconds());
}

static void
RxEnd3 (Ptr<const Packet> p) // used for third node
{

  Received3.push_back(Received3.back() + p->GetSize());
  theTime3.push_back(Simulator::Now().GetSeconds());


     ReceivedT.push_back(ReceivedT.back() + p->GetSize());
  theTimeT.push_back(Simulator::Now().GetSeconds());
  //NS_LOG_UNCOND ("Received : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
}

static void
TxEnd3 (Ptr<const Packet> p)//used for third node
{
 //NS_LOG_UNCOND ("Sent : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
  Received3.push_back(Received3.back() + p->GetSize());
  theTime3.push_back(Simulator::Now().GetSeconds());
 

      ReceivedT.push_back(ReceivedT.back() + p->GetSize());
  theTimeT.push_back(Simulator::Now().GetSeconds());
}


int main (int argc, char *argv[])
{
//used instead of gnuplot for monitoring reasons while the simulation is running but the data can still be plotted later with gnuplot
std::ofstream myfile1;
myfile1.open("node1.dat");

std::ofstream myfile2;
myfile2.open("node2.dat");

std::ofstream myfile3;
myfile3.open("node3.dat");

std::ofstream myfile;
myfile.open("total.dat");


  for(double dist = 6.50 ; dist < 8.1 ; dist+=.001){//loops the simulation by increasing distance between nodes


//creating each node object
Ptr<Node> Ap = CreateObject<Node>();
Ptr<Node> RouterAp = CreateObject<Node>();
Ptr<Node> relay1 = CreateObject<Node>();
Ptr<Node> Mt1 = CreateObject<Node>();
Ptr<Node> relay2 = CreateObject<Node>();
Ptr<Node> Mt2 = CreateObject<Node>();
Ptr<Node> relay3 = CreateObject<Node>();
Ptr<Node> Mt3 = CreateObject<Node>();

//puts all the nodes into one place
NodeContainer c = NodeContainer(Ap,RouterAp);
c.Add(relay1);
c.Add(Mt1);
c.Add(relay2);
c.Add(Mt2);
c.Add(relay3);
c.Add(Mt3);


InternetStackHelper internet;//This helper handles making all the components of the internet stack that will be layered on top on the already exsisting network
internet.Install(c);

//This helper sets up the P2P connections that we will be using
PointToPointHelper p2p;
p2p.SetDeviceAttribute("DataRate", StringValue("200Mbps"));
p2p.SetChannelAttribute("Delay", StringValue("2ms"));
NetDeviceContainer ndAp_Router = p2p.Install(Ap, RouterAp);
//VLC---------------------------------------------------------
 OOKHelper OOK;// This helper makes the VLC channel that we are going to use
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  NetDeviceContainer ndRouterAp_RelayMt1 = OOK.Install(RouterAp, relay1);
  NetDeviceContainer ndRouterAp_RelayMt2 = OOK.Install(RouterAp, relay2);
  NetDeviceContainer ndRouterAp_RelayMt3 = OOK.Install(RouterAp, relay3);

  Ptr<VlcMobilityModel> a = CreateObject<VlcMobilityModel> ();//These vectors are what represent the nodes moving
  Ptr<VlcMobilityModel> b = CreateObject<VlcMobilityModel> ();//in space  

  a -> SetPosition (Vector (0.0,0.0,dist));
  b -> SetPosition (Vector (0.0,0.0,0.0));
  a ->SetAzimuth(0.0);
  b ->SetAzimuth(0.0);
  a ->SetElevation(0.0);
  b ->SetElevation(0.0);

  //Instaniates an Error model to use on the VLC net devices
  AErrorModel *em2 ;
  AErrorModel x;
  em2 = &x;

  //Sets the initial conditions of the transmitter and receiver in the VLC network
  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(48.573);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);

  //Also initial conditions, but these are made in the error model since thats where the values are used to calculate BER
  em2->setRes(0.28);
  em2->setNo(1.0e-11);
  em2->setRb(1.0e6);
  em2->setWavelengths(380,380);
  em2->setTemperature(5000);
  em2->setRx(VPLM.GetRxPower(a,b));

  //putting the error model on each VLC device
  ndRouterAp_RelayMt1.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
  ndRouterAp_RelayMt1.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));

  ndRouterAp_RelayMt2.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
  ndRouterAp_RelayMt2.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));

  ndRouterAp_RelayMt3.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
  ndRouterAp_RelayMt3.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
  

//------------------------------------------------------------
//Wifi--------------------------------------------------------
 std::string phyMode ("DsssRate11Mbps");
  double rss = -80;  // -dBm

NodeContainer cont = NodeContainer(RouterAp, relay1);
cont.Add(relay2);
cont.Add(relay3);
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
  NetDeviceContainer ndRelayMt_RouterAp = wifi.Install (wifiPhy, wifiMac, cont);

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
NetDeviceContainer ndRelay_Mt1 = p2p.Install(relay1, Mt1);
NetDeviceContainer ndRelay_Mt2 = p2p.Install(relay2, Mt2);
NetDeviceContainer ndRelay_Mt3 = p2p.Install(relay3, Mt3);

//The following sets up address bases for out net devices on the nodes so as to identify them on a routing table as we do
Ipv4AddressHelper ipv4;
ipv4.SetBase("10.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer iAp = ipv4.Assign(ndAp_Router);

ipv4.SetBase("10.1.2.0", "255.255.255.0");
Ipv4InterfaceContainer iRelayApMt1 = ipv4.Assign(ndRouterAp_RelayMt1);

ipv4.SetBase("10.1.3.0", "255.255.255.0");
Ipv4InterfaceContainer iRelayApMt2 = ipv4.Assign(ndRouterAp_RelayMt2);

ipv4.SetBase("10.1.4.0", "255.255.255.0");
Ipv4InterfaceContainer iRelayApMt3 = ipv4.Assign(ndRouterAp_RelayMt3);

ipv4.SetBase("10.1.5.0", "255.255.255.0");
Ipv4InterfaceContainer iwifi = ipv4.Assign(ndRelayMt_RouterAp);

ipv4.SetBase("10.1.6.0", "255.255.255.0");
Ipv4InterfaceContainer iMt1 = ipv4.Assign(ndRelay_Mt1);

ipv4.SetBase("10.1.7.0", "255.255.255.0");
Ipv4InterfaceContainer iMt2 = ipv4.Assign(ndRelay_Mt2);

ipv4.SetBase("10.1.8.0", "255.255.255.0");
Ipv4InterfaceContainer iMt3 = ipv4.Assign(ndRelay_Mt3);

//The following sets up each nodes routing table that will be statically added to
Ptr<Ipv4> ipv4Ap = Ap->GetObject<Ipv4>();
Ptr<Ipv4> ipv4RouterAp = RouterAp->GetObject<Ipv4>();
Ptr<Ipv4> ipv4RelayMt1 = relay1->GetObject<Ipv4>();
Ptr<Ipv4> ipv4RelayMt2 = relay2->GetObject<Ipv4>();
Ptr<Ipv4> ipv4RelayMt3 = relay3->GetObject<Ipv4>();
Ptr<Ipv4> ipv4Mt1 = Mt1->GetObject<Ipv4>();
Ptr<Ipv4> ipv4Mt2 = Mt2->GetObject<Ipv4>();
Ptr<Ipv4> ipv4Mt3 = Mt3->GetObject<Ipv4>();

Ipv4StaticRoutingHelper ipv4RoutingHelper;

Ptr<Ipv4StaticRouting> staticRoutingAp = ipv4RoutingHelper.GetStaticRouting(ipv4Ap);
Ptr<Ipv4StaticRouting> staticRoutingRouterAp = ipv4RoutingHelper.GetStaticRouting(ipv4RouterAp);
Ptr<Ipv4StaticRouting> staticRoutingRelayMt1 = ipv4RoutingHelper.GetStaticRouting(ipv4RelayMt1);
Ptr<Ipv4StaticRouting> staticRoutingRelayMt2 = ipv4RoutingHelper.GetStaticRouting(ipv4RelayMt2);
Ptr<Ipv4StaticRouting> staticRoutingRelayMt3 = ipv4RoutingHelper.GetStaticRouting(ipv4RelayMt3);
Ptr<Ipv4StaticRouting> staticRoutingMt1 = ipv4RoutingHelper.GetStaticRouting(ipv4Mt1);
Ptr<Ipv4StaticRouting> staticRoutingMt2 = ipv4RoutingHelper.GetStaticRouting(ipv4Mt2);
Ptr<Ipv4StaticRouting> staticRoutingMt3 = ipv4RoutingHelper.GetStaticRouting(ipv4Mt3);


//The following are the specific routes added to various routing tables and this current scheme is modeing a VLC downlink and a WIFI uplink
staticRoutingAp->AddHostRouteTo(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.1.2"), 1,1);  //Route From AP to MT1 : Gateway Router AP : Interface 1
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.2.2"), 2,1);//Route from Router AP to MT1 : Gateway Relay MT1 : Interface 2
staticRoutingRelayMt1->AddHostRouteTo(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.6.2"), 3,1);//Route from Relay MT1 to MT1 : Gateway MT1 : Interface 3

staticRoutingAp->AddHostRouteTo(Ipv4Address("10.1.7.2"), Ipv4Address("10.1.1.2"), 1,1); //Route From AP to MT2 : Gateway Router AP : Interface 1
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.7.2"), Ipv4Address("10.1.3.2"), 3,1);//Route from Router AP to MT2 : Gateway Relay MT2 : Interface 3
staticRoutingRelayMt2->AddHostRouteTo(Ipv4Address("10.1.7.2"), Ipv4Address("10.1.7.2"), 3,1);//Route from Relay MT2 to MT2 : Gateway MT2 : Interface 3

staticRoutingAp->AddHostRouteTo(Ipv4Address("10.1.8.2"), Ipv4Address("10.1.1.2"), 1,1);//Route From AP to MT3 : Gateway Router AP : Interface 1
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.8.2"), Ipv4Address("10.1.4.2"), 4,1);//Route from Router AP to MT3 : Gateway Relay MT3 : Interface 4
staticRoutingRelayMt3->AddHostRouteTo(Ipv4Address("10.1.8.2"), Ipv4Address("10.1.8.2"), 3,1);//Route from Relay MT3 to MT3 : Gateway MT3 : Interface 3

staticRoutingMt1->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.6.1"), 1,1); //Route From MT1 to AP : Gateway Relay MT1 : Interface 1
staticRoutingRelayMt1->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.5.1"), 2,1);//Route From Relay MT1 to AP : Gateway Router AP : Interface 2
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.1"), 1,1);//Route From Router AP to AP : Gateway AP : Interface 1

staticRoutingMt2->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.7.1"), 1,1); //Route From MT2 to AP : Gateway Relay MT1 : Interface 1
staticRoutingRelayMt2->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.5.1"), 2,1);//Route From Relay MT2 to AP : Gateway Router AP : Interface 2
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.1"), 1,1);//Route From Router AP to AP : Gateway AP : Interface 1

staticRoutingMt3->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.8.1"), 1,1); //Route From MT3 to AP : Gateway Relay MT1 : Interface 1
staticRoutingRelayMt3->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.5.1"), 2,1);//Route From Relay MT3 to AP : Gateway Router AP : Interface 2
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.1"), 1,1);//Route From Router AP to AP : Gateway AP : Interface 1

//This sets up various sockets on the same node as to allow multiple TCP connections to be made as to pass information through the net devices
 Ptr<Socket> srcSocket1 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket2 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket3 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket4 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
 Ptr<Socket> srcSocket5 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket6 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket7 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket8 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));

//The following sets up ports on the various clients
  uint16_t dstport1 = 12345;
  Ipv4Address dstaddr1 ("10.1.6.2");

  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dstport1));
  ApplicationContainer apps = sink.Install (Mt1);
  sink1 = DynamicCast<PacketSink>(apps.Get(0));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (10.0));
 
 uint16_t dstport2 = 12346;
  Ipv4Address dstaddr2 ("10.1.7.2");

  PacketSinkHelper sinka ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dstport2));
  ApplicationContainer apps2 = sinka.Install (Mt2);
  sink2 = DynamicCast<PacketSink>(apps2.Get(0));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (10.0));


uint16_t dstport3 = 12347;
  Ipv4Address dstaddr3 ("10.1.8.2");

  PacketSinkHelper sinkb ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dstport3));
  ApplicationContainer apps3 = sinkb.Install (Mt3);
  sink3 = DynamicCast<PacketSink>(apps3.Get(0));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (10.0));


//The following is used for logginbg and various debugging purposes
AsciiTraceHelper ascii;
p2p.EnableAsciiAll(ascii.CreateFileStream ("RoutingTestCase.tr"));
p2p.EnablePcapAll("RoutingTestCase");

LogComponentEnableAll(LOG_PREFIX_TIME);
LogComponentEnable("RoutingTestCase", LOG_LEVEL_INFO);

Ptr<OutputStreamWrapper> stream1 = Create<OutputStreamWrapper> ("Table3", std::ios::out);
ipv4RoutingHelper.PrintRoutingTableAllAt(Seconds(2.0), stream1);


//Trace set ups
ndRelay_Mt3.Get (1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd3));

ndRelay_Mt3.Get (1)->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback (&TxEnd3));

ndRelay_Mt1.Get (1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

ndRelay_Mt1.Get (1)->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback (&TxEnd));

ndRelay_Mt2.Get (1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd2));

ndRelay_Mt2.Get (1)->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback (&TxEnd2));

//simulator schedule
Simulator::Schedule(Seconds(0.1), &StartFlow,srcSocket1, dstaddr3, dstport3);

Simulator::Schedule(Seconds(0.1), &StartFlow,srcSocket2, dstaddr1, dstport1);

Simulator::Schedule(Seconds(0.1), &StartFlow,srcSocket3, dstaddr2, dstport2);

Simulator::Run();

//various throughput calculations
double throughput1 = ((Received.back()*8))/ theTime.back();
double throughput2 = ((Received2.back()*8))/ theTime2.back();
double throughput3 = ((Received3.back()*8))/ theTime3.back();
double totalThroughput = ((ReceivedT.back()*8))/ theTimeT.back();

//std::cout<<"-------------------------"<< std::endl;
//std::cout<<"Received : " << Received.back() << std::endl;
//std::cout<<"Distance : " << dist << std::endl;
//std::cout<<"Time : " << theTime.back() << std::endl;
//std::cout<<"THROUGHPUT : " << throughput << std::endl;
//std::cout<<"BER : " << em2->getBER() << std::endl;

//writing to the files that will carry the data to be graphed
myfile <<dist << " " << totalThroughput <<std::endl;
myfile1 <<dist << " " << throughput1 <<std::endl;
myfile2 <<dist << " " << throughput2 <<std::endl;
myfile3 <<dist << " " << throughput3 <<std::endl; 

//the following clears the data received vectors so as to avoid calculation errors from old and irrelevant values
Received.clear();
Received2.clear();
Received3.clear();
ReceivedT.clear();



Simulator::Destroy();

}
 
//closes stream files
myfile.close();
myfile1.close();
myfile2.close();
myfile3.close();

return 0;
}

   
void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev)
   {
     sock->BindToNetDevice (netdev);
     return;
   }
void StartFlow (Ptr<Socket> localSocket,
                Ipv4Address servAddress,
                uint16_t servPort)
{
  //NS_LOG_INFO ("Starting flow at time " <<  Simulator::Now ().GetSeconds ());
  currentTxBytes = 0;
  localSocket->Bind ();
  localSocket->Connect (InetSocketAddress (servAddress, servPort)); //connect

  // tell the tcp implementation to call WriteUntilBufferFull again
  // if we blocked and new tx buffer space becomes available
  localSocket->SetSendCallback (MakeCallback (&WriteUntilBufferFull));
  WriteUntilBufferFull (localSocket, localSocket->GetTxAvailable ());
}

void WriteUntilBufferFull (Ptr<Socket> localSocket, uint32_t txSpace)
{
  while (currentTxBytes < totalTxBytes && localSocket->GetTxAvailable () > 0)
    {
      uint32_t left = totalTxBytes - currentTxBytes;
      uint32_t dataOffset = currentTxBytes % writeSize;
      uint32_t toWrite = writeSize - dataOffset;
      toWrite = std::min (toWrite, left);
      toWrite = std::min (toWrite, localSocket->GetTxAvailable ());
      int amountSent = localSocket->Send (&data[dataOffset], toWrite, 0);
      if(amountSent < 0)
        {
          // we will be called again when new tx space becomes available.
          return;
        }
      currentTxBytes += amountSent;
    }
  localSocket->Close ();
}
