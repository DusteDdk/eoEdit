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

sprite_s* dcur_spr; //"default" cursor


void explo( vec3 p )
{
	static particleEmitter_s* emitter = NULL;
	if( emitter == NULL )
	{
	  emitter = eoPsysNewEmitter();
	  emitter->addictive=1;
	  emitter->numParticlesPerEmission = 250;
	  emitter->ticksBetweenEmissions = 0;
	  emitter->particleLifeMax = 200;
	  emitter->particleLifeVariance = 400;
	  emitter->shrink=0;
	  emitter->fade=0;
	  emitter->percentFlicker=70;
	  emitter->sizeMax=0.25;
	  emitter->sizeVariance=0.04;
	  emitter->rotateParticles=1;
	  emitter->colorVariance[0]=0.4;
	  emitter->colorVariance[1]=0.5;
	  emitter->colorVariance[2]=0.2;
	  emitter->colorVariance[3]=0.0;
	  emitter->color[0]=0.8;
	  emitter->color[1]=0.4;
	  emitter->color[2]=0.2;
	  emitter->emitSpeedMax=5;
	  emitter->emitSpeedVariance=1;
	  eoPsysBake(emitter);
	}

	eoPsysEmit(emitter, p);
}

void frameStart()
{

}

void btnBoomClick(void* whereBoomIs)
{
  explo( *((vec3*)whereBoomIs) );
}

int main(int argc, char *argv[])
{
	//All must bow for the migthy vector of DOOOOM!
  vec3 p;

  eoInitAll(argc, argv, DATADIR);

  //Enable mouse-selection
  eoGameEnableMouseSelection(0.1);

  //Load the cursor
  sprite_base* dcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor.spr"));
  dcur_spr = eoSpriteNew( dcur_sprb, 1, 1 );


  //Setup the 2D gui context
  ingameContext = eoGuiContextCreate();
  eoGuiContextSet(ingameContext);

  //Add a window to show it's alive.
  guiWindow_s* placeHolder = eoGuiAddWindow( ingameContext, eoSetting()->res.x/2-150, eoSetting()->res.y/2-180 , 300,90, "Still Alive", 0 );
  eoGuiAddLabel(placeHolder, 0,0, "This is my skeleton project for Ludum Dare 26.\nI forgot how to use my engine,\nit's going to be great fun!\nF1 for console.");

  //Add a button
  guiButton_s* btn = eoGuiAddButton(placeHolder,300-56,90-52,50,20,"Boom", btnBoomClick);
  btn->callbackData = &p;
  
  
  //Set camera to look somewhere
  p.x=43.838181;
  p.y=20.039286;
  p.z=-94.781746;
  eoCamPosSet(p);

  p.x=19.156969;
  p.y=21.724607;
  p.z=-98.386940;

  //and at something
  eoCamTargetSet( p );

  //Enable interface
  eoGuiShow();
  
  //Pause simulation
  eoPauseSet(0);

  //Register callback function
  eoRegisterStartFrameFunc( frameStart );

  //Set some GL light stuffs
  GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 2.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

  GLfloat pos[] = { 20,60,38,1 }; //Last pos: 0 = dir, 1=omni
  glLightfv( GL_LIGHT0, GL_POSITION, pos );

  GLfloat specular[] = {0.0, 0.0, 0.0, 1.0};
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

  eoExec("autoexec.cmd");
  eoMainLoop();

  return 0;
}
