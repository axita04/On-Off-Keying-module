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
#include "ns3/OOKerror-model.h"
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
static const uint32_t totalTxBytes = 1000000;
static uint32_t currentTxBytes = 0;
static const uint32_t writeSize = 1040;
uint8_t data[writeSize];
void StartFlow (Ptr<Socket>, Ipv4Address, uint16_t);
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
Ptr<PacketSink> sink1;
Ptr<PacketSink> sink2;
Ptr<PacketSink> sink3;
std::vector<double> Received (1,0);
std::vector<double> theTime (1,0);

static void
RxEnd (Ptr<const Packet> p)
{
 // if(Received.back() != sink1->GetTotalRx()){
 //   Received.push_back(sink1->GetTotalRx());
 //   theTime.push_back(Simulator::Now().GetSeconds());
 // }

  Received.push_back(Received.back() + p->GetSize());
  theTime.push_back(Simulator::Now().GetSeconds());

  //NS_LOG_UNCOND ("Received : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
}

static void
TxEnd (Ptr<const Packet> p)
{
 //NS_LOG_UNCOND ("Sent : "<< p->GetSize() << " Bytes at " << Simulator::Now ().GetSeconds () <<"s" );
  Received.push_back(Received.back() + p->GetSize());
  theTime.push_back(Simulator::Now().GetSeconds());
 
}

int main (int argc, char *argv[])
{
  std::ofstream otherfile;
  otherfile.open ("sim2.txt");


//for(double distx = -2.5 ; distx < 2.5 ; distx+=0.1){
//for(double disty = -2.5 ; disty < 2.5 ; disty+=0.1){

Ptr<Node> Ap = CreateObject<Node>();
Ptr<Node> RouterAp = CreateObject<Node>();
Ptr<Node> relay1 = CreateObject<Node>();
Ptr<Node> Mt1 = CreateObject<Node>();
Ptr<Node> relay2 = CreateObject<Node>();
Ptr<Node> Mt2 = CreateObject<Node>();
Ptr<Node> relay3 = CreateObject<Node>();
Ptr<Node> Mt3 = CreateObject<Node>();

//NodeContainer c = NodeContainer(Ap,RouterAp,relay1,Mt1,relay2,Mt2,relay3,Mt3);
NodeContainer c = NodeContainer(Ap,RouterAp);
c.Add(relay1);
c.Add(Mt1);
c.Add(relay2);
c.Add(Mt2);
c.Add(relay3);
c.Add(Mt3);


InternetStackHelper internet;
internet.Install(c);

PointToPointHelper p2p;
p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
p2p.SetChannelAttribute("Delay", StringValue("2ms"));
NetDeviceContainer ndAp_Router = p2p.Install(Ap, RouterAp);
//VLC---------------------------------------------------------
 OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  NetDeviceContainer ndRouterAp_RelayMt1 = OOK.Install(RouterAp, relay1);
  NetDeviceContainer ndRouterAp_RelayMt2 = OOK.Install(RouterAp, relay2);
  NetDeviceContainer ndRouterAp_RelayMt3 = OOK.Install(RouterAp, relay3);

  Ptr<VlcMobilityModel> tx1 = CreateObject<VlcMobilityModel> (); //Tx1
  Ptr<VlcMobilityModel> rx1 = CreateObject<VlcMobilityModel> (); //Rx1 
  Ptr<VlcMobilityModel> tx2 = CreateObject<VlcMobilityModel> (); //Tx2
  Ptr<VlcMobilityModel> rx2 = CreateObject<VlcMobilityModel> (); //Rx2 
  Ptr<VlcMobilityModel> tx3 = CreateObject<VlcMobilityModel> (); //Tx3
  Ptr<VlcMobilityModel> rx3 = CreateObject<VlcMobilityModel> (); //Rx3 

  tx1 -> SetPosition (Vector (0.0,0.0,5.0));
  //rx1 -> SetPosition (Vector (distx,disty,0.0));
  rx1 -> SetPosition (Vector (0.0,0.0,0.0));
  tx1 ->SetAzimuth(0.0);
  rx1 ->SetAzimuth(0.0);
  tx1 ->SetElevation(0.0);
  rx1 ->SetElevation(0.0);

  tx2 -> SetPosition (Vector (1.5,1.5,5.0));
  //rx2 -> SetPosition (Vector (distx,disty,0.0));
  rx2 -> SetPosition (Vector (1.5,1.5,0.0));
  tx2 ->SetAzimuth(0.0);
  rx2 ->SetAzimuth(0.0);
  tx2 ->SetElevation(0.0);
  rx2 ->SetElevation(0.0);

  tx3 -> SetPosition (Vector (-1.5,-1.5,5.0));
  //rx3 -> SetPosition (Vector (distx,disty,0.0));
  rx3 -> SetPosition (Vector (-1.5,-1.5,0.0));
  tx3 ->SetAzimuth(0.0);
  rx3 ->SetAzimuth(0.0);
  tx3 ->SetElevation(0.0);
  rx3 ->SetElevation(0.0);

  OOKErrorModel *em2 ;
  OOKErrorModel x;
  

  OOKErrorModel *em3 ;
  OOKErrorModel y;
  em3 = &y;

  OOKErrorModel *em4 ;
  OOKErrorModel z;
  em4 = &z;

  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(48.573);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);

  //std::cout << "em2 : ***********************" << std::endl;
  /*
  em2->setNo(100e6, 1.0e-4);     //noise bandwidth 100Mbps
  em2->setRb(1.0e6);
  em2->setWavelengths(380,380);
  em2->setTemperature(5000);
  em2->setRx(VPLM.GetRxPower(tx3,rx3));
  em2->calculateBER();
  em2->getSNR();
  */
  x.setNo(100e6, 1.0);     //noise bandwidth 100Mbps
  x.setRb(1.0e6);
  x.setWavelengths(380,380);
  x.setTemperature(5000);
  x.setRx(VPLM.GetRxPower(tx3,rx3));
  x.calculateBER();
  x.getSNR();
  em2 = &x;

  //std::cout << "em3 : ***********************" << std::endl;
  /*
  em3->setNo(100e6, 1.0e-4);     //noise bandwidth 100Mbps
  em3->setRb(1.0e6);
  em3->setWavelengths(380,380);
  em3->setTemperature(5000);
  em3->setRx(VPLM.GetRxPower(tx2,rx2));
  em3->calculateBER();
  em3->getSNR();
  */
  y.setNo(100e6, 1.0);     //noise bandwidth 100Mbps
  y.setRb(1.0e6);
  y.setWavelengths(380,380);
  y.setTemperature(5000);
  y.setRx(VPLM.GetRxPower(tx2,rx2));
  y.calculateBER();
  y.getSNR();
  em3 = &y;

  //std::cout << "em4 : ***********************" << std::endl;
  /*
  em4->setNo(100e6, 1.0e-4);     //noise bandwidth 100Mbps
  em4->setRb(1.0e6);
  em4->setWavelengths(380,380);
  em4->setTemperature(5000);
  em4->setRx(VPLM.GetRxPower(tx1,rx1));
  em4->calculateBER();
  em4->getSNR();
  */
  z.setNo(100e6, 1.0);     //noise bandwidth 100Mbps
  z.setRb(1.0e6);
  z.setWavelengths(380,380);
  z.setTemperature(5000);
  z.setRx(VPLM.GetRxPower(tx1,rx1));
  z.calculateBER();
  z.getSNR();
  em4 = &z;


  ndRouterAp_RelayMt1.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
  ndRouterAp_RelayMt1.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));

  ndRouterAp_RelayMt2.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em3));
  ndRouterAp_RelayMt2.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em3));

  ndRouterAp_RelayMt3.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em4));
  ndRouterAp_RelayMt3.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em4));
  

//------------------------------------------------------------
//Wifi--------------------------------------------------------
 std::string phyMode ("DsssRate1Mbps");
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



staticRoutingAp->AddHostRouteTo(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.1.2"), 1,1);
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.2.2"), 2,1);
staticRoutingRelayMt1->AddHostRouteTo(Ipv4Address("10.1.6.2"), Ipv4Address("10.1.6.2"), 3,1);

staticRoutingAp->AddHostRouteTo(Ipv4Address("10.1.7.2"), Ipv4Address("10.1.1.2"), 1,1);
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.7.2"), Ipv4Address("10.1.3.2"), 3,1);
staticRoutingRelayMt2->AddHostRouteTo(Ipv4Address("10.1.7.2"), Ipv4Address("10.1.7.2"), 3,1);

staticRoutingAp->AddHostRouteTo(Ipv4Address("10.1.8.2"), Ipv4Address("10.1.1.2"), 1,1);
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.8.2"), Ipv4Address("10.1.4.2"), 4,1);
staticRoutingRelayMt3->AddHostRouteTo(Ipv4Address("10.1.8.2"), Ipv4Address("10.1.8.2"), 3,1);

staticRoutingMt1->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.6.1"), 1,1);
staticRoutingRelayMt1->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.5.1"), 2,1);
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.1"), 1,1);

staticRoutingMt2->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.7.1"), 1,1);
staticRoutingRelayMt2->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.5.1"), 2,1);
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.1"), 1,1);

staticRoutingMt3->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.8.1"), 1,1);
staticRoutingRelayMt3->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.5.1"), 2,1);
staticRoutingRouterAp->AddHostRouteTo(Ipv4Address("10.1.1.1"), Ipv4Address("10.1.1.1"), 1,1);

 Ptr<Socket> srcSocket1 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket2 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket3 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket4 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
 Ptr<Socket> srcSocket5 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket6 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket7 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));
  Ptr<Socket> srcSocket8 = Socket::CreateSocket (Ap, TypeId::LookupByName ("ns3::TcpSocketFactory"));

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

AsciiTraceHelper ascii;
p2p.EnableAsciiAll(ascii.CreateFileStream ("RoutingTestCase.tr"));
p2p.EnablePcapAll("RoutingTestCase");

LogComponentEnableAll(LOG_PREFIX_TIME);
LogComponentEnable("RoutingTestCase", LOG_LEVEL_INFO);

Ptr<OutputStreamWrapper> stream1 = Create<OutputStreamWrapper> ("Table3", std::ios::out);
ipv4RoutingHelper.PrintRoutingTableAllAt(Seconds(2.0), stream1);



ndRelay_Mt3.Get (1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

ndRelay_Mt3.Get (1)->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback (&TxEnd));

ndRelay_Mt1.Get (1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

ndRelay_Mt1.Get (1)->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback (&TxEnd));

Simulator::Schedule(Seconds(0.1), &StartFlow,srcSocket1, dstaddr3, dstport3);

Simulator::Schedule(Seconds(0.1), &StartFlow,srcSocket2, dstaddr1, dstport1);

Simulator::Run();
/*
double throughput = ((Received.back()*8))/ theTime.back();
std::cout<<"-------------------------"<< std::endl;
std::cout<<"Received : " << Received.back() << std::endl;
//std::cout<<"Distance : " << dist << std::endl;
std::cout<<"Time : " << theTime.back() << std::endl;
std::cout<<"THROUGHPUT : " << throughput << std::endl;
std::cout<<"BER : " << em2->getBER() << std::endl;
*/


//otherfile << distx << " " << disty << " " << x.getSNR()+y.getSNR()+z.getSNR() << std::endl;


Simulator::Destroy();


//} }

otherfile.close();
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

