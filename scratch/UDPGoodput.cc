

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
#include "ns3/gnuplot.h"
#include "ns3/VLC-Mobility-Model.h"
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");

class MyApp : public Application 
{
public:

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

Ptr<PacketSink> sink1;
std::vector<double> Received (1,0);
std::vector<double> theTime (1,0);
static void
RxEnd (Ptr<const Packet> p)
{
  if(Received.back() != sink1->GetTotalRx()){
    Received.push_back(sink1->GetTotalRx());
    theTime.push_back(Simulator::Now().GetSeconds());
  }

 // NS_LOG_UNCOND ("Rx Received at " << Simulator::Now().GetSeconds());
}



int 
main (int argc, char *argv[])
{

Gnuplot plot;

Gnuplot2dDataset dataSet;
dataSet.SetStyle(Gnuplot2dDataset::LINES);


  for(double el = 0.0 ; el < 360.0 ; el+=10){
NodeContainer nodes;
  nodes.Create (2);

  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//-----------------------------------------------------------------
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);

  Ptr<VlcMobilityModel> a = CreateObject<VlcMobilityModel> ();
  Ptr<VlcMobilityModel> b = CreateObject<VlcMobilityModel> ();  

  a -> SetPosition (Vector (0.0,0.0,5.0));
  b -> SetPosition (Vector (1.0,0.0,0.0));
  a ->SetAzimuth(0.0);
  b ->SetAzimuth(0.0);
  a ->SetElevation(el);
  b ->SetElevation(0.0);

  AErrorModel *em2 ;
  AErrorModel x;
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
  em2->setRx(VPLM.GetRxPower(a,b));

  

  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute("ErrorRate", DoubleValue(0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  
  //*****************************************
   sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
  
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20000.));

  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ());


  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3UdpSocket, sinkAddress, 1040, 1, DataRate ("1Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20000.0));

  devices.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));


  Simulator::Stop (Seconds (20000.0));
  Simulator::Run ();
 // double goodput = (Received.back()*8)/ theTime.back();
  //std::cout<< " Elevation : " << el << " BER : " << em2->getBER() << std::endl;
  
  dataSet.Add(el,std::log(em2->getBER()));

  Simulator::Destroy();
  }



std::ostringstream os;
os << "txPower" << 48.573 <<"dbm";
dataSet.SetTitle(os.str());
plot.AddDataset(dataSet);
GnuplotCollection gnuplots("AErrorModel-UDP.pdf");
{
gnuplots.AddPlot(plot);
}
gnuplots.GenerateOutput(std::cout);


  return 0;
}
 

