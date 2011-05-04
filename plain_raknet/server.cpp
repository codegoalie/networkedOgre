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
	YOUR_TURN=ID_USER_PACKET_ENUM+3,
	NEW_CLIENT=ID_USER_PACKET_ENUM+4
};

class Client {
  public:
    int id, x, y, z;
    RakNetGUID guid;

    Client(int passed_id) :id(passed_id) { }
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

  std::vector<Client> clients;


  while (1)
  {
    for (packet=peer->Receive(); 
         packet; 
         peer->DeallocatePacket(packet), packet=peer->Receive())
    {
      RakNet::BitStream bsIn(packet->data,packet->length,false);
      bsIn.IgnoreBytes(sizeof(MessageID));
      //bsIn.Read(rs);

      std::cout << "Client List:\n";
      for(int i=0; i < (int)clients.size(); ++i)
      {
        std::cout << i << " - " << clients[i].guid.g << std::endl;
      }
      std::cout << "\n\nNew Packet from:" 
        << packet->guid.g << std::endl;

      int client_id=0;
      int x=0,y=0,z=0;
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

          client_id = (int)clients.size();

          if((int)clients.size() > 0)
          {
            // send new client notification to existing clients
            std::cout << "Sending new spawn position to each client\n";
            bsOut.Reset();
            bsOut.Write((MessageID)NEW_CLIENT);
            bsOut.Write(client_id);
            bsOut.Write(20);
            bsOut.Write(20);
            bsOut.Write(10);
            for(int i=0; i < (int)clients.size(); ++i)
            {
              std::cout << "  To: " << i << " - " << clients[i].guid.g << std::endl;
              peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,
                 peer->GetSystemAddressFromGuid(clients[i].guid),false);
            }
            bsOut.Reset();

            std::cout << "Sending each client's position to new client\n";
            for (int i = 0; i < (int)clients.size(); ++i) {
              std::cout << "sending for " << i << std::endl;
              bsOut.Reset();
              bsOut.Write((MessageID)NEW_CLIENT);
              bsOut.Write(i);
              bsOut.Write(clients[i].x);
              bsOut.Write(clients[i].y);
              bsOut.Write(clients[i].z);
              peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,
                  packet->systemAddress,false);
            }
          }
          else
          {
            std::cout << "No clients yet, didn't send spawn pos to existing " <<
              "nor each existing pos to new\n";
          }

          // Add client 
          clients.push_back(Client(client_id));
          clients[client_id].guid = packet->guid;
          clients[client_id].x = 20;
          clients[client_id].y = 20;
          clients[client_id].z = 10;
          // Use a BitStream to write a custom user message
          // Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
          bsOut.Reset();
          bsOut.Write((MessageID)SPAWN_POSITION);
          bsOut.Write(client_id);
          bsOut.Write(20);
          bsOut.Write(20);
          bsOut.Write(10);
          peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
          bsOut.Reset();
          /*
          bsOut.Write((MessageID)YOUR_TURN);
          peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
          */
          break;
        case ID_DISCONNECTION_NOTIFICATION:
          printf("A client has disconnected.\n");
          break;
        case ID_CONNECTION_LOST:
          printf("A client lost the connection.\n");
          break;
        case POSITION_UPDATE:
          // received new position from client
          bsIn.Read(client_id);
          bsIn.Read(x);
          bsIn.Read(y);
          bsIn.Read(z);
          printf("Client %d sent new position %d,%d,%d\n", client_id, x,y,z);
          // save to vectors
          clients[client_id].x = x;
          clients[client_id].y = y;
          clients[client_id].z = z;
          std::cout << "Sending new position value to each client\n";
          bsOut.Reset();
          bsOut.Write((MessageID)POSITION_UPDATE);
          bsOut.Write(client_id);
          bsOut.Write(x);
          bsOut.Write(y);
          bsOut.Write(z);
          for(int i=0; i < (int)clients.size(); ++i)
          {
            if(client_id != i)
            {
              std::cout << "  To: " << i << " - " << clients[i].guid.g << std::endl;
              peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,
                 peer->GetSystemAddressFromGuid(clients[i].guid),false);
            }
            else
            {
              std::cout << "  Not sending to own client: " << client_id << "\n";
            }
          }
          bsOut.Reset();
          /*
          bsOut.Write((MessageID)YOUR_TURN);
          packet->systemAddress = 
            peer->GetSystemAddressFromGuid(clients[current_client]);
          if(current_client+1 >= (int)clients.size())
            current_client = 0;
          else
            ++current_client;
          peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
          */
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
