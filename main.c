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
engObj_s* room0 ;
engObj_s* portal ;
engObj_s* beach;
engObj_s* water;
vboModel* waterFrame[3];
engObj_s* globe;



sound_s* ambienceSnd;
sound_s* endSnd;
sound_s* touchSndA;
sound_s* touchSndB;

guiLabel_s* lblTl;

int minLeft=15;
int secLeft=29;
int ticksLeft=1000;

int hasLeaf=0;
int hasClickedPortal=0;
int isGameOver=0;

vec3 gameOverPos;

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



void btnBoomClick(void* whereBoomIs)
{
  explo( *((vec3*)whereBoomIs) );
}

void objPulsate( engObj_s* o )
{

  if( o->offsetRot.x < 0 )
  {
    if( o->solidColor[3] < 30 )
    {
      o->offsetRot.x = 1;
    } else {
      o->solidColor[3]-=3;
    }
  } else {
    if( o->solidColor[3] > 200 )
    {
      o->offsetRot.x = -1;
    } else {
      o->solidColor[3] +=3;
    }

  }
}

void camMoveDone()
{
  eoGuiShowCursor(1);
}

void exitMoveDone()
{
  guiWindow_s* placeHolder = eoGuiAddWindow( ingameContext, eoSetting()->res.x/2-150, eoSetting()->res.y/2-180 , 300,90, "Yay, you won!", 0 );
  eoGuiAddLabel(placeHolder, 0,0, "Well, you beat the game :)\n Press escape to exit.");
  eoSamplePlay(endSnd,128);
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

void fadeOut( engObj_s* o )
{
  if( o->solidColor[3] == 0 )
  {
    o->gameData=NULL;
    o->thinkFunc=NULL;
    o->clickedFunc=NULL;
    eoObjDel(o);
  } else {
    o->solidColor[3]-=1;
  }

}

void spawnPortalNow()
{
//  eoGuiShowCursor(1);
  eoObjAdd( portal );


  //Start room fadeout
  room0->thinkFunc = fadeOut;
}



void fadeInBeach()
{
  portal->thinkFunc = fadeOut;
  eoCamRecPlay( Data("/data/cam/","move12.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, NULL );
  eoObjAdd(beach);
  eoObjAdd(water);

}

void portalClicked( engObj_s* p, int btnState )
{
  int i=0;

  if( !eoGuiIsCursorVisible() ) { return; }

  eoGuiSetCursor(  hcur_spr,0,0  );
  //portalClicked
  if( btnState==1 )
  {
    eoGuiShowCursor(0);
    hasClickedPortal=1;
    p->clickedFunc=NULL;
    eoCamRecPlay( Data("/data/cam/","move11.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, fadeInBeach );

    tree->thinkFunc=NULL;
    for(i=0; i < 6; i++)
    {
      objPic[i]->thinkFunc=fadeOut;
    }
  }
}

void beachFadeIn(engObj_s* p)
{
  //When done fading, set clickedfunc and enable cursor
  if( p->solidColor[3] == 255 )
  {
    p->thinkFunc=NULL;
    eoCamRecPlay( Data("/data/cam/","move13.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, exitMoveDone );
  } else {
    p->solidColor[3]+=1;
  }

}

void globeSpin(engObj_s* g)
{
  g->rot.y += 0.02;
  g->rot.x += 0.1;
  g->rot.z += 0.005;
  g->pos.y += sin(g->rot.y)*0.1;
}

void waterAni(engObj_s* w)
{
  static int tickDown=250;
  static GLfloat level=0;

  level += 0.1;

  w->pos.y += sin(level)*0.1;

  tickDown -= eoTicks();
  if( tickDown < 1 )
  {
    tickDown=250;
    if( w->model == waterFrame[0] )
    {
      w->model = waterFrame[1];
    } else if( w->model == waterFrame[1] )
    {
      w->model = waterFrame[2];
    } else if( w->model == waterFrame[2] )
    {
      w->model = waterFrame[0];
    }
  }
}

void waterFadeIn(engObj_s* p)
{
  //When done fading, set clickedfunc and enable cursor
  if( p->solidColor[3] == 255 )
  {
    p->thinkFunc=waterAni;
    eoObjAdd(globe);
  } else {
    p->solidColor[3]+=1;
  }

}


void portalFadeIn(engObj_s* p)
{
  //When done fading, set clickedfunc and enable cursor
  if( p->solidColor[3] == 255 )
  {
    p->thinkFunc=NULL;
    p->solidColor[3] = 255;
    p->clickedFunc = portalClicked;
    eoGuiShowCursor(1);
  } else {
    p->solidColor[3]+=1;
  }

}

void treeMouseOverFunc( engObj_s* t, int btnState )
{
  if( !eoGuiIsCursorVisible() ) { return; }

  eoGuiSetCursor(  hcur_spr,0,0  );

  if( btnState== 1)
  {
    eoGuiShowCursor(0);
    if(hasLeaf)
    {
      tree->clickedFunc=NULL;
      eoCamRecPlay( Data("/data/cam/","move9.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, spawnPortalNow );
    } else {
      eoCamRecPlay( Data("/data/cam/","move8.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
    }
  }

}
void leafFlyDone( engObj_s* oa, engObj_s* ob )
{
  eoGuiShowCursor(1);
  explo(oa->pos);
  eoSamplePlay( touchSndB,128 );
  oa->gameData=NULL;

  oa->colTeam=0;
  ob->colTeam=0;

  oa->disabled=1;

  hasLeaf=1;
  tree->clickedFunc = treeMouseOverFunc;
}


void picsMouseOverFunc( engObj_s* o, int btnState )
{
  vec3 v;
  int i;

  if( !eoGuiIsCursorVisible() ) { return; }


  o->thinkFunc=NULL;
  eoGuiSetCursor(  hcur_spr,0,0  );
  if( btnState==1 )
  {
    eoPrint("ObjecClicks %i",(int)o->gameData);
    if( o->renderType == EO_RENDER_WIREFRAME )
    {
      o->renderType=EO_RENDER_FULL;
      eoSamplePlay(touchSndA, 128);
    } else {
      o->renderType=EO_RENDER_WIREFRAME;
      eoSamplePlay(touchSndB, 128);
    }


    lastClickedPicture=o;

    if( o == objPic[5] && (int)o->gameData == 0 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move1.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
    }
    if( o == objPic[5] && (int)o->gameData == 1 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move3.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
    }

    if( o == objPic[5] && (int)o->gameData > 1 && (int)objPic[4]->gameData==0 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move4.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
    }
    if( o == objPic[5] && (int)o->gameData > 1 && (!tree->thinkFunc|| tree->disabled ))
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move7.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
    }
    if( o == objPic[5] && (int)o->gameData > 1 && tree->thinkFunc&& !tree->disabled)
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move5.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
    }

    if( o == objPic[0] && (int)o->gameData > 0 )
    {
      eoGuiShowCursor(0);

      eoPrint("tree->thinkFunc: %p", tree->thinkFunc);
      eoPrint("tree->disabled: %i", tree->disabled);
      eoPrint("room0->thinkFunc: %p", room0->thinkFunc);
      if(tree->thinkFunc && !tree->disabled && room0->thinkFunc)
      {
        o->clickedFunc=NULL;

        v = tree->pos;
        v.y += 4;
        o->vel = eoVec3FromPoints( o->pos, v );
        o->vel = eoVec3Normalize(o->vel);
        o->vel = eoVec3Scale(v, 0.0001 );

        o->colTeam=1;
        tree->colTeam=2;
        o->colFunc=leafFlyDone;
      } else {
        eoCamRecPlay( Data("/data/cam/","move10.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
      }

    }




    if( o == objPic[0] && o->gameData==0 )
    {
      eoGuiShowCursor(0);
      eoCamRecPlay( Data("/data/cam/","move6.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );

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
        eoCamRecPlay( Data("/data/cam/","move2.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );
      }
    }

    o->gameData++;

  }
}


void inpQuitFunc(inputEvent* e )
{
  eoPrint("Quitting the game.");
  eoQuit();
}


void displayGameOver(inputEvent* e)
{

  isGameOver=1;
  eoGuiShowCursor(0);
  eoPrint("Bohoo!");
  eoCamRecStop();
  //Create window

  gameOverPos = eoCamPosGet();

  eoGuiAddLabel(ingameContext, eoSetting()->res.x/2 - 100, eoSetting()->res.y/2,"Game Over")->font=FONT_LARGE;
  eoGuiAddLabel(ingameContext, eoSetting()->res.x/2 - 80, eoSetting()->res.y/2+26,"(Press ESC)");
}

void frameStart()
{
  vec3 v;
  static GLfloat rot=0;
  if( isGameOver )
  {
    rot -=0.006;

    eoCamTargetSet(gameOverPos);
    v.y = gameOverPos.y;
    v.x = cos( rot )*5 + gameOverPos.x;
    v.z = sin( rot )*5 + gameOverPos.z;
    eoCamPosSet( v );
    return;
  }
  eoGuiSetCursor(  dcur_spr,0,0  );

  tree->thinkFunc = (objPic[5]->gameData>0 && objPic[4]->renderType==EO_RENDER_FULL)?treeRotate:NULL;

  if( (int)objPic[0]->gameData>0 && objPic[2]->renderType==EO_RENDER_FULL && tree->thinkFunc && room0->thinkFunc != fadeOut)
  {
    room0->thinkFunc = objPulsate;
    room0->fullBright = 1;

  } else {
    if( room0->thinkFunc == objPulsate )
      room0->thinkFunc = NULL;
  }


  //Countdown
  if( !hasClickedPortal && !isGameOver )
  {
    ticksLeft -= eoTicks();
    if( ticksLeft < 1 )
    {
      ticksLeft=0;
      secLeft--;

      if(secLeft < 1)
      {
        secLeft=0;
        minLeft--;

        if( minLeft < 1 && secLeft < 1 && ticksLeft < 1 )
        {
          displayGameOver(NULL);
        } else {
          secLeft = 59;
        }
      }

      if(!isGameOver)
        ticksLeft = 1000;
    }
    sprintf(lblTl->txt, "00:%i:%i.%i", minLeft, secLeft, ticksLeft );
  }



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

  //Add countdown
  lblTl = eoGuiAddLabel(ingameContext, eoSetting()->res.x/2-100,10, "00:15:30.00    ");
  lblTl->font=FONT_LARGE;


  //Set active context
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


  room0 = eoObjCreate(ENGOBJ_MODEL);
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
  tree->clickedFunc = treeMouseOverFunc; //Needs a clickfunction when baked to generate id color

  eoObjBake(tree);
  tree->clickedFunc = NULL;

  eoObjAdd(tree);


  //The portal out of the room
  portal = eoObjCreate(ENGOBJ_MODEL);
  portal->thinkFunc = portalFadeIn;
  portal->model = eoModelLoad( Data("/data/objs/",""), "portal.obj");
  portal->renderType = EO_RENDER_WIREFRAME;
  portal->fullBright = 1;
  portal->solidColor[0] = 255;
  portal->solidColor[1] = 128;
  portal->solidColor[2] = 0;
  portal->solidColor[3] = 0;
  portal->clickedFunc=portalClicked;
  eoObjBake(portal);
  portal->clickedFunc=NULL;

  beach = eoObjCreate(ENGOBJ_MODEL);
  beach->thinkFunc = beachFadeIn;
  beach->model = eoModelLoad( Data("/data/objs/",""), "beach.obj");
  beach->renderType = EO_RENDER_WIREFRAME;
  beach->fullBright = 1;
  beach->solidColor[0] = 200;
  beach->solidColor[1] = 150;
  beach->solidColor[2] = 0;
  beach->solidColor[3] = 0;
  eoObjBake(beach);

  //
  waterFrame[0] = eoModelLoad( Data("/data/objs/",""), "water0.obj");
  waterFrame[1] = eoModelLoad( Data("/data/objs/",""), "water1.obj");
  waterFrame[2] = eoModelLoad( Data("/data/objs/",""), "water2.obj");

  water = eoObjCreate(ENGOBJ_MODEL);
  water->thinkFunc = waterFadeIn;
  water->model = waterFrame[0];
  water->renderType = EO_RENDER_WIREFRAME;
  water->fullBright = 1;
  water->solidColor[0] = 0;
  water->solidColor[1] = 50;
  water->solidColor[2] = 255;
  water->solidColor[3] = 0;

  eoObjBake(water);
  water->pos.y -= 8.4;


  globe = eoObjCreate(ENGOBJ_MODEL);
  globe->model = eoModelLoad( Data("/data/objs/",""), "globe.obj");

  globe->thinkFunc = globeSpin;
  eoObjBake(globe);

  globe->pos.x = -376.361694;
  globe->pos.y = 52.695896;
  globe->pos.z = -44.950550;



  ambienceSnd = eoSampleLoad(Data("/data/sound/", "ambience.ogg"));
  endSnd = eoSampleLoad( Data("/data/sound/", "p.ogg"));
  touchSndA = eoSampleLoad(Data("/data/sound/", "ta.ogg"));
  touchSndB = eoSampleLoad(Data("/data/sound/", "tb.ogg"));


  eoSamplePlay(ambienceSnd,128);


  eoCamRecPlay( Data("/data/cam/","move0.rec"), CAM_PLAYBACK_POSITION_ABSOLUTE, camMoveDone );

  eoGuiWarpMouse( eoSetting()->res.x/2, eoSetting()->res.y/2 );

  eoInpAddFunc( "exit", "Exit the game.", inpQuitFunc, INPUT_FLAG_DOWN);
  eoInpAddFunc( "gameover", "Show the gameover screen.", displayGameOver, INPUT_FLAG_DOWN);
  eoExec("bind esc exit");

  eoMainLoop();

  return 0;
}
