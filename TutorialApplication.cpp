/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "TutorialApplication.h"

#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include "RakNetTypes.h"
#include "BitStream.h"

// RakNet Default
#define SERVER_PORT 60000

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
  // create your scene here :)
  Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "cube.mesh");

  Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
  headNode->attachObject(ogreHead);

  for (int i = 0; i < 10; ++i) {
    Ogre::String node_name, entity_name;
    Ogre::stringstream out;
    node_name = "Client_node_";
    entity_name = "Head_";
    out << i;
    node_name.append(out.str());
    entity_name.append(out.str());
    Ogre::Entity* ogreHead = mSceneMgr->createEntity(entity_name, "ogrehead.mesh");

    Ogre::SceneNode* headNode = 
      mSceneMgr->getRootSceneNode()->createChildSceneNode(node_name);
    headNode->attachObject(ogreHead);
    headNode->setVisible(false);
  }




  // Create a light
  Ogre::Light* l = mSceneMgr->createLight("MainLight");
  l->setPosition(20,80,50);

  Ogre::Entity *ent;
  Ogre::Plane p;
  p.normal = Ogre::Vector3(0,1,0); p.d = 0;
  Ogre::MeshManager::getSingleton().createPlane("FloorPlane",
      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
      p, 200000, 200000, 20, 20, true, 1, 9000, 9000,
      Ogre::Vector3::UNIT_Z);
  // Create an entity (the floor)
  ent = mSceneMgr->createEntity("floor", "FloorPlane");
  ent->setMaterialName("Examples/BeachStones");
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
  // Set ambient light
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
}



#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        TutorialApplication app;

        char str[512];
        app.connected = false;
        app.peer = RakNetworkFactory::GetRakPeerInterface();

        app.peer->Startup(1,30,&SocketDescriptor(), 1);
        //printf("Enter server IP or hit enter for 127.0.0.1\n");
        //gets(str);
        //if (str[0]==0){
          strcpy(str, "127.0.0.1");
        //}
        printf("Starting the client.\n");
        app.peer->Connect(str, SERVER_PORT, 0,0);




        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
