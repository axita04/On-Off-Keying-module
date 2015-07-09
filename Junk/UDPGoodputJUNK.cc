

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/on-off-keying-module-helper.h"
#include "ns3/applications-module.h"
#include "ns3/OOK-error-model.h"
#include "ns3/vlc-propagation-loss-model.h"
#include "ns3/packet-sink.h"
#include "ns3/gnuplot.h"
#include "ns3/double.h"
#include "ns3/mobility-module.h"
#include "ns3/VLC-Mobility-Model.h"
#include <vector>
#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");
//Implementation of the application that will be ran on the nodes
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
//Packet sink responsible for being able to trace the packets received at a node
Ptr<PacketSink> sink1;
std::vector<double> Received (1,0);//Keeps track of how many total bytes are received after the next packet is received
std::vector<double> theTime (1,0);//Keeps track of what times each packet is received in the simulator
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

Gnuplot plot; //used for data extracting purposes

Gnuplot2dDataset dataSet;
dataSet.SetStyle(Gnuplot2dDataset::LINES);


  for(double pks = 0.0 ; pks < 20000 ; pks+=1000){ // runs the code multiple time with different packet sizes
NodeContainer nodes; // A node container is an object that stores individual nodes and helps us access them
  nodes.Create (2);

  OOKHelper OOK; // This helper makes the VLC channel that we are going to use
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//-----------------------------------------------------------------
  NetDeviceContainer devices;// same as a node container just for net devices instead
  devices = OOK.Install (nodes);

  Ptr<VlcMobilityModel> a = CreateObject<VlcMobilityModel> ();//These vectors are what represent the nodes moving
  Ptr<VlcMobilityModel> b = CreateObject<VlcMobilityModel> ();//in space

  a -> SetPosition (Vector (0.0,0.0,5.0));
  b -> SetPosition (Vector (1.0,0.0,0.0));
  a ->SetAzimuth(0.0);
  b ->SetAzimuth(0.0);
  a ->SetElevation(0.0);
  b ->SetElevation(0.0);
  /*  //Still needs to be better implemented
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::VlcMobilityModel" , "Azimuth", DoubleValue(0.0) , "Elevation" , DoubleValue(0.0) , "Position2" , VectorValue(Vector(0.0,0.0,5.0)));

  mobility.Install(nodes.Get(0));

mobility.SetMobilityModel("ns3::VlcMobilityModel" , "Azimuth", DoubleValue(0.0) , "Elevation" , DoubleValue(0.0) , "Position2" , VectorValue(Vector(1.0,0.0,0.0)));


  mobility.Install(nodes.Get(1));
  
  Ptr<Object> object = nodes.Get(0);
  Ptr<MobilityModel> model = object->GetObject<MobilityModel>();
  std::cout << model->GetPosition() << std::endl;
*/
  OOKErrorModel *em2 ;///
  OOKErrorModel x;    //All this does is just instantiate an Error Model that we later install on the netdevice
  em2 = &x;         //

  //Sets the initial conditions of the transmitter and receiver in the VLC network
  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(48.573);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);

  //Also initial conditions, but these are made in the error model since thats where the values are used to calculate BER
  em2->setNo(380,380,5000,100e6,VPLM.GetPhotoDetectorArea());
  em2->setRx(VPLM.GetRxPower(a,b));


  

  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute("ErrorRate", DoubleValue(0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2)); // putting the error model on the netdevice

   //The following code is similar because the same error model is put on the other netdevice inorder to move the channel more towards a duplex VLC link
   OOKErrorModel *em3 ;
  OOKErrorModel y;
  em3 = &y;

  VLCPropagationLossModel VPLM2;
  VPLM2.SetTxPower(48.573);
  VPLM2.SetLambertianOrder(70);
  VPLM2.SetFilterGain(1);
  VPLM2.SetPhotoDetectorArea(1.0e-4);
  VPLM2.SetConcentratorGain(70,1.5);

  em3->setRes(0.28);
  em3->setNo(1.0e-11);
  em3->setRb(1.0e6);
  em3->setRx(VPLM.GetRxPower(a,b));

  devices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em3));

  InternetStackHelper stack; //This helper handles making all the components of the internet stack that will be layered on top on the already exsisting network
  stack.Install (nodes);

  Ipv4AddressHelper address; // Helps with assigning the correct addresses for each node
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort)); // Assigns a sink to a node using its interface address at the sinkPort
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort)); //makes it a udp sink
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  
  //*****************************************
   sink1 = DynamicCast<PacketSink>(sinkApps.Get(0)); //making the packet sink so packet events can be traced
  
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20000.));

  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ());

  //Creating the app to be installed on the nodes
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3UdpSocket, sinkAddress, pks, 1000, DataRate ("1Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20000.0));

  //This is an example of a trace source that will allow us to see when something happens, in this case, when a packet is received on the reciver side
  devices.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));


  Simulator::Stop (Seconds (20000.0));
  Simulator::Run ();
  double goodput = (Received.back()*8)/ theTime.back(); //goodput calculation
  //std::cout<< " Elevation : " << el << " BER : " << em2->getBER() << std::endl;
  
  dataSet.Add(pks, goodput); //adds these parameters to a data set that will be used for future graphs 

  Simulator::Destroy();
  }


//More data formatting stuff
std::ostringstream os;
os << "txPower" << 48.573 <<"dbm";
dataSet.SetTitle(os.str());
plot.AddDataset(dataSet);
GnuplotCollection gnuplots("OOKErrorModel-UDP.pdf");
{
gnuplots.AddPlot(plot);
}
gnuplots.GenerateOutput(std::cout);


  return 0;
}
 

