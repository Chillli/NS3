#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/netanim-module.h"
#include "ns3/propagation-module.h"
#include "ns3/propagation-loss-model.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("ProjectNS3");
int main (int argc, char  *argv[])
{
	int nNodes = 1;
double StartTime = 0.0;
double StopTime = 10.0;
uint32_t payloadSize = 1472;
uint32_t maxPacket = 10000;
double edThreshold = -96.0;
double ccaThreshold = -99.0;
double powerStart = 16.0206;
double distance = 100.0;
double rg  = 0;
double tg = 0;

StringValue DataRate;
//DataRate = StringValue("DsssRate11Mbps");    //for 802.11b
DataRate = StringValue("OfdmRate12Mbps");    //for 802.11a
//DataRate = VhtWifiMacHelper::DataRateForMcs (0);     //for 802.11ac
//DataRate = HtWifiMacHelper::DataRateForMcs (0);      //for 802.11n
 
 CommandLine cmd;
 cmd.AddValue("distance", "Distance between AP and STA", distance);
 cmd.AddValue("ps", "Base power of PHY", powerStart);
 cmd.AddValue("t", "Simulation time", StopTime);
  cmd.AddValue("rg", "Antenna gain of receiver", rg);
   cmd.AddValue("tg", "Simulation time", tg);
    cmd.Parse (argc, argv);
    
	   
  //Nodes generation
  NodeContainer wifiApNode;
wifiApNode.Create(1);

std::cout<<"Access point and stations are created."<< '\n';
  
  // PHY layer setup
  NodeContainer wifiStaNodes;
wifiStaNodes.Create(nNodes);
std::cout<<"Nodes created."<<'\n';

YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
phy.Set("RxGain", DoubleValue(rg));  
phy.Set("TxGain", DoubleValue(tg));
phy.Set("TxPowerStart", DoubleValue(powerStart));
phy.Set("TxPowerEnd", DoubleValue(powerStart));
std::cout<<"power base: "<<powerStart<<" dbm; power end:"<<powerStart<<" dbm;"<<'\n';

phy.Set("EnergyDetectionThreshold", DoubleValue(edThreshold));
phy.Set("CcaMode1Threshold", DoubleValue(ccaThreshold));
std::cout<<"Energy Detection threshold: "<<edThreshold<<"dbm; Carrier Sense threshold:"<<ccaThreshold<<"dbm"<<'\n';

//Channel setup
YansWifiChannelHelper channel;
channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
channel.AddPropagationLoss("ns3::FriisPropagationLossModel");
phy.SetChannel(channel.Create());

//wifi setup
WifiHelper wifi = WifiHelper::Default();
//wifi.SetStandard( WIFI_PHY_STANDARD_80211b); //for 802.11b
//std::cout<<"802.11b standard set"<<'\n';

//wifi.SetStandard( WIFI_PHY_STANDARD_80211ac);  //for 802.11ac
//std::cout<<"802.11ac standard set"<<'\n';

//wifi.SetStandard( WIFI_PHY_STANDARD_80211n_5GHZ); //for 802.11n 5 GHz
//std::cout<<"802.11n 5 G standard set"<<'\n';

//wifi.SetStandard( WIFI_PHY_STANDARD_80211n_2_4GHZ); //for 802.11n 2.4 GHz
//std::cout<<"802.11n 2.4 G standard set"<<'\n';

wifi.SetStandard( WIFI_PHY_STANDARD_80211a);
std::cout<<"802.11b standard set"<<'\n';
wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager","DataMode",DataRate, "ControlMode", DataRate);



NqosWifiMacHelper mac = NqosWifiMacHelper::Default();   //for 802.11b and 802.11a
//VhtWifiMacHelper mac = VhtWifiMacHelper::Default ();  //for 802.11ac
 //HtWifiMacHelper mac = HtWifiMacHelper::Default ();  //for 802.11n
std::cout<<"Control rate configured"<<'\n';

//MAC setup
Ssid ssid = Ssid("ProjectNS3");
mac.SetType("ns3::StaWifiMac","Ssid",SsidValue(ssid),"ActiveProbing",BooleanValue(false));

NetDeviceContainer staDevices;
  staDevices=wifi.Install(phy,mac,wifiStaNodes);
  
  mac.SetType("ns3::ApWifiMac","Ssid",SsidValue(ssid));
  
  NetDeviceContainer apDevice;
  apDevice=wifi.Install(phy,mac,wifiApNode);
  std::cout<<"SSID, ApDevice and StaDevice set"<<'\n';
  
  //Mobility setup
MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
				"MinX", DoubleValue(0.0),
				"MinY", DoubleValue(0.0),
				"DeltaX", DoubleValue(distance),
				"DeltaY", DoubleValue(0.0),
				"GridWidth", UintegerValue(nNodes + 1),
				"LayoutType", StringValue("RowFirst"));


mobility.Install(wifiStaNodes);


mobility.Install(wifiApNode);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
std::cout<<"mobility configured to be constant position"<<'\n';

//Stack & address setup
InternetStackHelper stack;
stack.Install(wifiApNode);
stack.Install(wifiStaNodes);

Ipv4AddressHelper address;
Ipv4Address addr;
address.SetBase("10.1.1.0","255.255.255.0");
Ipv4InterfaceContainer staNodesInterface;
Ipv4InterfaceContainer apNodeInterface;
staNodesInterface = address.Assign(staDevices);
apNodeInterface = address.Assign(apDevice);

addr = apNodeInterface.GetAddress(0);
for(int i = 0; i < nNodes; i++)
{
	addr = staNodesInterface.GetAddress(i);
	std::cout << "Node" << i<< " IP Address: " << addr << std::endl;
}
std::cout<<"Internet Stack & IPv4 address configured"<<'\n';

//Server & client setup
ApplicationContainer serverApp;
UdpServerHelper myServer(400);
serverApp = myServer.Install (wifiStaNodes.Get(0));
serverApp.Start(Seconds(StartTime));
serverApp.Stop(Seconds(StopTime));

UdpClientHelper myClient(apNodeInterface.GetAddress(0),400);
myClient.SetAttribute ("MaxPackets", UintegerValue(maxPacket));
myClient.SetAttribute ("Interval", TimeValue(Time("0.002")));
myClient.SetAttribute ("PacketSize", UintegerValue(payloadSize));
ApplicationContainer clientApp;
clientApp = myClient.Install (wifiStaNodes.Get(0));
clientApp.Start(Seconds(StartTime));
clientApp.Stop (Seconds(StopTime+5));
std::cout << "UDP traffic generated.." << '\n';

// Calculate Throughput & Delay using Flowmonitor
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll();
AnimationInterface anim ("projectAnim.xml");
Simulator::Stop (Seconds(StopTime+2));
Simulator::Run();

monitor->CheckForLostPackets();
monitor->SerializeToXmlFile("projectFlow.xml", true, true);
Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

std::cout<<" Distance: " << distance<< "\n";
std::cout<<" Maximum power: " << powerStart<< "\n";
std::cout<<" Receiver antenna gain:  " <<rg << "\n";
std::cout<<" Transmitter antenna gain:  " <<tg << "\n";
for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end () ; ++i)
{
	 
	

	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	std::cout<<"Flow"<<i->first<<"("<<t.sourceAddress<<"->" << t.destinationAddress << ")\n";
	std::cout<<" Tx Bytes: " << i->second.txBytes << "\n";
	std::cout<<" Rx Bytes: " << i->second.rxBytes << "\n";
	std::cout<<" Packet Loss: " << ( i->second.txBytes - i->second.rxBytes) * 100 /  i->second.txBytes << "% \n";
	std::cout<<"  Average Throughput: " << i->second.rxBytes  * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 / nNodes << "kbps\n";
	std::cout << " Delay: " << i->second.delaySum / i->second.rxPackets << "\n";

}
Simulator::Destroy();
return 0;
}
  
