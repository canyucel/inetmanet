//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <omnetpp.h>
#include "UDPApp.h"

// UDPApp: generic UDP application baseclass

// No module definition required, since there's no NED definition
// Define_Module( UDPAppBase );

void UDPAppBase::initialize()
{
    strcpy(_nodename, par("nodename"));
    _local_port = par("local_port");
    _dest_port = par("dest_port");
    _msg_length = par("message_length");
    _msg_freq = par("message_freq");
    _destType = par("routeDestNo");
}


//=====================================================

Define_Module(UDPServer);

void UDPServer::handleMessage(cMessage *msg)
{
    simtime_t arrivalTime = msg->arrivalTime();
    int sent_port = msg->par("src_port");
    int rec_port = msg->par("dest_port");
    int content = msg->par("content");
    bool isRequest = msg->par("request");
    int length = msg->length();

    IPAddress src = msg->par("src_addr").stringValue();
    IPAddress dest = msg->par("dest_addr").stringValue();

    // print out Packet info
    ev  << "\n+++" << _nodename << " UDP Server: Packet received:";
    ev  << "Cont: " << content
        << (isRequest ? " Request" : " Reply")
        << " BitLength: " << length
        << " Arr. Time: " << arrivalTime
        << " Simtime: " << simTime() << endl;
    ev  << "Src/Port: " << src.getString() << " / " << sent_port << "  ";
    ev  << "Dest/Port: " << dest.getString() << " / " << rec_port << endl;

    delete (msg);

    if (!isRequest)
        return;

    // send reply, reverse src and dest
    /* problem: no out gate!!!
    cPacket *np = p->dup();
    np->addPar("request") = false;
    IPInterface *nip = new IPInterface(np, src);
    nip->setSrcAddr(dest);
    nip->setProtocol(protocol);
    send(nip, "to_udp");
    */
}


//===============================================


Define_Module(UDPClient);

void UDPClient::initialize()
{
    UDPAppBase::initialize();
    contCtr++;
}

void UDPClient::activity()
{
    int contCtr = intrand(100);

    wait(1);

    while(true)
    {
        cMessage *msg = new cMessage();
        wait(truncnormal(_msg_freq, _msg_freq * 0.1));

        msg->setLength(1 + intrand(_msg_length));
        msg->addPar("content") = contCtr++;
        msg->addPar("request") = true;

        IPAddress _dest_addr;
        chooseDestAddr(_dest_addr);
        msg->addPar("dest_addr").setStringValue( _dest_addr.getString() );
        msg->addPar("src_port") = _local_port;
        msg->addPar("dest_port") = _dest_port;


        ev << "\n*** " << _nodename
            << " UDP App: Packet sent:"
            << "\nContent: " << int(msg->par("content"))
            << " Bitlength: " << int(msg->length())
            << "    Simtime: " << simTime()
            << "\nSrc: " << "unknown"
            << " / " << _local_port
            << "   Dest: " << _dest_addr.getString()
            << " / " << _dest_port
            << "\n";
        send(msg, "to_udp");

    }
}


void UDPClient::chooseDestAddr(IPAddress& dest)
{
    // data based on test2.irt
    int typeCtr = 3;
    int destCtr[] = {6, 5, 12};
    char *destAddrArray[][20] = {
        // MC Network 1
        { "225.0.0.1", "230.0.0.1", "230.0.1.0", "230.0.1.1", "10.0.1.3", "127.0.0.1"},
        // TCP/UDP complete netowrk
        { "10.0.0.1", "10.0.0.2", "10.0.0.3", "10.0.0.4", "127.0.0.1"},
        // MC Network 2
        { "172.0.0.1", "172.0.0.2", "172.0.0.3", "172.0.1.1", "172.0.2.1", "172.0.2.2",
          "225.0.0.1", "225.0.0.2", "225.0.0.3", "225.0.1.1", "225.0.1.2", "225.0.2.1"}
    };

    if (_destType >= typeCtr)
        opp_error("wrong routeDestNo value %d", _destType);

    dest = destAddrArray[_destType][intrand(destCtr[_destType])];
}
