/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Md Ashiqur Rahman: University of Arizona.
 *
 **/

// simple-wifi-mobility

// NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=simple-wifi-mobility 2>&1 | tee log.txt

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-velocity-mobility-model.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("simple-wifi-mobility");

using namespace std;

namespace ns3 {

/**
 * This scenario simulates a tree topology (using topology reader module)
 *
 *                                    /--------\
 *                           +------->|  root  |<--------+
 *                           |        \--------/         |    10Mbps 100ms
 *                           |                           |
 *                           v                           v
 *                      /-------\                    /-------\
 *              +------>| rtr-4 |<-------+   +------>| rtr-5 |<--------+
 *              |       \-------/        |   |       \-------/         |
 *              |                        |   |                         |   10Mbps 50ms
 *              v                        v   v                         v
 *         /-------\                   /-------\                    /-------\
 *      +->| rtr-1 |<-+             +->| rtr-2 |<-+              +->| rtr-3 |<-+
 *      |  \-------/  |             |  \-------/  |              |  \-------/  |
 *      |             |             |             |              |             | 10Mbps 2ms
 *      v             v             v             v              v             v
 *   /------\      /------\      /------\      /------\      /------\      /------\
 *   |wifi-1|      |wifi-2|      |wifi-3|      |wifi-4|      |wifi-5|      |wifi-6|
 *   \------/      \------/      \------/      \------/      \------/      \------/
 *
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     ./waf --run=simple-wifi-mobility
 */
  
  Ptr<ConstantVelocityMobilityModel> cvmm1, cvmm2;
  double position_interval = 1.0;
  
  // two callbacks
  void printPosition() 
  {
    Vector thePos = cvmm2->GetPosition();
    Simulator::Schedule(Seconds(position_interval), &printPosition);
    std::cout << "position: " << thePos << std::endl;
  }

  void stopMover() 
  {
    cvmm2 -> SetVelocity(Vector(0,0,0));
  }

  int main (int argc, char *argv[])
  {
    std::string phyMode ("DsssRate1Mbps");
    double rss = -80; // -dBm
    uint32_t packetSize = 1000; // bytes
    uint32_t numPackets = 1;
    uint32_t nWifi = 1;
    uint32_t wifiSta = 2;
    double interval = 1.0; // seconds
    bool verbose = false;

    int bottomrow = 6;            // number of bottom-row nodes
    int spacing = 200;            // between bottom-row nodes
    //int mheight = 150;            // height of mover above bottom row
    //int brheight = 50;            // height of bottom row
    double endtime = 20.0;
    double speed = (double)(bottomrow*spacing)/endtime; 

    string animFile = "mobility-animation.xml";

    CommandLine cmd;
    cmd.AddValue ("nWifi", "Wifi Stations (Consumers)", nWifi);
    cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
    cmd.AddValue ("rss", "received signal strength", rss);
    cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
    cmd.AddValue ("numPackets", "number of packets generated", numPackets);
    cmd.AddValue ("interval", "interval (seconds) between packets", interval);
    cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
    cmd.AddValue ("animFile", "File Name for Animation Output", animFile);
    cmd.Parse (argc, argv);

    // Reading file for topology setup
    AnnotatedTopologyReader topologyReader("", 1);
    topologyReader.SetFileName("scenarios/x-topo.txt");
    topologyReader.Read();

    //Getting containers for the producer/wifi-ap
    Ptr<Node> producer = Names::Find<Node>("root");
    Ptr<Node> wifiApNodes[6] = {Names::Find<Node>("ap1"), 
                                Names::Find<Node>("ap2"),
                                Names::Find<Node>("ap3"),
                                Names::Find<Node>("ap4"),
                                Names::Find<Node>("ap5"),
                                Names::Find<Node>("ap6")};
    
    Ptr<Node> routers[5] = {Names::Find<Node>("r1"), 
                                Names::Find<Node>("r2"),
                                Names::Find<Node>("r3"),
                                Names::Find<Node>("r4"),
                                Names::Find<Node>("r5"),};

    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    // The below set of helpers will help us to put together the wifi NICs we want 
    WifiHelper wifi;
    
    if(verbose)
    {
      wifi.EnableLogComponents (); // Turn on all Wifi logging
    }

    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

    // This is one parameter that matters when using FixedRssLossModel
    // set it to zero; otherwise, gain will be added
    // wifiPhy.Set ("RxGain", DoubleValue (0) );

    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

    // The below FixedRssLossModel will cause the rss to be fixed regardless
    // of the distance between the two stations, and the transmit power
    // wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue(rss));

    // the following has an absolute cutoff at distance > 250
    wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", 
                                    "MaxRange", DoubleValue(110)); // radius
    wifiPhy.SetChannel (wifiChannel.Create ());
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue (phyMode),
                                  "ControlMode", StringValue (phyMode));

    // Setup the rest of the upper mac
    // Setting SSID
    Ssid ssid = Ssid ("wifi-default");

    // Add a non-QoS upper mac of STAs, and disable rate control
    NqosWifiMacHelper wifiMacHelper = NqosWifiMacHelper::Default ();
    wifiMacHelper.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid),
                           "ActiveProbing", BooleanValue (true),
                           "ProbeRequestTimeout", TimeValue(Seconds(0.5)));

    NodeContainer consumers;
    consumers.Create(wifiSta);

    NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMacHelper, consumers);
    NetDeviceContainer devices = staDevice;

    // setup AP.
    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
    wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid),
                     "BeaconGeneration", BooleanValue(false));
                     //"BeaconInterval", TimeValue(Seconds(2.5)));
    for (int i = 0; i < bottomrow; i++)
    {
        NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, wifiApNodes[i]);
        devices.Add (apDevice);
    }

    // Note that with FixedRssLossModel, the positions below are not
    // used for received signal strength.
    
    // set positions. 
    MobilityHelper sessile;               // for fixed nodes
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    int Xpos = 0;
    for (int i=0; i < bottomrow; i++) {
        positionAlloc->Add(Vector(100+Xpos, 0.0, 0.0));
        Xpos += spacing;
    }
    sessile.SetPositionAllocator (positionAlloc);
    //sessile.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    for (int i = 0; i < bottomrow; i++) {
        sessile.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        sessile.Install (wifiApNodes[i]);
    }

    // ConstantVelocityMobilityModel is a subclass of MobilityModel
    Vector pos1 (0, 0, 0);
    Vector vel1 (speed, 0, 0);
    Vector pos2 (-100, 0, 0); //bottomrow*spacing, opposite direction
    Vector vel2 (speed, 0, 0);
    MobilityHelper mobile; 
    mobile.SetMobilityModel("ns3::ConstantVelocityMobilityModel"); // no Attributes
    mobile.Install(consumers);
    cvmm1 = consumers.Get(0)->GetObject<ConstantVelocityMobilityModel> ();
    cvmm1->SetPosition(pos1);
    cvmm1->SetVelocity(vel1);
    cvmm2 = consumers.Get(1)->GetObject<ConstantVelocityMobilityModel> ();
    cvmm2->SetPosition(pos2);
    cvmm2->SetVelocity(vel2);
    //std::cout << "position: " << cvmm->GetPosition() << " velocity: " << cvmm->GetVelocity() << std::endl;
    //std::cout << "mover mobility model: " << mobile.GetMobilityModelType() << std::endl; // just for confirmation

    // 3. Install NDN stack on all nodes
    NS_LOG_INFO("Installing NDN stack");
    ndn::StackHelper ndnHelper;
    ndnHelper.InstallAll();
    ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize", "1000");
    //ndnHelper.SetOldContentStore("ns3::ndn::cs::Nocache");

    // Choosing forwarding strategy
    ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route");

    // Installing global routing interface on all nodes
    ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
    ndnGlobalRoutingHelper.InstallAll();

    // 4. Set up applications
    NS_LOG_INFO("Installing Applications");
    // Consumer Helpers
    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    consumerHelper.SetPrefix("/root/prefix");
    consumerHelper.SetAttribute("Frequency", DoubleValue(10.0));
    consumerHelper.Install(consumers.Get(0)).Start(Seconds(0.0));
    consumerHelper.Install(consumers.Get(1)).Start(Seconds(3.2));
    // Producer Helpers
    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
    // Register /root prefix with global routing controller and
    // install producer that will satisfy Interests in /root namespace
    ndnGlobalRoutingHelper.AddOrigins("/root", producer);
    producerHelper.SetPrefix("/root");
    producerHelper.Install(producer);

    // Calculate and install FIBs
    ndn::GlobalRoutingHelper::CalculateRoutes();

    // Tracing
    wifiPhy.EnablePcap ("simple-wifi-mobility", devices); // check this

    // uncomment the next line to verify that node 'mover' is actually moving
    Simulator::Schedule(Seconds(position_interval), &printPosition);

    Simulator::Stop (Seconds (endtime));

    AnimationInterface anim (animFile);

    ndn::L3RateTracer::Install(routers[0],"simple-wifi-mobility-trace.txt", Seconds(0.5));
    //ndn::CsTracer::Install(routers[0],"simple-wifi-mobility-trace.txt", Seconds(1.0));
    
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
  }
}

int main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
