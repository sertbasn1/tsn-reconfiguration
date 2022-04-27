
#ifndef ABSTRACTCONTROLLERAPP_H_
#define ABSTRACTCONTROLLERAPP_H_

#include <omnetpp.h>
#include <list>
#include "openflow/openflow/controller/OF_Controller.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "openflow/openflow/protocol/OpenFlow.h"
#include "openflow/messages/Open_Flow_Message_m.h"
#include "openflow/messages/OFP_Packet_In_m.h"
#include "openflow/messages/OFP_Packet_Out_m.h"
#include "openflow/messages/OFP_Flow_Mod_m.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"

namespace openflow{

struct CommonHeaderFields{
    uint32_t buffer_id;
    Switch_Info * swInfo;
    uint32_t inport;
    MACAddress src_mac;
    MACAddress dst_mac;
    int eth_type;
    IPv4Address arp_src_adr;
    IPv4Address arp_dst_adr;
    int arp_op;
};

class AbstractControllerApp:public cSimpleModule, public cListener {

protected:
    simsignal_t PacketInSignalId;
    simsignal_t PacketOutSignalId;
    simsignal_t PacketFeatureRequestSignalId;
    simsignal_t PacketFeatureReplySignalId;
    simsignal_t PacketExperimenterSignalId;
    simsignal_t BootedSignalId;
    simsignal_t TopologySignalId;

    long packetsFlooded;
    long packetsDropped;
    long numPacketOut;
    long numFlowMod;
    int priority;

    OF_Controller * controller;


    virtual void initialize() override;
    virtual void handleParameterChange(const char *parname) override;
    virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) override;
    virtual void finish() override;

    virtual OFP_Packet_Out * createFloodPacketFromPacketIn(OFP_Packet_In *packet_in_msg);
    virtual OFP_Packet_Out * createDropPacketFromPacketIn(OFP_Packet_In *packet_in_msg);
    virtual OFP_Packet_Out * createPacketOutFromPacketIn(OFP_Packet_In *packet_in_msg, uint32_t outport);
    virtual OFP_Flow_Mod * createFlowMod(ofp_flow_mod_command mod_com,const oxm_basic_match &match, uint32_t outport,int idleTimeOut, int hardTimeOut);
    virtual OFP_Flow_Mod * createFlowMod(ofp_flow_mod_command mod_com,const oxm_basic_match  &match, uint32_t outport, int priority, int idleTimeOut, int hardTimeOut);

    virtual CommonHeaderFields extractCommonHeaderFields(OFP_Packet_In *packet_in_msg);

    virtual void floodPacket(OFP_Packet_In *packet_in_msg);
    virtual void dropPacket(OFP_Packet_In *packet_in_msg);
    virtual void sendPacket(OFP_Packet_In *packet_in_msg, uint32_t outport);
    virtual void sendFlowModMessage(ofp_flow_mod_command mod_com,const oxm_basic_match &match, uint32_t outport, TCPSocket *socket,int idleTimeOut, int hardTimeOut);


public:
    AbstractControllerApp();
    ~AbstractControllerApp();

};

} /*end namespace openflow*/

#endif
