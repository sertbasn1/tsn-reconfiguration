/*
 * Edge_OF_RelayUnit.cc
 *
 *  Created on: Feb 12, 2021
 *      Author: root
 */

#include "core4inet/linklayer/ethernet/avb/SRPFrame_m.h"
#include "core4inet/linklayer/contract/ExtendedIeee802Ctrl_m.h"
#include "inet/linklayer/ethernet/EtherMACBase.h"
//openflow
#include "openflow/messages/OFP_Packet_In_m.h"
#include "openflow/messages/OFP_Packet_Out_m.h"
#include "sdn4core/switch/tsn/engine/TSN_OF_RelayUnit.h"
#include "core4inet/base/avb/AVBDefs.h"

#include <sdn4core/switch/ieee8021qbv/edgeRelay/Edge_OF_RelayUnit.h>
#include <sdn4core/switch/base/engine/OF_RelayUnit.h>
#include <sstream>
#include <string>

//inet
#include "inet/linklayer/ethernet/EtherMAC.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/ITransportPacket.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/ICMPMessage.h"

#include "core4inet/linklayer/ethernet/base/EtherFrameWithQTag_m.h"

using namespace std;
using namespace inet;
using namespace CoRE4INET;
using namespace openflow;

namespace SDN4CoRE{

Define_Module(Edge_OF_RelayUnit);

void Edge_OF_RelayUnit::initialize(int stage) {

    OF_RelayUnit::initialize(stage);
    forwardSRPtoConSig = registerSignal("forwardSRPtoConSig");
    isTaggingEnabled = par("isTaggingEnabled").boolValue();
}

void Edge_OF_RelayUnit::processDataPlanePacket(cMessage *msg){
    bool msgHandled = false;
    IInterfaceTable *inet_ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    MACAddress mac = inet_ift->getInterface(0)->getMacAddress();

    if(msg->arrivedOn("dataPlaneIn")){
        if (inet::EthernetIIFrame *frame = dynamic_cast<inet::EthernetIIFrame*>(msg)){
            if (CoRE4INET::SRPFrame * srpFrame = dynamic_cast<CoRE4INET::SRPFrame *>(frame->decapsulate())) {
                if (CoRE4INET::TalkerAdvertise* talkerAdvertise = dynamic_cast<CoRE4INET::TalkerAdvertise*>(srpFrame)) {
                    StreamPortMap[talkerAdvertise->getStreamID()]= frame->getArrivalGate()->getIndex();

                    std::tuple<std::string,std::string,uint16_t> key= std::make_tuple(frame->getSrc().str(),frame->getDest().str(),talkerAdvertise->getVlan_identifier());
                    StreamInfoMap[key]=talkerAdvertise->getStreamID();

                    inet::Ieee802Ctrl * controlInfo = new inet::Ieee802Ctrl();
                    controlInfo->setSwitchPort(frame->getArrivalGate()->getIndex());
                    controlInfo->setSourceAddress(frame->getSrc()); //this can be used instead of src address field in  TA msg

                    CoRE4INET::SRPFrame* toController = dynamic_cast<CoRE4INET::SRPFrame *>(talkerAdvertise);
                    toController->setControlInfo(controlInfo);
                    emit(forwardSRPtoConSig,toController->dup());
                    cout<<"TA forwarded by  "<<this->getFullPath()<<"to the controller "<<std::endl;
                    delete toController;
                    msgHandled = true;
                }
                //delete msg;
            }
            else
            {  //tag packet with the rule version
                if (EthernetIIFrameWithQTag* qframe = dynamic_cast< EthernetIIFrameWithQTag*>(frame)){
                   if(qframe->getmatchVersion()==0){         //if I am the ingress point for this frame, rules are already uploaded
                        cModule *targetModule = getModuleByPath("getnet.open_flow_controller1.controllerApps[1]");
                        ConfigurationEngine * module= check_and_cast<ConfigurationEngine *>(targetModule);
                        std::tuple <std::string,std::string,uint16_t> key= std::make_tuple(qframe->getSrc().str(),qframe->getDest().str(), qframe->getVID());
                        uint64_t sid = StreamInfoMap[key];
                        uint16_t match_version = module->getMatchVersionbyStreamId(sid);
                        //TODO: this should be handled differently, for now switches + controller shares StreamFlowVersionTable table
                        qframe->setmatchVersion(match_version);
                        //cout<<"[Edge_OF_RelayUnit] t:"<<simTime()<<"at "<<mac<<" Stream "<<sid<<"->frame between "<<frame->getSrc()<<":"<<frame->getDest()<<" service type: "<<qframe->getVID()<<" is tagged with "<<match_version<<endl;
                   }

                    OF_RelayUnit::processFrame(frame);
                    msgHandled = true;

                }
              //
            }

        }

    }



}



void Edge_OF_RelayUnit::handleSRPMsgFromController(CoRE4INET::SRPFrame * srpFrame ) {
    Enter_Method("handleSRPMsgFromController");
    //has been triggered by the Edge_OF_Agent
            if (CoRE4INET::ListenerReady* listenerReady = dynamic_cast<CoRE4INET::ListenerReady*>(srpFrame)) {
                std::cout<<"--->"<<this->getFullPath()<<listenerReady->getFullName()<<" msg "<<endl;
                //sent to the realted talker
                uint64_t sid = listenerReady->getStreamID();
                inet::EthernetIIFrame * frame = new inet::EthernetIIFrame(listenerReady->getName());
                frame->setEtherType(MSRP_ETHERTYPE);
                frame->setDest(CoRE4INET::SRP_ADDRESS);
                frame->encapsulate(listenerReady->dup());

                send(frame->dup(), "dataPlaneOut",  StreamPortMap[sid]);
                delete frame;
                delete listenerReady;
            }
}

Edge_OF_RelayUnit::~Edge_OF_RelayUnit(){
    StreamPortMap.clear();
    StreamInfoMap.clear();
}


} /*end namespace SDN4CoRE*/
