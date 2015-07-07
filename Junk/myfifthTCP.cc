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
#include "ns3/OOK-error-model.h"
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
//Constructor for Custom App
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
//Deconstructor
MyApp::~MyApp()
{
  m_socket = 0;
}
//Sets up the Application will all of the different values in the parameters
void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}
//Start the App and connects to the other node
void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}
//Stop application, cancels all events, and closes the socket
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
//SchedulsTx each packet needed to be sent
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
//Schedules a SendPacket in the simulator
void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}
//count values for different events
int countSent = 0;
int countReceived = 0;
int countDropped = 0;
//Keeps a count of the number of Packets Dropped
static void
RxDrop (Ptr<const Packet> p)
{
  countDropped  ++;
  //NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}

//Keeps a count of the number of Packets Received by net device
static void
RxEnd (Ptr<const Packet> p)
{
  countReceived ++;
 // NS_LOG_UNCOND ("Rx Received at " << Simulator::Now().GetSeconds());
}

//Keeps a count of the number of packets transmitted by device
static void
TxEnd (Ptr<const Packet> p)
{
  countSent ++;
 // NS_LOG_UNCOND ("Tx Sent at " << Simulator::Now().GetSeconds());
}


int 
main (int argc, char *argv[])
{




//Create Node container, the nodes, and the Helper with the appropreate values
NodeContainer nodes;
  nodes.Create (2);
  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("2ms"));
//Sets up the Net devices with the Helper
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);
//Creates two constant mobility model
  Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();  
//Set the distance between them
  a -> SetPosition (Vector (0.0,0.0,0.0));
  b -> SetPosition (Vector (0.0,5.0,0.0));
  //Creates a pointer to the OOKerrorModel
  OOKErrorModel *em2 ;
  OOKErrorModel x;
  em2 = &x;
  //Sets up a new Propagation Model with all of values set
  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(48.573);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);
//Set all the values for the Error model and get the Received power from the Propagation model
  em2->setRes(0.28);
  em2->setNo(1.0e-11);
  em2->setRb(1.0e6);
  em2->setRx(VPLM.GetRxPower(a,b));

  

  //Attachs the Error model to the net device
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em2));
 //Makes a new Internet Stack and installs it to the nodes
  InternetStackHelper stack;
  stack.Install (nodes);
 // Makes a new Address helper and assign addresses to the devices
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  //Makes the Sink and installs it on the receiver
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));
//Makes a new socket
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());

//Creates the App with the Socket, Sink, 1000 packets to send at 1040 bytes long at a data rate of 1Mbps
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1040, 1000, DataRate ("1Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (20.));

//Traces the PhyTx End(sent packets) , PHYRxEnd (Received packets), and PhyRxDrop (Dropped packets) from the net devices

    devices.Get (0) -> TraceConnectWithoutContext ("PhyTxEnd", MakeCallback(&TxEnd));

  devices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

  devices.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

//Runs and destorys the simulation
  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy();


  std::cout<<"------------------------"<< std::endl;
   std::cout<<"BER : " <<x.getBER()<<std::endl;
  std::cout<<"Distance : " <<VPLM.GetDistance(a,b)<< " m" <<std::endl;
  std::cout<<"RxPower : " << VPLM.GetRxPower(a,b)<<std::endl;
  std::cout<<"SNR : " << x.getSNR()<<std::endl;  

  std::cout << "Packets Sent : " << countSent << std::endl;
  std::cout << "Packets Received : " << countReceived << std::endl;
  std::cout << "Packets Dropped : " << countDropped << std::endl;

  return 0;
}
