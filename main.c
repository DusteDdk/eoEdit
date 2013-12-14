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

guiWindow_s* winHalp;
guiWindow_s* winObjInfo;
guiLabel_s* lblInfoClassName;
guiLabel_s* lblInfoId;
guiLabel_s* lblInfoPos;
guiLabel_s* lblInfoRot;


engObj_s* selectedObj = NULL;

guiContext* ingameContext;

sprite_s* dcur_spr; //"default" cursor
sprite_s* hcur_spr; //"hover" cursor

vec3 hackyPos;
vec3 hackyRot;
GLfloat pinc=0.2;
GLfloat rinc=2;

void updateObjInfoWin()
{
  if( selectedObj == NULL )
  {
    strcpy( lblInfoClassName->txt,"Class: No Object");
    strcpy( lblInfoId->txt, "Id: No Object");
    strcpy( lblInfoPos->txt, "Pos: No Object");
    strcpy( lblInfoRot->txt, "Rot: No Object");
  } else {
    sprintf( lblInfoClassName->txt, "Class: %s", selectedObj->className  );
    sprintf( lblInfoId->txt, "Id: %i",selectedObj->id );
    sprintf( lblInfoPos->txt, "Pos: %.3f %.3f %.3f", selectedObj->pos.x,selectedObj->pos.y,selectedObj->pos.z);
    sprintf( lblInfoRot->txt, "Rot: %.3f %.3f %.3f", selectedObj->rot.x,selectedObj->rot.y,selectedObj->rot.z);
  }
}



int slow=0;
void inpSlowObj(inputEvent* e)
{
  slow = !slow;
}

void inpObjRm(inputEvent* e)
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
    eoObjDel(selectedObj);
    selectedObj=NULL;
    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpMoveObjUp(inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
    selectedObj->pos.y += (slow)?pinc/20.0:pinc;
    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpMoveObjDn(inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
    selectedObj->pos.y -= (slow)?pinc/20.0:pinc;
    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpMoveObjLeft(inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {

    vec3 dir = eoVec3FromPoints(eoCamTargetGet(), eoCamPosGet());
    dir = eoVec3Normalize( dir );
    GLfloat a = atan2( dir.z, dir.x );

    GLfloat c = cos( a+4.71238898 )*((slow)?pinc/20.0:pinc);
    GLfloat s = sin( a+4.71238898 )*((slow)?pinc/20.0:pinc);
    selectedObj->pos.x -= c;
    selectedObj->pos.z -= s;


    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpMoveObjRight (inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
    vec3 dir = eoVec3FromPoints(eoCamTargetGet(), eoCamPosGet());
    dir = eoVec3Normalize( dir );
    GLfloat a = atan2( dir.z, dir.x );

    GLfloat c = cos( a+4.71238898 )*((slow)?pinc/20.0:pinc);
    GLfloat s = sin( a+4.71238898 )*((slow)?pinc/20.0:pinc);
    selectedObj->pos.x += c;
    selectedObj->pos.z += s;


    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpMoveObjForward (inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
    vec3 dir = eoVec3FromPoints(eoCamTargetGet(), eoCamPosGet());
    dir = eoVec3Normalize( dir );
    GLfloat a = atan2( dir.z, dir.x );

    GLfloat c = cos( a );
    GLfloat s = sin( a );
    selectedObj->pos.x -= c*((slow)?pinc/20.0:pinc);
    selectedObj->pos.z -= s*((slow)?pinc/20.0:pinc);

    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpMoveObjBackward (inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
//    selectedObj->pos.z += (slow)?pinc/20.0:pinc;

    vec3 dir = eoVec3FromPoints(eoCamTargetGet(), eoCamPosGet());
    dir = eoVec3Normalize( dir );
    GLfloat a = atan2( dir.z, dir.x );

    GLfloat c = cos( a );
    GLfloat s = sin( a );
    selectedObj->pos.x += c*((slow)?pinc/20.0:pinc);
    selectedObj->pos.z += s*((slow)?pinc/20.0:pinc);

    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpRotYPlus (inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
    selectedObj->rot.y += (slow)?pinc/20.0:rinc;

    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}

void  inpRotYMinus (inputEvent* e )
{
  if( consoleVisible() ) return;
  if( selectedObj != NULL)
  {
    selectedObj->rot.y -= (slow)?pinc/20.0:rinc;
    updateObjInfoWin();
  } else {
    eoPrint("No Object Selected..");
  }
}


void inpQuitFunc(inputEvent* e )
{
  eoPrint("Exiting.");
  eoQuit();
}

void inpToggleHalp(inputEvent* e)
{
  winHalp->visible=!winHalp->visible;
}

int camFree=1;
void inpToggleCam(inputEvent* e)
{
  if(!consoleVisible() )
  {
    camFree=!camFree;
    if(camFree)
    {
      eoExec("camfree 1");
    } else {
      eoExec("camfree 0");
    }
  }

}

void frameStart()
{
  eoGuiSetCursor(  dcur_spr,0,0  );
}


void objSelect( engObj_s* obj, int mouseBtnState )
{
  eoGuiSetCursor(  hcur_spr,0,0  );

  if( mouseBtnState == INPUT_MOUSEBTN_DOWN )
  {
    eoPrint("Selected Object %i (%s) btnState %i", obj->id, obj->className,mouseBtnState);
    selectedObj = obj;
    updateObjInfoWin();
  }
}


int conPosObj( const char* args, void* notUsed)
{
  char buf[256];
  if(selectedObj)
  {

    if(args && strlen(args) > 4)
    {
    eoPrint("Positioning currently selected object %i (%s)",selectedObj->id,selectedObj->className);

    sprintf(buf,"_selectedPos %s", args);
    eoExec(buf);

      selectedObj->pos = hackyPos;
    } else {
      eoPrint("Usage: pos x y z");
    }
    updateObjInfoWin();
  } else {
    eoPrint("No object selected.");
  }
  return( CON_CALLBACK_HIDE_RETURN_VALUE );
}


int conRotObj( const char* args, void* notUsed)
{
  char buf[256];
  if(selectedObj)
  {
    if(args && strlen(args) > 4)
    {
    eoPrint("Rotating currently selected object %i (%s)",selectedObj->id,selectedObj->className);

    sprintf(buf,"_selectedRot %s", args);
    eoExec(buf);

      selectedObj->rot = hackyRot;
    } else {
      eoPrint("Usage: Rot x y z");
    }
    updateObjInfoWin();
  } else {
    eoPrint("No object selected.");
  }
  return( CON_CALLBACK_HIDE_RETURN_VALUE );
}



int conClassObj( const char* args, void* notUsed)
{
  if(selectedObj)
  {
    eoPrint("Renaming currently selected object %i (%s)",selectedObj->id,selectedObj->className);
    if(strlen(args)>0)
    {
      free(selectedObj->className);
      selectedObj->className = malloc(strlen(args)+1);
      strcpy(selectedObj->className, args);
      updateObjInfoWin();
    } else {
      eoPrint("Usage: class CLASSNAME");
    }
  } else {
    eoPrint("No object selected.");
  }
  return( CON_CALLBACK_HIDE_RETURN_VALUE );
}


void editorAddObj( engObj_s* o)
{
  o->clickedFunc = objSelect;
  eoObjBake(o);
  eoObjAdd(o);
}


void objInitFunc( engObj_s* o)
{
  eoPrint("initFunc called for obj %p (%s)",o,o->className);
  editorAddObj(o);
}

int conMergeLevel(const char* args, void* notUsed )
{
  if( args && strlen(args) > 3 )
  {
    eoPrint("Merging level file %s", Data( args,"") );
    eoLoadScene(Data( args,""), objInitFunc );
  } else {
    eoPrint("Usage merge path/to/filename.lvl");
  }
  return(CON_CALLBACK_HIDE_RETURN_VALUE);
}

int conSaveLevel(const char* args, void* notUsed )
{
  engObj_s* o;
  FILE* fp;
  char buf[512];
  if( args && strlen(args) > 3 )
  {
    fp = fopen( Data("/",args), "w");
    if(!fp)
    {
      eoPrint("Could not open %s for writing", Data("/",args));
      return(1);
    }
    eoPrint("Saving to file %s", Data(args,"") );
    gameState_s* state = eoGetState();
    listItem* li = state->world.objs;


    while( (li=li->next) )
    {
      o = (engObj_s*)li->data;
      if( o->type == ENGOBJ_MODEL )
      {
        eoPrint("Obj: %i", o->id);
        fputs( "[model]\n", fp );
        sprintf(buf, "class=%s\n", o->className);
        fputs(buf,fp);
        sprintf(buf, "pos=%f,%f,%f\n", o->pos.x,o->pos.y,o->pos.z );
        fputs(buf,fp);
        sprintf(buf, "rot=%f,%f,%f\n", o->rot.x,o->rot.y,o->rot.z );
        fputs(buf,fp);
        sprintf(buf, "file=%s%s\n", o->model->dir,o->model->name );
        fputs(buf,fp);
        fputs("[end]\n", fp);

      }
    }
    fclose(fp);


  } else {
    eoPrint("Usage save path/to/filename.lvl");
  }
  return(CON_CALLBACK_HIDE_RETURN_VALUE);
}


int conLoadObj( const char* args, void* notUsed)
{
  char model[256];
  char className[256];
  char dir[256];
  char file[256];
  int i;

  engObj_s* obj;

  if( splitVals(' ',args, model, className) )
  {
    i=charrpos(model,'/')+1;

    memcpy(dir, model, i);

    strcpy(file, model+i);

    eoPrint("ModelDir: "TXTCOL_CYAN"%s "TXTCOL_WHITE" ModelFile: "TXTCOL_CYAN"%s "TXTCOL_WHITE"ClassName: "TXTCOL_GREEN"%s", dir,file,className);
    obj = eoObjCreate( ENGOBJ_MODEL );

    obj->model = eoModelLoad(dir, file);
    if( obj->model )
    {
      obj->className = malloc( sizeof(char)* strlen(className)+1 );
      strcpy( obj->className, className );
      obj->pos = eoCamPosGet();
      editorAddObj(obj);

    } else {
      eoPrint("File not found.");
      eoObjDel(obj);
    }

  } else {
    eoPrint("Usage: "TXTCOL_RED"add "TXTCOL_CYAN"path/to/modelfile.obj "TXTCOL_GREEN"className");
    eoPrint("Ex: "TXTCOL_RED"add "TXTCOL_CYAN"data/models/coin.obj "TXTCOL_GREEN"goldenCoin");
  }


  return( CON_CALLBACK_HIDE_RETURN_VALUE );
}

int main(int argc, char *argv[])
{
	//All must bow for the migthy vector of DOOOOM!
  vec3 p;

  eoInitAll(argc, argv, DATADIR);
  
  //Enable mouse-selection
  eoGameEnableMouseSelection(0.2);

  eoExec("testbox 1");
  //Load the cursor
  sprite_base* dcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursor.spr"));
  dcur_spr = eoSpriteNew( dcur_sprb, 1, 1 );


  sprite_base* hcur_sprb = eoSpriteBaseLoad(Data("/data/gfx/","cursorH.spr"));
  hcur_spr = eoSpriteNew( hcur_sprb, 1, 1 );


  //Setup the 2D gui context
  ingameContext = eoGuiContextCreate();


  winHalp = eoGuiAddWindow(ingameContext, 10, 10, 200, 50, "Halp", NULL);

  int lblOffset=0, lblInc=13;
  eoGuiAddLabel(winHalp, 0, 0, "F1 = Console");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "h = Toggle Halp");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "click = Select Obj");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "esc = exit");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "u,o = obj move y");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "i,j,k,l  = obj move x/z");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "8,9= obj rot around y");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "space =free/lock camera");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "del = delete selected object");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "shift = move slower");
  lblOffset+=lblInc;

  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "Cmds:");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "-------------");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "add /dir/file.obj className");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "rot x y z");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "pos x y z");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "class [newClassName]");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "save FILENAME.lvl");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "merge FILENAME.lvl");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "pinc [num]");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "rinc [num]");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "testbox [0 or 1]");
  eoGuiAddLabel(winHalp, 0, lblOffset+=lblInc, "hitbox [0 or 1]");

  winHalp->_size.y +=lblOffset;

  winObjInfo = eoGuiAddWindow(ingameContext, eoSetting()->res.x- 160, eoSetting()->res.y - 110, 150,100, "Object Info", NULL );
  lblInfoClassName = eoGuiAddLabel( winObjInfo, 0,0, "Class: No Object                                                ");
  lblInfoId = eoGuiAddLabel( winObjInfo, 0,10, "Id: No Object                                                         ");
  lblInfoPos = eoGuiAddLabel( winObjInfo, 0,20, "Pos: No Object                                                       ");
  lblInfoRot = eoGuiAddLabel( winObjInfo, 0,30, "Rot: No Object                                                       ");




  hackyPos.x = 0;
  hackyPos.y = 2.5;
  hackyPos.z = 0;
  eoCamPosSet( hackyPos );
  hackyPos.x = 0;
  hackyPos.y = 0;
  hackyPos.z = -10;
  eoCamTargetSet( hackyPos );

  eoExec( "camfree 1" );

  eoFuncAdd( conLoadObj, NULL, "add");
  eoFuncAdd( conRotObj, NULL, "rot");
  eoFuncAdd( conPosObj, NULL, "pos");
  eoFuncAdd( conClassObj, NULL, "class");

  eoFuncAdd( conMergeLevel, NULL, "merge");
  eoFuncAdd( conSaveLevel, NULL, "save");

  eoVarAdd(CON_TYPE_VEC3,0, &hackyPos, "_selectedPos");
  eoVarAdd(CON_TYPE_VEC3,0, &hackyRot, "_selectedRot");


  eoVarAdd(CON_TYPE_FLOAT,0, &rinc, "rinc");
  eoVarAdd(CON_TYPE_FLOAT,0, &pinc, "pinc");

  //Set active context
  eoGuiContextSet(ingameContext);


  eoGuiWarpMouse( eoSetting()->res.x/2, eoSetting()->res.y/2 );

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
  eoGuiShowCursor(1);
  
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


  eoInpAddFunc( "exit", "Exit the game.", inpQuitFunc, INPUT_FLAG_DOWN);

  eoInpAddFunc( "halp", "Toggle the halp window.", inpToggleHalp, INPUT_FLAG_DOWN);
  eoExec("bind h halp");

  eoInpAddFunc( "camtoggle", "Toggle the camfree.", inpToggleCam, INPUT_FLAG_DOWN);
  eoExec("bind space camtoggle");

  eoInpAddFunc( "objMoveUp", "Move currently selected object up along Y.", inpMoveObjUp, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objMoveDn", "Move currently selected object down along Y.", inpMoveObjDn, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objMoveLeft", "Move currently selected object down along X.", inpMoveObjLeft, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objMoveRight", "Move currently selected object up along X.", inpMoveObjRight, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objMoveForward", "Move currently selected object down along Z.", inpMoveObjForward, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objMoveBackward", "Move currently selected object up along Z.", inpMoveObjBackward, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objRotYCW", "Rotate around Y clockwise.", inpRotYPlus, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objRotYCCW", "Rotate around Y counter-clockwise.", inpRotYMinus, INPUT_FLAG_HOLD);
  eoInpAddFunc( "objSlow", "Make movement and rotation slow.", inpSlowObj, INPUT_FLAG_UP|INPUT_FLAG_DOWN);
  eoInpAddFunc( "objRm", "Remove object", inpObjRm, INPUT_FLAG_DOWN);

  eoExec("bind u objMoveUp");
  eoExec("bind o objMoveDn");
  eoExec("bind i objMoveForward");
  eoExec("bind k objMoveBackward");
  eoExec("bind j objMoveLeft");
  eoExec("bind l objMoveRight");
  eoExec("bind 8 objRotYCW");
  eoExec("bind 9 objRotYCCW");
  eoExec("bind shiftl objSlow");
  eoExec("bind delete objRm");


  eoMainLoop();

  return 0;
}
