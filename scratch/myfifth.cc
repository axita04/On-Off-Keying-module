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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
//
// We want to look at changes in the ns-3 TCP congestion window.  We need
// to crank up a flow and hook the CongestionWindow attribute on the socket
// of the sender.  Normally one would use an on-off application to generate a
// flow, but this has a couple of problems.  First, the socket of the on-off 
// application is not created until Application Start time, so we wouldn't be 
// able to hook the socket (now) at configuration time.  Second, even if we 
// could arrange a call after start time, the socket is not public so we 
// couldn't get at it.
//
// So, we can cook up a simple version of the on-off application that does what
// we want.  On the plus side we don't need all of the complexity of the on-off
// application.  On the minus side, we don't have a helper, so we have to get
// a little more involved in the details, but this is trivial.
//
// So first, we create a socket and do the trace connect on it; then we pass 
// this socket into the constructor of our simple application which we then 
// install in the source node.
// ===========================================================================
//
class MyApp : public Application 
{
public:
//Declaration for a new custom App as an example
  MyApp ();
  virtual ~MyApp();
//Method for setting up the new Application
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
//Methods for starting and Stopping the application
  virtual void StartApplication (void);
  virtual void StopApplication (void);
//Methods for Sechuleing a Tramission on the stack and sending packets
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
//Basic Constructor
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
//Sets up the App with all of the different parameters
void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}
//Start the Application and connect to where it need to transmit and call Send Packet
void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}
//Closes the socket and cancels everything
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
//Cal scheduleTx for the number of packets to send
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
//If the App is runing it Has the Simulator sechdule a packet transmission
void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}
/*
static void
CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ());
}

static void
RxDrop (Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}
*/
int 
main (int argc, char *argv[])
{


//Loops throught the simulation moving the receiver in the y direction. Goes from 0 to 5 meters increaing my .05m
for(double currentD = 0.0; currentD < 5.0 ; currentD += .05){
//Creates the 2 nodes plus their container
NodeContainer nodes;
  nodes.Create (2);
//Makes a helper and set the Data rate and Delay
  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//Make Net Device Container for the devices and install them to the nodes
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);
//Makes two Constant Mobility Model
  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();  
//Set where the two models are positioned based on currentD
  a -> SetPosition (Vector (0.0,0.0,0.0));
  b -> SetPosition (Vector (0.0,currentD,0.0));
  AErrorModel *em2 ; //Make a new AerrorModel pointer
  AErrorModel x;     // Makes a new AerrorModel object
  em2 = &x;          //Points the pointer to that object
  
  VLCPropagationLossModel VPLM;  //Makes a new VLCPropagationLossModel object
  VPLM.SetTxPower(48.573);       //Sets Tx power to 48.573 dbm
  VPLM.SetLambertianOrder(70);   //Sets LambertainOrder semi angle to 70
  VPLM.SetFilterGain(1);         //Sets Filter gain to 1
  VPLM.SetPhotoDetectorArea(1.0e-4);//Sets the PhotoDectoreArea to 1.0e-4(m^2)
  VPLM.SetConcentratorGain(70,1.5);//Sets variable to Concentrator can with angle 70 and constant 1.5

  em2->setRes(0.28);   //Sets the Resposcitvity in the Error Model to 0.28
  em2->setNo(1.0e-14);  //Sets Noise power to 1.0e-14
  em2->setRb(1.0e6);    //Sets Bit rate to 1.0e6
  em2->setRx(VPLM.GetRxPower(a,b)); //Sets the Received power to the Propagation Model's calcuation

  std::cout<<"------------------------"<< std::endl;
   std::cout<<"BER : " <<x.getBER()<<std::endl;
  std::cout<<"Radiance Angle : " <<VPLM.GetRadianceAngle(a,b)<<std::endl;
  std::cout<<"Distance : " <<VPLM.GetDistance(a,b)<< " m" <<std::endl;
  //std::cout<<"Energy per Bit : " <<x.getEb()<<std::endl;
  std::cout<<"RxPower : " << VPLM.GetRxPower(a,b)<<std::endl;
  std::cout<<"SNR : " << std::pow(x.getSNR(),2)<<std::endl;

  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute("ErrorRate", DoubleValue(0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2)); //Sets the Devices Error model to AerroModel
//Sets up the Stack, installs the Stacks to nodes, Sets up the Addressing scheme and assigns it to the net devices
  InternetStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
//Sets up a New sink used to catch Packets and tracks different infromations
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));
//Creates a New TCP Socket and puts in on the FIrst node
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
//Makes a new App object and sets it up to use the socket, the sink, 
 //sent packets 10 packets 1040 bytes long at a Data rate of 1Mbps
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1040, 10, DataRate ("1Mbps"));
  //Puts the App on the socket and then run the simulation
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.));

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy();
  
}

  Simulator::Destroy ();

  return 0;
}
