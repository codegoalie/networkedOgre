#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"

#include "RakNetTypes.h"
#include "BitStream.h"

#include <string>
#include <iostream>
#include <vector>

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

enum GameMessages
{
	SPAWN_POSITION=ID_USER_PACKET_ENUM+1,
	POSITION_UPDATE=ID_USER_PACKET_ENUM+2,
	YOUR_TURN=ID_USER_PACKET_ENUM+3
};

int main(void)
{
	RakPeerInterface *peer = RakNetworkFactory::GetRakPeerInterface();
  Packet *packet;

  peer->Startup(MAX_CLIENTS, 30, &SocketDescriptor(SERVER_PORT,0), 1);

  printf("Starting the server.\n");
  // We need to let the server accept incoming connections from the clients
  peer->SetMaximumIncomingConnections(MAX_CLIENTS);

  RakNet::BitStream bsOut;
  RakNet::RakString rs;

  std::vector<RakNetGUID> clients;
  int current_client = 0;

  int counter = 0;

  while (1)
  {
    for (packet=peer->Receive(); 
         packet; 
         peer->DeallocatePacket(packet), packet=peer->Receive())
    {
      RakNet::BitStream bsIn(packet->data,packet->length,false);
      bsIn.IgnoreBytes(sizeof(MessageID));
      bsIn.Read(rs);

      std::cout << "Client List:\n";
      for(int i=0; i < (int)clients.size(); ++i)
      {
        std::cout << i+1 << " - " << clients[i].g << std::endl;
      }
      std::cout << "\n\nNew Packet from:" 
        << packet->guid.g << std::endl;

      switch (packet->data[0])
      {
        case ID_REMOTE_DISCONNECTION_NOTIFICATION:
          printf("Another client has disconnected.\n");
          break;
        case ID_REMOTE_CONNECTION_LOST:
          printf("Another client has lost the connection.\n");
          break;
        case ID_REMOTE_NEW_INCOMING_CONNECTION:
          printf("Another client has connected.\n");
          break;
        case ID_NEW_INCOMING_CONNECTION:
          printf("A connection is incoming.\n");
          clients.push_back(packet->guid);
          // Use a BitStream to write a custom user message
          // Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
          bsOut.Write((MessageID)SPAWN_POSITION);
          bsOut.Write((int)clients.size());
          peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
          bsOut.Reset();
          sleep(3);
          bsOut.Write((MessageID)YOUR_TURN);
          peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
          break;
        case ID_DISCONNECTION_NOTIFICATION:
          printf("A client has disconnected.\n");
          break;
        case ID_CONNECTION_LOST:
          printf("A client lost the connection.\n");
          break;
        case POSITION_UPDATE:
          // received new position from client
          printf("Client sent incr flag, incrementing counter\n");
          ++counter;
          std::cout << "Sending new counter value to each client\n";
          bsOut.Reset();
          bsOut.Write((MessageID)POSITION_UPDATE);
          bsOut.Write(counter);
          for(int i=0; i < (int)clients.size(); ++i)
          {
            std::cout << "  To: " << i+1 << " - " << clients[i].g << std::endl;
            peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,
               peer->GetSystemAddressFromGuid(clients[i]),false);
          }
          sleep(3);
          bsOut.Reset();
          bsOut.Write((MessageID)YOUR_TURN);
          packet->systemAddress = 
            peer->GetSystemAddressFromGuid(clients[current_client]);
          if(current_client+1 >= (int)clients.size())
            current_client = 0;
          else
            ++current_client;
          peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
          break;
        default:
          printf("Message with identifier %i has arrived.\n", packet->data[0]);
          break;
      }
      bsOut.Reset();
    }
  }

  RakNetworkFactory::DestroyRakPeerInterface(peer);

	return 0;
}
