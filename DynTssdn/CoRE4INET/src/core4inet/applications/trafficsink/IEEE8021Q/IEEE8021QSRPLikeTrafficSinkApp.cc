/*
 * IEEE8021QSRPLikeTrafficSinkApp.cc
 *
 *  Created on: Apr 23, 2022
 *      Author: root
 */


//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "IEEE8021QSRPLikeTrafficSinkApp.h"
//CoRE4INET
#include "core4inet/utilities/ConfigFunctions.h"

using namespace std;
namespace CoRE4INET {

Define_Module(IEEE8021QSRPLikeTrafficSinkApp);

void IEEE8021QSRPLikeTrafficSinkApp::initialize()
{
    BGTrafficSinkApp::initialize();
    handleParameterChange(nullptr);
    for (unsigned int i = 0; i < this->numPCP; i++)
    {
        simsignal_t signalPk = registerSignal(("rxQPcp" + std::to_string(i) + "Pk").c_str());
        cProperty* statisticTemplate = getProperties()->get("statisticTemplate", "rxQPcpPk");
        getEnvir()->addResultRecorders(this, signalPk, ("rxQPcp" + std::to_string(i) + "Pk").c_str(), statisticTemplate);
        statisticTemplate = getProperties()->get("statisticTemplate", "rxQPcpBytes");
        getEnvir()->addResultRecorders(this, signalPk, ("rxQPcp" + std::to_string(i) + "Bytes").c_str(), statisticTemplate);

        this->rxQPcpPkSignals.push_back(signalPk);
    }
    received=0;
}

void IEEE8021QSRPLikeTrafficSinkApp::handleMessage(cMessage *msg)
{
    if (EthernetIIFrameWithQTag *qframe = dynamic_cast<EthernetIIFrameWithQTag*>(msg)){
        if (address.isUnspecified() || qframe->getSrc() == address){
            int pcp = qframe->getPcp();
            emit(this->rxQPcpPkSignals[pcp], qframe);
            //save frame
            saveTTframe(qframe);
            received++;
        }
    }
    delete msg;
}

void IEEE8021QSRPLikeTrafficSinkApp::handleParameterChange(const char* parname)
{
    BGTrafficSinkApp::handleParameterChange(parname);
    if (!parname || !strcmp(parname, "numPCP"))
    {
        this->numPCP = static_cast<unsigned int>(parameterULongCheckRange(par("numPCP"), 1, std::numeric_limits<int>::max()));
    }
}


void IEEE8021QSRPLikeTrafficSinkApp::saveTTframe(EthernetIIFrameWithQTag *qframe){
    //save TT frame
    std::tuple<string,string,uint16_t> key=make_tuple(qframe->getSrc().str(),qframe->getDest().str(),qframe->getVID());
    ReceivedTTFrameLatencies[key].push_back(qframe->getArrivalTime()-qframe->getCreationTime());
}



void IEEE8021QSRPLikeTrafficSinkApp::finish() {

    double ave_E2Elatency=0;
    if(ReceivedTTFrameLatencies.size()!=0){
        for (auto n : ReceivedTTFrameLatencies){
            std::tuple<std::string,std::string,uint16_t> key = n.first;
            std::vector<simtime_t> value = n.second;

            double tmpSum=0;
            for (auto n : value)
                tmpSum+=n.dbl();

            ave_E2Elatency=tmpSum/value.size();
            cout<<"Average E2E latency for stream "<<std::get<0>(key)<<":"<<std::get<1>(key)<<":"<<std::get<2>(key)<<" => " <<ave_E2Elatency<<" sec"<<endl;
        }

        ReceivedTTFrameLatencies.clear();
    }

    //note: results can be extended depending on the purpose, this is just a simple showcase
    //solution: outputting related results to the files for further processing

}


} //namespace


