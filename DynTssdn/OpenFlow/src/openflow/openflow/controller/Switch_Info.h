
#ifndef SWITCH_INFO_H_
#define SWITCH_INFO_H_

#include "openflow/openflow/protocol/OpenFlow.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"

using namespace __gnu_cxx;
using namespace inet;

namespace openflow{

class Switch_Info {
    public:
        Switch_Info();

        int getConnId() const;
        void setConnId(int connId);
        int getVersion() const;
        void setVersion(int version);
        std::string getMacAddress() const;
        void setMacAddress(std::string macAddress);
        int getNumOfPorts() const;
        void setNumOfPorts(int numOfPorts);
        TCPSocket* getSocket() const;
        void setSocket(TCPSocket* socket);

        int getTopo_index() const;
        void setTopo_index(int ti);
        bool IsEdge=false;
        std::vector<int> connectedHostIndexes; //topo index



    protected:
        int connID;
        int numOfPorts;
        std::string macAddress;
        TCPSocket *socket;
        int version;

        int topoindex;

};

} /*end namespace openflow*/

#endif /* FLOW_TABLE_H_ */
