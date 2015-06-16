#include "ns3/core-module.h"
//#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/packet-sink.h"

using namespace ns3;

class MyApp : public Application 
{
public:
//Constructor and Deconstructer header
  MyApp ();
  virtual ~MyApp();
//Header fo setup method that will set up the application with the Socket, Address, packetSize, nPackets, dataRate
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
//Methods for starting and stopping the app
  virtual void StartApplication (void);
  virtual void StopApplication (void);
//Method to have the simulator schedule a transmission  
  void ScheduleTx (void);
//Method to have a packet made and call schdedule Tx
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}
//Sets up the App with the socket , address, packetSize, nPackets, dataRate
void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}
//Start Application by binding the socket, connecting it to the peer address and sending a packet
void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}
//Stop application by cancelling all send events, closing the socket, setting it to not running
void 
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}
//Sends the packet to be send to the socket and schedules nPackets number of transmissions
void 
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}
//Has the Simulator Schedule a Transmission, at tNext time.
void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}


NS_LOG_COMPONENT_DEFINE ("Wifi-test");

int
main (int argc, char *argv[])
{

  NodeContainer wifiNodes;
  wifiNodes.Create(2);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiNodes);

  MobilityHelper mobility;

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiNodes);

  InternetStackHelper stack;
  stack.Install(wifiNodes);

  Ipv4AddressHelper address;
  
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  
   Ipv4InterfaceContainer interfaces = address.Assign (staDevices);
//Makes a new SinkAddress and port, and Packet Sink Helper and sinkApps
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (wifiNodes.Get (1));
  
  
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));
//Makes a new Socket on the First Node
  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (wifiNodes.Get (0), UdpSocketFactory::GetTypeId ());

//Sets up a new App with the Socket, the PacketSink address, packet size of 1040 bytes , 1 packet , and DataRate
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3UdpSocket, sinkAddress, 1040, 10, DataRate ("1Mbps"));
  //Adds the application to the node
  wifiNodes.Get (0)->AddApplication (app);
  
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.0));


  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();

  Simulator::Destroy();

  LogComponentEnable("app", LOG_LEVEL_INFO);
}


  
  
