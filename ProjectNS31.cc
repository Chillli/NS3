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
double StopTime = 20.0;
uint32_t payloadSize = 1472;
uint32_t maxPacket = 10000;
double edThreshold = -96.0;
double ccaThreshold = -99.0;
double powerStart = 20;
double powerEnd = 20;
double distance = 50.0;
double speed = 50.0;
uint32_t powerLevel = 1;
StringValue DataRate;
DataRate = StringValue("DsssRate11Mbps");
 CommandLine cmd;
 cmd.AddValue("distance", "Distance between AP and STA", distance);
  cmd.AddValue("ps", "Base power of PHY", powerStart);
 cmd.AddValue("pe", "Maximum power of PHY", powerEnd);
 cmd.AddValue("pl", "Amount of power levels", powerLevel);
 cmd.AddValue("s", "STA moving speed", speed);
 cmd.AddValue("t", "Simulation time", StopTime);
    cmd.Parse (argc, argv);
    
        if(powerStart > powerEnd){
		  std::cout<<"ERROR: pe must be larger than ps"<<'\n';
           return 0;
            }
            
        if(powerLevel > 5 || powerLevel <= 0){
		  std::cout<<"ERROR: pl shouble be positive integer less than 6"<<'\n';
           return 0;
            }
            
     if(powerStart == powerEnd && powerLevel != 1){
		  std::cout<<"ERROR: pe equals to ps. Only 1 powerlevel is permitted"<<'\n';
           return 0;
            }
            
      if(powerStart > 20){
		  std::cout<<"ERROR: When TxGain and RxGain is 0. The maximum power level is 100mW (20dBm)."<<'\n';
           return 0;
	   }
	  
  //Nodes generation
  NodeContainer wifiApNode;
wifiApNode.Create(1);

std::cout<<"Access point and stations are created."<< '\n';
  
  // PHY layer setup
  NodeContainer wifiStaNodes;
wifiStaNodes.Create(nNodes);
std::cout<<"Nodes created."<<'\n';

YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
phy.Set("RxGain", DoubleValue(0));  
phy.Set("TxGain", DoubleValue(0));
phy.Set("TxPowerStart", DoubleValue(powerStart));
phy.Set("TxPowerEnd", DoubleValue(powerEnd));
phy.Set("TxPowerLevels", UintegerValue(powerLevel));
std::cout<<"power base: "<<powerStart<<" dbm; power end:"<<powerEnd<<" dbm; number of levels:"<<powerLevel<<'\n';

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
wifi.SetStandard( WIFI_PHY_STANDARD_80211b);
std::cout<<"802.11b standard set"<<'\n';

wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager","RtsCtsThreshold", UintegerValue(0), "DataMode",DataRate, "ControlMode", DataRate, "DefaultTxPowerLevel", UintegerValue(0));
NqosWifiMacHelper mac = NqosWifiMacHelper::Default();
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
Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
  positionAlloc ->Add(Vector(0, 0, 0)); // node0
  positionAlloc ->Add(Vector(distance, 0, 0)); // node1
  mobility.SetPositionAllocator(positionAlloc);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install(wifiApNode);
mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
mobility.Install(wifiStaNodes);
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
std::cout<<" Power base: " << powerStart<< "\n";
std::cout<<" Maximum power: " << powerEnd<< "\n";
std::cout<<" Power levels: " << powerLevel<< "\n";
std::cout<<" Finished location " <<(wifiStaNodes.Get(0) -> GetObject<ConstantVelocityMobilityModel>()) -> GetPosition()<< "\n";
std::cout<<" STA speed: " << speed<< "\n";
for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end () ; ++i)
{
	 
	

	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	std::cout<<"Flow"<<i->first<<"("<<t.sourceAddress<<"->" << t.destinationAddress << ")\n";
	std::cout<<" Tx Bytes: " << i->second.txBytes << "\n";
	std::cout<<" Rx Bytes: " << i->second.rxBytes << "\n";
	std::cout<<"  Average Throughput: " << i->second.rxBytes  * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 / nNodes << "kbps\n";
	std::cout << " Delay: " << i->second.delaySum / i->second.rxPackets << "\n";

}
Simulator::Destroy();
return 0;
}
  
