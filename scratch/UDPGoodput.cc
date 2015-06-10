

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
//Class for new custom app to use in this script
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

//**************************************
//Makes a Packet sink
Ptr<PacketSink> sink1;
//Sets up a vector of data Received call Received and every time data is Received in theTime
std::vector<double> Received (1,0);
std::vector<double> theTime (1,0);

//Trace Method that every time Rx recieves something and the if total Data does not equal the current total
//It pushes the current time and current data total on the back of the vectors
static void
RxEnd (Ptr<const Packet> p)
{
  if(Received.back() != sink1->GetTotalRx()){
    Received.push_back(sink1->GetTotalRx());
    theTime.push_back(Simulator::Now().GetSeconds());
  }
}


//Main method
int 
main (int argc, char *argv[])
{
//Sets up a new plot
Gnuplot plot;
//sets up dataSet 
Gnuplot2dDataset dataSet;
dataSet.SetStyle(Gnuplot2dDataset::LINES);

//Loops Elevation from 0 degress to 360 incrementing by 10
  for(double el = 0.0 ; el < 360.0 ; el+=10){
//Creates a node container with 2 nodes
  NodeContainer nodes;
  nodes.Create (2);
// Makes a new OOKhelper with a Data rate of 5mbps and delay of 2ms
  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//-----------------------------------------------------------------
//Makes a Net Device contain and installs two new net devies to the nodes and put them in the container
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);
//Makes 2 Vlc Mobility Model A is for the Transmitter B is for the Receiver
  Ptr<VlcMobilityModel> a = CreateObject<VlcMobilityModel> ();
  Ptr<VlcMobilityModel> b = CreateObject<VlcMobilityModel> ();  
//Sets the Transimeter to 5.0m on the Z
  a -> SetPosition (Vector (0.0,0.0,5.0));
//Sets the Receiver to 1m on the X
  b -> SetPosition (Vector (1.0,0.0,0.0));
  //Sets the Azimuth to 0.0 for both 
  a ->SetAzimuth(0.0);
  b ->SetAzimuth(0.0);
  //Sets Elevation for the Transmitter to el and For Receiver at 0.0 
  //If both were at 0.0 they would be point to eachother
  a ->SetElevation(el);
  b ->SetElevation(0.0);
 //Makes a new Aerror model and pointer and pointer the pointer to the model
  AErrorModel *em2 ;
  AErrorModel x;
  em2 = &x;
//Makes a new VLCPropagaionLossModel and Sets Txpower, Semiangle in LambertianOrder, Filter gain, 
//PhotoDectorArea and Concentrator Gain 
  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(48.573);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);
//Sets the Responcsitivity Noise power, Bit Rate, and Received power from the VPLM
  em2->setRes(0.28);
  em2->setNo(1.0e-11);
  em2->setRb(1.0e6);
  em2->setRx(VPLM.GetRxPower(a,b));

  
  //Sets the devices Error model to the AerrorModel
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
 //Makes a new InternetStack and isntalls the nodew to it
  InternetStackHelper stack;
  stack.Install (nodes);
//Sets up a new address Base  and sets up a new container with the devices
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
//Makes a new SinkAddress and port, and Packet Sink Helper and sinkApps
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  
  //*****************************************
  // Makes the sink into a Packets Sink
   sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
  
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20000.));
//Makes a new Socket on the First Node
  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ());

//Sets up a new App with the Socket, the PacketSink address, packet size of 1040 bytes , 1 packet , and DataRate
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3UdpSocket, sinkAddress, 1040, 1, DataRate ("1Mbps"));
  //Adds the application to the node
  nodes.Get (0)->AddApplication (app);
  
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20000.0));
//Makes a Trace on the net device when the Receiver receives something.
  devices.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));


  Simulator::Stop (Seconds (20000.0));
  Simulator::Run ();
 
 //Adds the current elevation angle and ln of the BER to the data set
  dataSet.Add(el,std::log(em2->getBER()));

  Simulator::Destroy();
  }


//Graphs the new data set to the plot
std::ostringstream os;
os << "txPower" << 48.573 <<"dbm";
dataSet.SetTitle(os.str());
plot.AddDataset(dataSet);
GnuplotCollection gnuplots("AErrorModel-UDP.pdf");
{
gnuplots.AddPlot(plot);
}
//outputs the data set
gnuplots.GenerateOutput(std::cout);


  return 0;
}
 

