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
#include <vector>
#include <cmath>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/on-off-keying-module-helper.h"
#include "ns3/applications-module.h"
#include "ns3/Aerror-model.h"
#include "ns3/vlc-propagation-loss-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/Illumination.h"
#include "ns3/VLC-Mobility-Model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 UDP    |    |    ns-3 UDP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                1 Mbps, 1 ms
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

int countSent = 0;
int countReceived = 0;
int countDropped = 0;


static void
RxDrop (Ptr<const Packet> p)
{
  countDropped  ++;
  //NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
  //std::cout << "RxDrop at " << Simulator::Now ().GetSeconds () << std::endl;
}
static void
RxEnd (Ptr<const Packet> p)
{
  countReceived ++;
 // NS_LOG_UNCOND ("Rx Received at " << Simulator::Now().GetSeconds());
  //std::cout << "Rx Received at " << Simulator::Now().GetSeconds() << std::endl;
}

static void
TxEnd (Ptr<const Packet> p)
{
  countSent ++;
 // NS_LOG_UNCOND ("Tx Sent at " << Simulator::Now().GetSeconds());
  //std::cout << "Packet of size: " << p->GetSize() << " sent at " << Simulator::Now().GetSeconds() << std::endl;

}


//Standard Luminosity Function from 380 - 770 nm (visible light spectrum)
double V_lambda[] = { 0.000039, 0.000120, 0.000396, 0.001210, 0.004000, 0.011600, 0.023000, 0.038000, 0.060000, 0.090980, 0.139020, 0.208020, 0.323000,  0.503000, 0.710000, 0.862000, 0.954000, 0.994950,  0.995000, 0.952000, 0.870000, 0.757000, 0.631000, 0.503000, 0.381000, 0.265000, 0.175000, 0.107000, 0.061000, 0.032000, 0.017000, 0.008210, 0.004102, 0.002091, 0.001047, 0.000520, 0.000249, 0.000120, 0.000060, 0.000030 };

//Response of a silicon photo diode from 380 - 770 nm (visible light spectrum)
double Response[] = { 
0.150, 0.160, 0.170, 0.190, 0.200, 0.220, 0.230, 0.240, 0.250, 0.260, 
0.270, 0.280, 0.300, 0.320, 0.330, 0.350, 0.360, 0.370, 0.375, 0.380, 
0.390, 0.400, 0.415, 0.420, 0.430, 0.440, 0.450, 0.460, 0.470, 0.475,
0.480, 0.485, 0.490, 0.495, 0.500, 0.505, 0.510, 0.520, 0.526, 0.532 };

//Spectral Radiance (Planck's Law)
double SpectralRadiance( int wavelength, double temperature){
        double spectral_rad;
        double h = 6.62606957e-34; //Planck's constant
        double c = 299792458;      //speed of light
        double k = 1.3806488e-23;  //Boltzmann constant
        double waveLength = wavelength * 1e-9; //nm
        return spectral_rad = 15*((std::pow((h*c)/(M_PI*k*temperature), 4)))/((std::pow(waveLength, 5)) * ((std::exp((h*c)/(waveLength*k*temperature)))-1));
};

//Definite integral of the Luminosity Function(wavelength)*Spectral Radiance(wavelength, temperature) d(wavelength)
double integralLum(int wavelength_lower, int wavelength_upper, double T){
        double integral = 0;
        
        while(wavelength_lower <= wavelength_upper)
        {
                integral += V_lambda[(wavelength_lower-380)/10] * SpectralRadiance(wavelength_lower, T) * 10e-9;
                wavelength_lower += 10;
        }
        
        return integral;
};

//Definite integral of the Spectral Radiance(wavelength, temperature) d(wavelength)
double integralPlanck(int wavelength_lower, int wavelength_upper, double T){
        double integral = 0;
        
        while(wavelength_lower <= wavelength_upper)
        {
                integral += SpectralRadiance(wavelength_lower, T) * 10e-9;
                wavelength_lower += 10;
        }
        
        return integral;
};

//Definite integral of the Response(wavelength)*Spectral Radiance(wavelength, temperature) d(wavelength)
double integralRes(int wavelength_lower, int wavelength_upper, double T){
        double integral = 0;
        
        while(wavelength_lower <= wavelength_upper)
        {
                integral += Response[(wavelength_lower-380)/10] * SpectralRadiance(wavelength_lower, T) * 10e-9;
                wavelength_lower += 10;
        }
        
        return integral;
};

int 
main (int argc, char *argv[])
{

  double luminous_efficacy, responsivity;
  int wave_lower = 380;       //wavelength lower bound in nm
  int wave_upper = 380;       //wavelength upper bound in nm
  double T = 5000;      //Black-body temperature in Kelvin
  
       
for (int count = 1; count <= 39; count++){
NodeContainer nodes;
  nodes.Create (2);

  OOKHelper OOK;
  OOK.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  OOK.SetChannelAttribute ("Delay", StringValue ("1ms"));
//-----------------------------------------------------------------

  std:: cout << "Wavelength from " << wave_lower << " to " << wave_upper << " and Temperature @: " << T << std::endl;

  
  //Lumious efficacy calculation
  luminous_efficacy = 683 * integralLum(wave_lower, wave_upper, T);
  //truncated, to emitt light only in given ranges
  luminous_efficacy = luminous_efficacy/integralPlanck(wave_lower, wave_upper, T);
  //Responsivity calculation
  responsivity = integralRes(wave_lower, wave_upper, T);
  //truncated, to emitt light only in given ranges
   responsivity = responsivity/integralPlanck(wave_lower, wave_upper, T);

  std::cout << "Responsivity: " << responsivity << std::endl;
  std::cout << "Luminous efficacy: " << luminous_efficacy << std::endl;

//-----------------------------------------------------------------
  NetDeviceContainer devices;
  devices = OOK.Install (nodes);

  Ptr<VlcMobilityModel> a = CreateObject<VlcMobilityModel> ();
  Ptr<VlcMobilityModel> b = CreateObject<VlcMobilityModel> ();  

  a -> SetPosition (Vector (0.0,0.0,5.0));  //transmitter  (x,y,z)
  b -> SetPosition (Vector (0.0,0.0,0.0));  //receiver (x,y,z)

  a -> SetAzimuth (0 * M_PI / 180);
  a -> SetElevation(180 * M_PI / 180);   //transmitter points straight down at 180 degree elevation
  b -> SetAzimuth (0 * M_PI / 180);      // azimuth 0 means that it points at the positive x direction
  b -> SetElevation(0 * M_PI / 180);     //receiver points straight up at 0 degree elevation

  AErrorModel *em2 ;
  AErrorModel x;
  em2 = &x;

  VLCPropagationLossModel VPLM;
  VPLM.SetTxPower(72.0);
  VPLM.SetLambertianOrder(70);
  VPLM.SetFilterGain(1);
  VPLM.SetPhotoDetectorArea(1.0e-4);
  VPLM.SetConcentratorGain(70,1.5);

  em2->setRes(responsivity);
  em2->setNo(1.0e-11);
  em2->setRb(1.0e6);
  em2->setRx(VPLM.GetRxPower(a,b));

  Illumination illum;
  illum.setEfficacy(luminous_efficacy);
  illum.setPower(72.0);
  illum.setLambertianOrder(70.0);

  

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
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (40.));

  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ());

  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3UdpSocket, sinkAddress, 1040, 2000, DataRate ("1Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (0.));
  app->SetStopTime (Seconds (40.));

    devices.Get (0) -> TraceConnectWithoutContext ("PhyTxEnd", MakeCallback(&TxEnd));

  devices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

  devices.Get(1)->TraceConnectWithoutContext ("PhyRxEnd", MakeCallback (&RxEnd));

  Simulator::Stop (Seconds (40));
  Simulator::Run ();

  std::cout<<std::endl;
  std::cout<<"BER : " <<x.getBER()<<std::endl;
  std::cout<<"Distance : " <<VPLM.GetDistance(a,b)<< " m" <<std::endl;
  std::cout<<"RxPower : " << VPLM.GetRxPower(a,b)<<std::endl;
  std::cout<<"SNR : " << x.getSNR()<<std::endl; 
  std::cout<<"Illuminance : " << illum.calculateIlluminance(a, b) << " lux" <<std::endl;   

  std::cout << "Packets Sent : " << countSent << std::endl;
  std::cout << "Packets Received : " << countReceived << std::endl;
  std::cout << "Packets Dropped : " << countDropped << std::endl;

  std::cout<<"*******************************************"<< std::endl;
  std::cout<<std::endl;

  Simulator::Destroy();

  countSent = 0;
  countReceived = 0;
  countDropped = 0;

  wave_upper += 10;
  //wave_upper = wave_lower;

}


  return 0;
}
