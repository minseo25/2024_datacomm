#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/pointer.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/on-off-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-net-device.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RRM");

int 
main (int argc, char *argv[])
{  
  /* simulation seed, do not change the seed*/
  SeedManager::SetSeed (42);
  SeedManager::SetRun (42);
  RngSeedManager::SetSeed (42);
  /*******************************************/

  const uint32_t nAps = 4;

  // configure
  uint32_t channelNumbers = 1111; // available 1 5 9 ,,, 1 1 1 1 == 5 5 5 5 == 9 9 9 9
  double pw = 20;
  uint32_t scenario = 1;
  uint32_t simTime = 1; 
  
  CommandLine cmd;
  cmd.AddValue ("simTime", "Simulation time (seconds)", simTime);
  cmd.AddValue ("ch", "Channel number of each APs", channelNumbers);
  cmd.AddValue ("pw", "Tx power of APs", pw);
  cmd.AddValue ("scen", "1 for problem 1 and 2, and 2 for problem 3 and 4", scenario);
  cmd.Parse (argc, argv);

  std::string drate = "70Mb/s";
  NS_LOG_UNCOND("Scenario number: " << scenario);
  NS_LOG_UNCOND("Transmission power (dBm): " << pw);

  uint32_t channels[nAps];
  for(uint32_t i = 0; i < nAps; i++){
    channels[nAps-(i+1)] = channelNumbers % 10;
    channelNumbers /= 10;
  }

  uint32_t nStas = 0;
  uint32_t scen1[nAps] = {1, 1, 1, 1};
  uint32_t scen2[nAps] = {3, 2, 1, 1};
  uint32_t* assocStas;
  
  if(scenario == 1){
    assocStas = scen1;
    nStas = 4;
  }
  else if(scenario == 2){
    assocStas = scen2;
    nStas = 7;
  }
  else{
    NS_LOG_UNCOND("invalid scenario number");
    return 1;
  }

  uint32_t payloadSize = 700;

  NodeContainer aps;
  NodeContainer stas;
  NodeContainer staVec[nAps]; // for convenient bss control

  aps.Create(nAps);
  stas.Create(nStas);  

  uint32_t staIdx = 0; 
  for(uint32_t i = 0; i < nAps; i++){
    for(uint32_t j = 0; j < assocStas[i]; j++){
      staVec[i].Add(stas.Get(staIdx++)); 
    } 
  }  

  // position
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  Vector apPos[nAps] = {Vector(20, 20, 3),
                        Vector(50, 20, 3),
                        Vector(20, 50, 3),
                        Vector(50, 50, 3)
                        };

  
  Vector staPos[nAps][3];
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  for(uint32_t i = 0; i < nAps; i++){
    for(uint32_t j = 0; j < staVec[i].GetN(); j++){
        staPos[i][j] = Vector(apPos[i].x+x->GetValue(-7, 7), apPos[i].y+x->GetValue(-7, 7), 1.5);
      }
  }                      

  for(uint32_t i = 0; i < nAps; i++){
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (apPos[i]);        
    for(uint32_t j = 0; j < staVec[i].GetN(); j++){
      positionAlloc->Add (staPos[i][j]);
    }    
    mobility.SetPositionAllocator (positionAlloc);
    mobility.Install(aps.Get(i));
    mobility.Install(staVec[i]);           
  }

  // Yans channel
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");

  // path loss model
  channel.AddPropagationLoss ("ns3::ThreeLogDistancePropagationLossModel",
                              "Exponent0", DoubleValue(2.0),
                              "Exponent1", DoubleValue(3.5),
                              "Exponent2", DoubleValue(3.5),
                              "Distance0", DoubleValue(1.0),
                              "Distance1", DoubleValue(10.0),
                              "Distance2", DoubleValue(100.0),
                              "ReferenceLoss", DoubleValue(40.05));
  
  WifiMacHelper mac;
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

  InternetStackHelper internet;
  internet.Install (stas);
  internet.Install (aps);

  NetDeviceContainer apDevices;
  NetDeviceContainer staDevices;

  for (uint32_t i = 0; i < nAps; ++i){    
    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::IdealWifiManager");    
    wifi.SetStandard (WIFI_STANDARD_80211ax);

    std::string ssidString ("wifi-infra");
    std::stringstream ss;
    ss << i;
    ssidString += ss.str ();
    Ssid ssid = Ssid (ssidString);
 
    phy.Set ("ChannelSettings", StringValue ("{" + std::to_string(channels[i]) + ", 20, BAND_2_4GHZ, 0}"));
    phy.Set("TxPowerStart", DoubleValue(pw));
    phy.Set("TxPowerEnd", DoubleValue(pw));
    std::cout << "AP " << i << " channel number: " << channels[i] << std::endl;
    
    // AP setting
    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));    
    apDevices.Add(wifi.Install (phy, mac, aps.Get(i)));

    // STA setting
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));  
    for(uint32_t j = 0; j < staVec[i].GetN(); j++){
        staDevices.Add(wifi.Install (phy, mac, staVec[i].Get(j)));
    }
  }
  
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("10.0.0.0", "255.255.255.0");
  ipAddrs.Assign (apDevices);
  ipAddrs.Assign (staDevices);

  // Downlink traffic
  ApplicationContainer staApps; 
  ApplicationContainer dlApps;

  uint16_t port = 9;
  UdpServerHelper server (port);
  for(uint32_t i = 0; i < nAps; i++){
    for (uint32_t j = 0; j < staVec[i].GetN(); ++j){
      staApps.Add(server.Install (staVec[i].Get(j)));
    }
  }
  staApps.Start (Seconds (1));
  staApps.Stop (Seconds (simTime + 1));

  for (uint32_t i = 0; i < nAps; ++i){
    for (uint32_t j = 0; j < staVec[i].GetN(); ++j){
      Ipv4Address remoteAddr = staVec[i].Get(j)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (remoteAddr, port));
      onoff.SetConstantRate (DataRate (drate), payloadSize);
      dlApps.Add(onoff.Install ( aps.Get(i)));
      }
  }
  dlApps.Start (Seconds (1));
  dlApps.Stop (Seconds (simTime+ 1));
   
  NS_LOG_UNCOND ("Run Simulation.");
  Simulator::Stop (Seconds (simTime + 1)); /////
  Simulator::Run ();
  Simulator::Destroy ();
  
  // results
  double total_thr = 0;
  staIdx = 0;
  for (uint32_t i = 0; i < nAps; ++i){
    double throughput = 0;
    for (uint32_t j = 0; j < staVec[i].GetN(); ++j) {
      // UDP tracing
      uint64_t totalPacketsThrough = DynamicCast<UdpServer> (staApps.Get (staIdx++))->GetReceived (); 
      throughput += totalPacketsThrough * payloadSize * 8 / (simTime * 1000000.0); //Mbit/s
    }       
    NS_LOG_UNCOND("AP " << i << "'s throughput: "<< throughput << " Mbit/s");
    total_thr += throughput;
  }
  NS_LOG_UNCOND("Average_throughput: " << total_thr / nAps << " Mbit/s");

  return 0;
}
