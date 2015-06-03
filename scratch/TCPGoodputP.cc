

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
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");
//Class for new App
class MyApp : public Application 
{
public:
//Methods heads for Constructor, Deconstructor, Setup, StartApplication, StopApplication, ScheduleTx and Send packet
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
//Basic Constructor that sets some of the fields 
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
//Deconstructor set the sockets to 0
MyApp::~MyApp()
{
  m_socket = 0;
}
//Sets up the App with the socket, Sink Address, Packet Size, Number of Packets, and Data Rate
void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}
//Starts Application
void
MyApp::StartApplication (void)
{
  m_running = true;  //Sets the Application to Running
  m_packetsSent = 0; // Set the number of packets sent to 0
  m_socket->Bind (); //Binds the Socket
  m_socket->Connect (m_peer); //Connects the Socket to
  SendPacket (); //Sends the Packets
}
//Stops the simulation, cancels all still qeued events and closes the socket
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
//calls SechdueTx and makes a packet for it to send
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
//If the app is running it has the simulator schedule an event
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
//Makes A packet sink and 2 vectors 
Ptr<PacketSink> sink1;
std::vector<double> Received (1,0); //Vector of all data values evertime a packet is received. 
                                    //Last entry is all data recieced by the application
std::vector<double> theTime (1,0); // Vecotr of all time stamps at when the application received data
static void
RxEnd (Ptr<const Packet> p)   //Trace Method 
{
  if(Received.back() != sink1->GetTotalRx()){   //If the last value on the vecotr does not equal the current total of RX
    Received.push_back(sink1->GetTotalRx());    //Then the current total of RX is pushed to the backer
    theTime.push_back(Simulator::Now().GetSeconds());//And the time is pushed on theTime
  }

 // NS_LOG_UNCOND ("Rx Received at " << Simulator::Now().GetSeconds());
}



int 
main (int argc, char *argv[])
{
//Makes a GNUPLOT
Gnuplot plot;
//Makes a data set for the Plot and sets the style of the plot
Gnuplot2dDataset dataSet;
dataSet.SetStyle(Gnuplot2dDataset::LINES);

//Loops through the simulation going from a packet of size 1 byte to 65000 bytes(MaxSizeofCongestionWindw) 
//Incrementing by 512 bytes
for(double P = 1 ; P < 65000; P+=512){  
//Creates a Node container with 2 nodes and a OOK helper that sets DataRate and Delay
NodeContainer nodes; 
nodes.Create (2);
OOKHelper OOK;
OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//-----------------------------------------------------------------
//Creates a Netdevice container and Install new Netdevices to the nodes and puts them in devices  
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);
//Creates two Constant Position Mobility Models and sets them at a distance of 5.0m in the y direction
  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();  
  a -> SetPosition (Vector (0.0,0.0,0.0));
  b -> SetPosition (Vector (0.0,5.0,0.0));
  //Creates a new Error and pointer to the error model
  AErrorModel *em2 ;
  AErrorModel x;
  em2 = &x;
 // Makes a new Propagation Loss Model and sets Txpower, Semiangle in LambertianOrder, Filtergain, PhotoDetecor Acrea,
 //FOV and Refractive index for the Concentrator gain
  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(48.573);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);
//Sets the Responcitivity, Nosie Power, and Data Rate, 
//It then Get the RX power from the propagaion model and sets it in the ErrorModel
  em2->setRes(0.28);
  em2->setNo(1.0e-11);
  em2->setRb(1.0e6);
  em2->setRx(VPLM.GetRxPower(a,b));

  
//Attaches the Error model to the Receivers Net Device
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
//Creates an InternetStack helper and installs the nodes to it
  InternetStackHelper stack;
  stack.Install (nodes);
//Creates the Ipv4 dress helper and assignt addresses to the net devices
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
//Makes a new Sink and install a sink applicaion on the reciever
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  
  //*****************************************
  //Sets sink1 to the Applicaion that was installed on the reciever
   sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
  //Sets start and stop times for the app
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20000.));
//Makes a pointer to a new TCP socket
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());

//Makes a new app from the custom on delcared at the begining of the file, sets it up, and adds it to the nodes
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, P, 1000, DataRate ("1Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20000.0));
//Sets up a Trace on the netdevice that Call RxEnd every time the Netdevice recieves some data
  devices.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

//Runs the Simulation
  Simulator::Stop (Seconds (20000.0));
  Simulator::Run ();
//Calculates avg Goodput from the simulaion and pints out the PacketSize and Goodput
  double goodput = (Received.back()*8)/ theTime.back();
  std::cout << P << " " << goodput << std::endl;
  //Adds the Packets size and good put to the dataset
  dataSet.Add(P,goodput);
 //Destories the Simulation
  Simulator::Destroy();
  
} 

//Adds the dataSet to the Plot and te Plot to gnuplots
std::ostringstream os;
os << "txPower" << 48.573 <<"dbm";
dataSet.SetTitle(os.str());
plot.AddDataset(dataSet);
GnuplotCollection gnuplots("AErrorModel-TCP.pdf");
{
gnuplots.AddPlot(plot);
}
gnuplots.GenerateOutput(std::cout);


  return 0;
}
 

