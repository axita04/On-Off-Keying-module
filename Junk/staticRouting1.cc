// taken from: http://ce.sharif.ir/~kalantari/projects/Course%20Projects/Data%20Networks/3/Lab3_Network_Layer_Fall_91.pdf

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/animation-interface.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Static_Routing");

int main (int argc, char *argv[])
{

	uint32_t PacketSize = 512; //bytes
	std::string DataRate ("1Mbps");
	uint16_t num_Nodes = 3;
	uint16_t UDPport = 9;
	bool tracing = true;

	CommandLine cmd;
	cmd.AddValue ("PacketSize", "size of application packet sent", PacketSize);
	cmd.AddValue ("DataRate", "rate of packets sent", DataRate);

	cmd.AddValue ("tracing", "turn on ascii and pcap tracing", tracing);
	cmd.Parse (argc, argv);

	Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue(PacketSize));
	Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue(DataRate));
	Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));

	ns3::PacketMetadata::Enable();
	std::string animFile = "staticRouting.xml";

	NodeContainer nodes;
	nodes.Create (num_Nodes);

	NodeContainer AB = NodeContainer (nodes.Get(0), nodes.Get(1));
	NodeContainer BC = NodeContainer (nodes.Get(1), nodes.Get(2));

	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", StringValue("1Mbps"));

	p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
	NetDeviceContainer dAB = p2p.Install (AB);
	NetDeviceContainer dBC = p2p.Install (BC);

	NS_LOG_INFO("Setting routing protocols");
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4GlobalRoutingHelper globalRouting;
	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	list.Add(globalRouting, 10);
	//Install network stacks on the nodes
	InternetStackHelper internet;
	internet.SetRoutingHelper(list);
	internet.Install(nodes);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer iAB = ipv4.Assign (dAB);
	iAB.SetMetric(0,1);
	ipv4.SetBase("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer iBC = ipv4.Assign (dBC);
	iBC.SetMetric(1,1);
	

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	PacketSinkHelper UDPsink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), UDPport));
	ApplicationContainer App;
	NodeContainer SourceNode = NodeContainer (nodes.Get(0));
	
	NodeContainer SinkNode = NodeContainer (nodes.Get(2));
	
	App = UDPsink.Install (SinkNode);
	App.Start (Seconds(0.0));
	App.Stop(Seconds(10.0));
	Address D_Address(InetSocketAddress(iAB.GetAddress(1), UDPport));
	
	OnOffHelper UDPsource ("ns3::UdpSocketFactory", D_Address);
	
	//UDPsource.SetAttribute ("OnTime", one);
	//UDPsource.SetAttribute ("OffTime", zero);
	App.Start (Seconds (1.0));
	App.Stop (Seconds (10.0));

	/*
	AnimationInterface anim (animFile);
	Ptr<Node> n = nodes.Get(0);
	anim.SetConstantPosition (n, 0, 4);
	n = nodes.Get(1);
	anim.SetConstantPosition (n, 2, 2);
	*/

	if (tracing == true)
	{
		AsciiTraceHelper ascii;
		p2p.EnableAsciiAll (ascii.CreateFileStream ("statciRout.tr"));
		p2p.EnablePcapAll ("staticRout");
	}

	Ptr<OutputStreamWrapper> stream1 = Create<OutputStreamWrapper> ("Table2", std::ios::out);
	Ipv4GlobalRoutingHelper helper2;
	helper2.PrintRoutingTableAllAt(Seconds(2.0), stream1);

	Simulator::Stop (Seconds(10.0));
	Simulator::Run();
	Simulator::Destroy();
	return 0;











}
