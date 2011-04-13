#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"

#include "RakNetTypes.h"
#include "BitStream.h"

#include <string>
#include <iostream>

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
  char str[512];
	RakPeerInterface *peer = RakNetworkFactory::GetRakPeerInterface();
  Packet *packet;

  peer->Startup(1,30,&SocketDescriptor(), 1);

  printf("Enter server IP or hit enter for 127.0.0.1\n");
  gets(str);
  if (str[0]==0){
    strcpy(str, "127.0.0.1");
  }
  printf("Starting the client.\n");
  peer->Connect(str, SERVER_PORT, 0,0);

  bool connected = false;
  RakNet::RakString rs;
  int int_message;
  RakNet::BitStream bsOut;

  while (1)
  {
    for (packet=peer->Receive(); 
         packet; 
         peer->DeallocatePacket(packet), packet=peer->Receive())
    {
      RakNet::BitStream bsIn(packet->data,packet->length,false);
      bsIn.IgnoreBytes(sizeof(MessageID));
      bsIn.Read(int_message);

      switch (packet->data[0])
      {
        case SPAWN_POSITION:
          std::cout << "Server said I'm client number " << int_message << std::endl;
          break;
        case ID_CONNECTION_REQUEST_ACCEPTED:
          connected = true;
          break;
        case POSITION_UPDATE:
          // received new position of other player from server
          std::cout << "Server said we are now at " << int_message << std::endl;
          break;
        case YOUR_TURN:
          printf("My Turn. Sending message.\n");
          bsOut.Write((MessageID)POSITION_UPDATE);
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

