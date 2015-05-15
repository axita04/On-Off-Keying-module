
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef ON_OFF_KEYING_MODULE_HELPER_H
#define ON_OFF_KEYING_MODULE_HELPER_H

#include <string>
//#include "ns3/on-off-keying-module.h"
#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"

#include "ns3/trace-helper.h"

namespace ns3 {

class Queue;
class NetDevice;
class Node;

class OOKHelper : public PcapHelperForDevice, 
                  public AsciiTraceHelperForDevice
{

public:
	OOKHelper();
	virtual ~OOKHelper(){}


	void SetQueue(std::string type, std::string n1= "", const AttributeValue &v1 = EmptyAttributeValue(),
					std::string n2 = "" ,const AttributeValue &v2 = EmptyAttributeValue(),
					std::string n3 = "" ,const AttributeValue &v3 = EmptyAttributeValue(),
					std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue());

	void SetDeviceAttribute(std::string name, const AttributeValue &value);

	void SetChannelAttribute(std::string name, const AttributeValue &value);

	NetDeviceContainer Install(NodeContainer c);

	NetDeviceContainer Install(Ptr<Node> a, Ptr<Node> b);

	NetDeviceContainer Install(std::string aname, Ptr<Node> b);

	NetDeviceContainer Install(Ptr<Node> a, std::string bname);

	NetDeviceContainer Install(std::string aname, std::string bname);

private:
	virtual void EnablePcapInternal(std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilname);

	virtual void EnableAsciiInternal(Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool explicitFilename);

	ObjectFactory m_queueFactory;
	ObjectFactory m_channelFactory;
	ObjectFactory m_remoteChannelFactory;
	ObjectFactory m_deviceFactory;
};

}

#endif /* ON_OFF_KEYING_MODULE_HELPER_H */
