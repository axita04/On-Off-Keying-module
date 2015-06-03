

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/on-off-keying-module-helper.h"
#include "ns3/applications-module.h"
#include "ns3/Aerror-model.h"
#include "ns3/vlc-propagation-loss-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/packet-sink.h"
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");
//Creates a new Custom App
class MyApp : public Application 
{
public:
//Header of the new App Contains Constructor, Deconstructor, Setup, Start/Stop Application 
  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
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

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

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

void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

//**************************************

Ptr<PacketSink> sink1; //Put on Rx so as to get info on when packets are received
std::vector<double> Received (1,0); //Keeps track on how much data is received
std::vector<double> theTime (1,0); //Keeps track of on what time each packet is received
static void
RxEnd (Ptr<const Packet> p)
{
  if(Received.back() != sink1->GetTotalRx()){ //checks that no duplicate packets are counted as data
    Received.push_back(sink1->GetTotalRx());
    theTime.push_back(Simulator::Now().GetSeconds());
  }

 // NS_LOG_UNCOND ("Rx Received at " << Simulator::Now().GetSeconds());
}



int 
main (int argc, char *argv[])
{


for(double d = 0.0 ; d < 10.0 ; d+=0.05){  //loops the simulation for different distances
NodeContainer nodes;
  nodes.Create (2);

  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//-----------------------------------------------------------------
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);

  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();  

  a -> SetPosition (Vector (0.0,0.0,0.0));
  b -> SetPosition (Vector (0.0,d,0.0)); //this node will move in the y direction 
  AErrorModel *em2 ;
  AErrorModel x;
  em2 = &x;

  VLCPropagationLossModel VPLM;       //*
  VPLM.SetTxPower(48.573);            //*
  VPLM.SetLambertianOrder(70);        //*        This block is responsible for setting up the variables in the propagation equation
  VPLM.SetFilterGain(1);              //*
  VPLM.SetPhotoDetectorArea(1.0e-4);  //*
  VPLM.SetConcentratorGain(70,1.5);   //

  em2->setRes(0.28);             //*
  em2->setNo(1.0e-11);           //* This block sets up the variables for the Error model
  em2->setRb(1.0e6);             //*
  em2->setRx(VPLM.GetRxPower(a,b));

  

  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute("ErrorRate", DoubleValue(0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2)); // layers on the error model

  InternetStackHelper stack; 
  stack.Install (nodes); //installs the internet stack onto each node

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1)); //on Rx Node
  
  //*****************************************
   sink1 = DynamicCast<PacketSink>(sinkApps.Get(0)); //puts sink on Rx Node
  
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20000.)); //giving each simulation ample time to execute

  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ()); //creating socket and putting it on the first node


  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3UdpSocket, sinkAddress, 1040, 1000, DataRate ("1Mbps")); //* sets up the application that sends 1000 packets each of size 1040bytes at 1Mbps
  nodes.Get (0)->AddApplication (app); // installs on Tx Node
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20000.0));

  devices.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd)); //needed in order to know when each packet is sent


  Simulator::Stop (Seconds (20000.0));
  Simulator::Run ();
  double goodput = (Received.back()*8)/ theTime.back(); //the amount of data over the time it took to transmit that data or in this case throughput at the application layer
  std::cout << d << " " << goodput << std::endl;
  

  Simulator::Destroy();
  
}
  


  return 0;
}
 

