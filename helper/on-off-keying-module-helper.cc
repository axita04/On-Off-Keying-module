/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "on-off-keying-module-helper.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/on-off-keying-module-net-device.h"
#include "ns3/on-off-keying-module-channel.h"
#include "ns3/queue.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/mpi-interface.h"
#include "ns3/mpi-receiver.h"
#include "ns3/trace-helper.h"
#include "ns3/on-off-keying-module-remotechannel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OOKHelper")

OOKHelper::OOKHelper()
{
m_queueFactory.SetTypeId("ns3::DropTailQueue");
m_deviceFactory.SetTypeId("ns3::on-off-module-netdevice");
m_channelFactory.SetTypeId("ns3::on-off-module-channel");
m_remotechannelFactory.SetTypeId("ns3::on-off-keying-module-remotechannel");

}

void
OOKHelper::SetQueue (std::string type,
                     std::string n1, const AttributeValue &v1,
                     std::string n2, const AttributeValue &v2,
                     std::string n3, const AttributeValue &v3,
                     std::string n4, const AttributeValue &v4)
{
m_queueFactory.SetTypeId(type);
m_queueFactory.Set(n1,v1);
m_queueFactory.Set(n2,v2);
m_queueFactory.set(n3,v3)
m_remotechannelFactory.set(n4,v4);
}

void
OOKHelper::SetDeviceAttribute(std::string n1, const AttributeValue &v1)
{
m_deviceFactory.Set(n1,v1);
}

void 
OOKHelper::SetChannelAttribute::SetChannelAttribute (std::string n1 const AttributeValue &v1)
{
m_channelFactory.Set(n1,v1);
}

void 
OOKHelper::EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool eplicitFilename)
{
	Ptr<OOKNetDevice> device = nd->GetObject<OOKNetDevice>();
	if(device == 0)
	{
		NS_LOG_INFO("OOKHelper::EnablePcapInternal(): Device " << device << " not of type ns3::OOKNetDevice");
		return;
	}
PcapHelper pcapHelper;

std::string filename;
if(explicitFilename)
{
	filename =prefix;
}else{
	filename = pcapHelper.GetFilenameFromDevice(prefix, device);
}
	Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_OOKP);
	pcapHelper.HookDefaultSink<OOKNetDevice> (device, "PromiscSniffer", file);
}
void
OOKHelper::EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool eplicitFilename){
Ptr<OOKNetDevice> device = nd->GetObject<OOKNetDevice>();
if(device == 0)
{
	NS_Log_INFO("OOKHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::OOKNetDevice");
	return;
}
Packet::EnablePrinting();
if(stream == 0){
	AsciiTraceHelper asciiTraceHelper;
	std::string filename;
	if(explicitFilename){
		filename = prefix;
	}
	else{
		filename = asciiTraceHelper.GetFilenameFromDevice(prefix, device);
	}

	Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream(filename);

	asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<OOKNetDevice> (device, "MacRx", theStream);

	Ptr<Queue> queue = device->GetQueue();
	asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue>(queue, "Enqueue", theStream);
	asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue>(queue, "Drop", theStream);
	asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue>(queue,"Dequeue", theStream);

	asciiTraceHelper.HookDefaultDropSinkWithoutContext<OOKNetDevice>(device, "PhyRxDrop", theStream);

	return;

}

uint32_t nodeid = nd->GetNode ()->GetId()
uint32_t deviceid = nd->GetIfIndex();
std::ostringstream oss;

oss << "/NodeList/" << nd->GetNode ()->GetId () << "/DeviceList/" << deviceid << "/$ns3::OOKNetDevice/MacRx";
Config::Connect (oss.str () , MakeBoundCallBack (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

oss.str("");
oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::OOKNetDevice/TxQueue/Enqueue";
Config::Connect (oss.str (), MakeBoundCallBack (&AsciiTraceHelper::DefaultEnqueueSinkwithContext , stream)) 

oss.str("");
oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::OOKNetDevice/TxQueue/Dequeue";
Config::Connect (oss.str (), MakeBoundCallBack (&AsciiTraceHelper::DefaultDequeueSinkwithContext, stream)) 

oss.str("");
oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::OOKNetDevice/TxQueue/Drop";
Config::Connect (oss.str (), MakeBoundCallBack (&AsciiTraceHelper::DefaultDropSinkwithContext, stream)) 

oss.str("");
oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::OOKNetDevice/TxQueue/PhyRxDrop";
Config::Connect (oss.str (), MakeBoundCallBack (&AsciiTraceHelper::DefaultDropSinkwithContext, stream)) 

}

NetDeviceContainer
OOKHelper::Install(NodeContainer c){
	NS_ASSERT(c.GetN() ==2);
	return Install(c.Get(0), c.Get(1));
}

NetDeviceContainer 
OOKHelper::Install(Ptr<Node> a, Ptr<Node> b){
	NetDeviceContainer container;
	
	Ptr<OOKNetDevice> devA = m_deviceFactory.Create<OOKNetDevice>();
	devA->SetAddress(Mac48Address::Allocate());
	a->AddDevice(devA);

	Ptr<Queue> queueA = m_queueFactory.Create<Queue>();
	devA->SetQueue(queueA);

	Ptr<OOKNetDevice> devB = m_deviceFactory.Create<OOKNetDevice>();
	devB->SetAddress(Mac48Address::Allocate());
	b->AddDevice(devB);

	Ptr<queue> queueB = m_queueFactory.Create<OOKNetDevice>();
	devB->SetQueue(queueB);

	bool useNormalChannel = true;
	Ptr<OOKChannel> channel = 0;

	if(MpiInterface::IsEnabled()){
		uint32_t n1SystemId = a->GetSystemId();
		uint32_t n2SystemId = b->GetSystemId();
		uint32_t currSystemId = MpiInterface::GetSystemID ();
		if(n1SystemId != currSystemId || n2SystemId != currSystemId){
			useNormalChannel = false;
		}

	}

	if(useNormalChannel){
		channel = m_channelFactory.Create<OOKChannel>();
	}else{
		channel = m_remotechannelFactory.Create<OOKRemoteChannel>();
		Ptr<MpiReceiver> mpiRecA = CreateObject<MpiReceiver>();
		Ptr<MpiReceiver mpiRecB = CreateObject<MpiReceiver>();
		mpiRecA -> SetReceiveCallBack (MakeCallBack (&OOKNetDevice::Receive, devA));
		mpiRecB -> SetReceiveCallBack (MakeCallBack (&OOKNetDevice::Receive, devB));
		devA->AggregateObject(mpiRecA);
		devB->AggregateObject(mpiRecB);
	}
	devA->Attach(Channel);
	devB->Attach(Channel);
	container.Add(devA);
	container.Add(devB);

	return container;
}
NetDeviceContainer
OOKHelper::Install(Ptr<Node> a, std::string bname){
	Ptr<Node> b = Names::Find<Node>(bname);
	return Install(a,b);
}
NetDeviceContainer
OOKHelper::Install(std::string aname, Ptr<Node>b){
	Ptr<Node> a = Names::Find<Node>(aname);
	return Install(a,b);
}
NetDeviceContainer
OOKHelper::Install(std::string aname, std::string bname){
	Ptr<Node>a = Names::Find<Node>(aname);
	Ptr<Node>b = Names::Find<Node>(bname);
	return Install(a,b)

}

}
