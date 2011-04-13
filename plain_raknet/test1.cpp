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
	ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1
};

int main(void)
{
	char str[512];
	RakPeerInterface *peer = RakNetworkFactory::GetRakPeerInterface();
	bool isServer;
  Packet *packet;

	printf("(C) or (S)erver?\n");
	gets(str);
	if ((str[0]=='c')||(str[0]=='C'))
	{
		peer->Startup(1,30,&SocketDescriptor(), 1);
		isServer = false;
	} else {
		peer->Startup(MAX_CLIENTS, 30, &SocketDescriptor(SERVER_PORT,0), 1);
		isServer = true;
	}

  if (isServer)
  {
    printf("Starting the server.\n");
    // We need to let the server accept incoming connections from the clients
    peer->SetMaximumIncomingConnections(MAX_CLIENTS);
  } 
  else {
    printf("Enter server IP or hit enter for 127.0.0.1\n");
    gets(str);
    if (str[0]==0){
      strcpy(str, "127.0.0.1");
    }
    printf("Starting the client.\n");
    peer->Connect(str, SERVER_PORT, 0,0);

  }

  while (1)
  {
    for (packet=peer->Receive(); packet; peer->DeallocatePacket(packet), packet=peer->Receive())
    {
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
        case ID_CONNECTION_REQUEST_ACCEPTED:
          {
            printf("Our connection request has been accepted.\n");

            std::string in="default";
            std::cin >> in;
            while(in != "q")
            {
              char *a=new char[in.size()+1];
              a[in.size()]=0;
              memcpy(a,in.c_str(),in.size());
              // Use a BitStream to write a custom user message
              // Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
              RakNet::BitStream bsOut;
              bsOut.Write((MessageID)ID_GAME_MESSAGE_1);
              bsOut.Write(a);
              peer->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);

              std::cin >> in;
            }
            printf("Thanks for chatting\n");
          }
          break;					
        case ID_NEW_INCOMING_CONNECTION:
          printf("A connection is incoming.\n");
          break;
        case ID_NO_FREE_INCOMING_CONNECTIONS:
          printf("The server is full.\n");
          break;
        case ID_DISCONNECTION_NOTIFICATION:
          if (isServer){
            printf("A client has disconnected.\n");
          } else {
            printf("We have been disconnected.\n");
          }
          break;
        case ID_CONNECTION_LOST:
          if (isServer){
            printf("A client lost the connection.\n");
          } else {
            printf("Connection lost.\n");
          }
          break;
        case ID_GAME_MESSAGE_1:
          {
            RakNet::RakString rs;
            RakNet::BitStream bsIn(packet->data,packet->length,false);
            bsIn.IgnoreBytes(sizeof(MessageID));
            bsIn.Read(rs);
            printf("%s\n", rs.C_String());
          }
          break;
        default:
          printf("Message with identifier %i has arrived.\n", packet->data[0]);
          break;
      }
    }
  }

  RakNetworkFactory::DestroyRakPeerInterface(peer);

	return 0;
}
