/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libeo/eng.h"

#ifndef DATADIR
  #define DATADIR "."
#endif


guiContext* ingameContext;
engObj_s* objPic[6];
engObj_s* tree;
engObj_s* lastClickedPicture=NULL;
engObj_s* treePsysObj[3];

sprite_s* dcur_spr; //"default" cursor
sprite_s* hcur_spr; //"hover" cursor
char buf[2048];

void explo( vec3 p )
{
	static particleEmitter_s* emitter = NULL;
	if( emitter == NULL )
	{
	  emitter = eoPsysNewEmitter();
	  emitter->addictive=1;
	  emitter->numParticlesPerEmission = 200;
	  emitter->ticksBetweenEmissions = 0;
	  emitter->particleLifeMax = 500;
	  emitter->particleLifeVariance = 2500;
	  emitter->shrink=1;
	  emitter->fade=0;
	  emitter->percentFlicker=50;
	  emitter->sizeMax=0.11;
	  emitter->sizeVariance=0.07;
	  emitter->rotateParticles=0;
	  emitter->colorVariance[0]=0.7;
	  emitter->colorVariance[1]=0.2;
	  emitter->colorVariance[2]=0.6;
	  emitter->colorVariance[3]=0.0;
	  emitter->color[0]=0.9;
	  emitter->color[1]=0.4;
	  emitter->color[2]=0.7;
	  emitter->emitSpeedMax=1.6;
	  emitter->emitSpeedVariance=0.5;
	  eoPsysBake(emitter);
	}

	eoPsysEmit(emitter, p);
}

void treeRotate(engObj_s* o )
{
  o->rot.y += 0.3;
}

void parRot( engObj_s* o )
{
  o->rot.y -=0.6;
  o->pos.x = cos(o->rot.y)*2;
  o->pos.z = sin(o->rot.y)*2;
}

void frameStart()
{
  eoGuiSetCursor(  dcur_spr,0,0  );

  tree->thinkFunc = (objPic[5]->gameData>0 && objPic[4]->renderType==EO_RENDER_FULL)?treeRotate:NULL;


}

void btnBoomClick(void* whereBoomIs)
{
  explo( *((vec3*)whereBoomIs) );
}

void objPulsate( engObj_s* o )
{

  o->solidColor[3]++;
}

void camMoveDone()
{
  eoGuiShowCursor(1);
}

void exitMoveDone()
{
  guiWindow_s* placeHolder = eoGuiAddWindow( ingameContext, eoSetting()->res.x/2-150, eoSetting()->res.y/2-180 , 300,90, "Yay, you won!", 0 );
  eoGuiAddLabel(placeHolder, 0,0, "Well, you beat the game :)\n Press escape to exit.");
}

particleEmitter_s* psysGen()
{
  //The particles for the tree top

  particleEmitter_s* e = eoPsysNewEmitter();
  e->addictive=1;
  e->numParticlesPerEmission = 10;
  e->ticksBetweenEmissions = 100;
  e->emitTimeVariance = 90;
  e->particleLifeMax = 9000;
  e->particleLifeVariance = 2000;
  e->shrink=0;
  e->fade=0;
  e->percentFlicker=40;
  e->sizeMax=0.008;
  e->sizeVariance=0.005;
  e->rotateParticles=0;
  e->colorVariance[0]=0.7;
  e->colorVariance[1]=0.2;
  e->colorVariance[2]=0.6;
  e->colorVariance[3]=0.0;
  e->color[0]=0.9;
  e->color[1]=0.3;
  e->color[2]=0.8;
  e->emitSpeedMax=0.07;
  e->emitSpeedVariance=0.045;
  e->wind.y = -0.1;
  eoPsysBake(e);
  return(e);
}

void picsMouseOverFunc( engObj_s* o, int btnState )
{
  vec3 v;
  int i;

  o->thinkFunc=NULL;
  eoGuiSetCursor(  hcur_spr,0,0  );
  if( btnState==1 )
  {
    eoPrint("ObjecClicks %i",(int)o->gameData);
    if( o->renderType == EO_RENDER_WIREFRAME )
    {
      o->renderType=EO_RENDER_FULL;
    } else {
      o->renderType=EO_RENDER_WIREFRAME;
    }


    lastClickedPicture=o;

    if( o == objPic[5] && (int)o->gameData == 0 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move1.rec"), TRUE, camMoveDone );
    }
    if( o == objPic[5] && (int)o->gameData == 1 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move3.rec"), TRUE, camMoveDone );
    }

    if( o == objPic[5] && (int)o->gameData > 1 && (int)objPic[4]->gameData==0 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move4.rec"), TRUE, camMoveDone );
    }
    if( o == objPic[5] && (int)o->gameData > 1 && (!tree->thinkFunc|| tree->disabled ))
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move7.rec"), TRUE, camMoveDone );
    }
    if( o == objPic[5] && (int)o->gameData > 1 && tree->thinkFunc&& !tree->disabled)
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move5.rec"), TRUE, camMoveDone );
    }

    if( o == objPic[0] && o->gameData==0 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move6.rec"), TRUE, exitMoveDone );

      for(i=0; i<3;i++)
      {
        treePsysObj[i]=eoObjCreate(ENGOBJ_PAREMIT);
        treePsysObj[i]->emitter=psysGen();
        eoObjBake(treePsysObj[i]);
       // eoObjAdd(treePsysObj[i] );
        eoObjAdd(treePsysObj[i]);

        treePsysObj[i]->pos.y=4.9;
        treePsysObj[i]->thinkFunc = parRot;
      }

    }

    if( o == objPic[3] )
    {
      tree->disabled= (o->renderType==EO_RENDER_FULL)?0:1;

      explo( tree->pos );
      v = tree->pos;
      v.y +=1;
      explo( v );
      v.y +=1;
      explo( v );
      v.y +=1;
      explo( v );
      v.y +=1;
      explo( v );
      v.y +=1;
      explo( v );

      if( o->gameData==0 )
      {
        eoGuiShowCursor(0);
        eoCamRecPlay( Data("/data/cam/","move2.rec"), TRUE, camMoveDone );
      }
    }

    o->gameData++;

  }
}

void inpQuitFunc(inputEvent* e )
{
  eoPrint("Quitting the game.");
  eoExec("quit 1");
}


int main(int argc, char *argv[])
{
	//All must bow for the migthy vector of DOOOOM!
  vec3 p;

  eoInitAll(argc, argv, DATADIR);

  //Enable mouse-selection
  eoGameEnableMouseSelection(0.2);

  //Load the cursor
  sprite_base* dcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor.spr"));
  dcur_spr = eoSpriteNew( dcur_sprb, 1, 1 );


  sprite_base* hcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursorH.spr"));
  hcur_spr = eoSpriteNew( hcur_sprb, 1, 1 );


  //Setup the 2D gui context
  ingameContext = eoGuiContextCreate();
  eoGuiContextSet(ingameContext);

  //Set camera to somewhere
  p.x=14;
  p.y=2;
  p.z=-2.5;
  eoCamPosSet(p);

  p.x=-8.52066;
  p.y=2.107272;
  p.z=7.96192;
  //and at something
  eoCamTargetSet( p );

  //Enable 2Dinterface
  eoGuiShow();
  eoGuiShowCursor(0);
  
  //Pause simulation
  eoPauseSet(FALSE);

  //Register callback function
  eoRegisterStartFrameFunc( frameStart );

  //Set some GL light stuffs
  GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 2.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

  GLfloat pos[] = { 10,10,10,0 }; //Last pos: 0 = dir, 1=omni
  glLightfv( GL_LIGHT0, GL_POSITION, pos );

  GLfloat specular[] = {0.0, 0.0, 0.0, 1.0};
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);


  engObj_s* room0 = eoObjCreate(ENGOBJ_MODEL);
  room0->model = eoModelLoad( Data("/data/objs/room0/",""), "room0.obj" );
  room0->renderType=EO_RENDER_WIREFRAME;
  eoObjBake(room0);
  eoObjAdd( room0 );


  int i;
  for(i=0; i < 6; i++)
  {
    sprintf(buf, "pic%i.obj",i );
    objPic[i] = eoObjCreate(ENGOBJ_MODEL);
    objPic[i]->model = eoModelLoad( Data("/data/objs/room0/",""), buf );
    objPic[i]->renderType=EO_RENDER_WIREFRAME;
    objPic[i]->clickedFunc=picsMouseOverFunc;
    objPic[i]->fullBright=1;
    eoObjBake(objPic[i]);
    eoObjAdd( objPic[i] );
  }

  objPic[0]->thinkFunc=objPulsate;


  //Load the obj
  engObj_s* room1 = eoObjCreate(ENGOBJ_MODEL);
  room1->model = eoModelLoad( Data("/data/objs/",""), "world.obj" );

  eoObjBake(room1);

  //Add tree
  tree = eoObjCreate(ENGOBJ_MODEL);
  tree->model = eoModelLoad( Data("/data/objs/",""), "tree.obj" );
  tree->renderType = EO_RENDER_WIREFRAME;
  tree->solidColor[0] = 0;
  tree->solidColor[2] = 0;
  tree->fullBright=1;
  tree->disabled=1;

  eoObjBake(tree);

  eoObjAdd(tree);


  eoCamRecPlay( Data("/data/cam/","move0.rec"), TRUE, camMoveDone );

  eoGuiWarpMouse( eoSetting()->res.x/2, eoSetting()->res.y/2 );

  eoInpAddFunc( "exit", "Exit the game.", inpQuitFunc, INPUT_FLAG_DOWN);
  eoExec("bind esc exit");

  eoMainLoop();

  return 0;
}
