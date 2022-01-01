
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <ctime>
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <SDL2/SDL.h>
#include <projectM.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdarg>
#include <cstdbool>
#define FLAG_DISABLE_PLAYLIST_LOAD 1

Uint8 *stm;
Uint8 *wptr;
EGLDisplay display;
EGLContext contextegl;
EGLSurface surface;
const float FPS=30;
SDL_AudioDeviceID dev;
struct{SDL_AudioSpec spec;Uint8 *snd;Uint32 slen;int pos;}wave;
typedef struct{projectM *pm;SDL_Window *win;SDL_GLContext *glCtx;bool done;projectM::Settings settings;SDL_AudioDeviceID dev;}
projectMApp;projectMApp app;
EGLint config_size,major,minor;
int lft;
char flnm[16];

static void renderFrame(){
app.pm->renderFrame();
  
eglSwapBuffers(display,surface);
auto sndat=reinterpret_cast<short*>(stm);
app.pm->pcm()->addPCM16Data(sndat,1024);
}

static const EGLint attribut_list[]={
EGL_NONE
};

static const EGLint attribute_list[]={
EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR,
EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,
EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT,EGL_TRUE,
EGL_DEPTH_ENCODING_NV,EGL_DEPTH_ENCODING_NONLINEAR_NV,
EGL_RENDER_BUFFER,EGL_QUADRUPLE_BUFFER_NV,
EGL_RED_SIZE,8,
EGL_GREEN_SIZE,8,
EGL_BLUE_SIZE,8,
EGL_ALPHA_SIZE,8,
EGL_STENCIL_SIZE,8,
EGL_DEPTH_SIZE,32,
EGL_BUFFER_SIZE,32,
EGL_NONE
};

EGLint anEglCtxAttribs2[]={
EGL_CONTEXT_CLIENT_VERSION,3,
EGL_CONTEXT_PRIORITY_LEVEL_IMG,EGL_CONTEXT_PRIORITY_REALTIME_NV,
EGL_NONE};

static void chngt(){
EmscriptenWebGLContextAttributes attr;
attr.alpha=EGL_TRUE;
attr.stencil=EGL_TRUE;
attr.depth=EGL_TRUE;
attr.antialias=EGL_FALSE;
attr.premultipliedAlpha=EGL_FALSE;
attr.preserveDrawingBuffer=EGL_FALSE;
attr.powerPreference=EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
emscripten_webgl_init_context_attributes(&attr);
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx=emscripten_webgl_create_context("#canvas",&attr);
EGLConfig eglconfig=NULL;
display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
eglInitialize(display,&major,&minor);
if(eglChooseConfig(display,attribute_list,&eglconfig,1,&config_size)==EGL_TRUE && eglconfig!=NULL){
if(eglBindAPI(EGL_OPENGL_ES_API)!=EGL_TRUE){
}
contextegl=eglCreateContext(display,eglconfig,EGL_NO_CONTEXT,anEglCtxAttribs2);
if(contextegl==EGL_NO_CONTEXT){
}
else{
surface=eglCreateWindowSurface(display,eglconfig,NULL,attribut_list);
eglMakeCurrent(display,surface,surface,contextegl);
}}
emscripten_webgl_make_context_current(ctx);
int width=EM_ASM_INT({return parseInt(document.getElementById('pmhig').innerHTML,10);});
int height=width;

app.settings.meshX=60;
app.settings.meshY=60;
app.settings.textureSize=1024;
app.settings.fps=FPS;
app.settings.windowWidth=width;
app.settings.windowHeight=width;
app.settings.smoothPresetDuration=22;
app.settings.presetDuration=44;
app.settings.beatSensitivity=5.0;
app.settings.aspectCorrection=0;
app.settings.easterEgg=0;
app.settings.shuffleEnabled=0;
app.settings.presetURL="/presets";  
app.pm=new projectM(app.settings);
printf("Init ProjectM\n");
app.pm->selectRandom(true);
printf("Select random preset.\n");
app.pm->projectM_resetGL(width,height);
printf("Reseting GL.\n");
DIR *m_dir;
if((m_dir=opendir("/"))==NULL){printf("error opening /\n");
}
else{
struct dirent *dir_entry;
while((dir_entry=readdir(m_dir))!=NULL){
printf("%s\n",dir_entry->d_name);
}}
for(uint i=0;i<app.pm->getPlaylistSize();i++){
printf("%d\t%s\n",i,app.pm->getPresetName(i).c_str());
}
emscripten_set_main_loop((void (*)())renderFrame,0,0);
}
static void swtcht(){
printf("Selecting random preset.\n");
app.pm->selectRandom(true);
}
static void lckt(){
app.pm->setPresetLock(true);
printf("Preset locked.\n");
}
static void cls_aud(){
if(dev!=0){
SDL_PauseAudioDevice(dev,SDL_TRUE);
SDL_CloseAudioDevice(dev);
dev=0;
}}
static void qu(int rc){
SDL_Quit();
exit(rc);
}
static void opn_aud(){
dev=SDL_OpenAudioDevice(NULL,SDL_FALSE,&wave.spec,NULL,0);
if(!dev){
SDL_FreeWAV(wave.snd);
qu(2);
}
SDL_PauseAudioDevice(dev,SDL_FALSE);
}

static void SDLCALL bfr(void *unused,Uint8 *stm,int len){
wptr=wave.snd+wave.pos;
lft=wave.slen-wave.pos;
while (lft<=len){
SDL_memcpy(stm,wptr,lft);
stm+=lft;
len-=lft;
wptr=wave.snd;
lft=wave.slen;
wave.pos=0;
}
SDL_memcpy(stm,wptr,len);
wave.pos+=len;
}

static void plt(){
cls_aud();
SDL_FreeWAV(wave.snd);
SDL_Quit();
SDL_SetMainReady();
if (SDL_Init(SDL_INIT_AUDIO)<0){
qu(1);
}
SDL_strlcpy(flnm,"/snd/sample.wav",sizeof(flnm));
if(SDL_LoadWAV(flnm,&wave.spec,&wave.snd,&wave.slen)==NULL){
qu(1);
}
wave.pos=0;
wave.spec.callback=bfr;
opn_aud();
}
extern "C" {
void pl(){
plt();
}
void chng(){
chngt();
}
void lck(){
lckt();
}
void swtch(){
swtcht();
}}
EM_JS(void,ma,(),{
let d=S();if(d)d();d=S();function S(){
let w$=parseInt(document.getElementById("iwid").innerHTML,10);
let h$=parseInt(document.getElementById("ihig").innerHTML,10);
w$=Math.round(w$);h$=Math.round(h$);let canvas=document.getElementById("vcanvas");
let contx=canvas.getContext('webgl2',{alpha:true,stencil:false,depth:true,premultipliedAlpha:false,imageSmoothingEnabled:false,lowLatency:true,powerPreference:'high-performance',majorVersion:2});
const g=new GPU({canvas:canvas,webGl:contx});
let Rn=parseInt(document.getElementById("frate").innerHTML,10);
let l=(w$*h$*4);let m=((l/65536)+1);m=Math.floor(m);
let W=new WebAssembly.Memory({initial:m});let o=[w$,h$];
const v=document.getElementById("mv");
const t=g.createKernel(function(v){const P=v[this.thread.y][this.thread.x];
return[P[0],P[1],P[2]];}).setTactic("precision").setPipeline(true).setOutput(o);
const r=g.createKernel(function(f){const p=f[this.thread.y][this.thread.x];
this.color(p[0],p[1],p[2]);}).setTactic("precision").setGraphical(true).setOutput(o);
let $=new Uint8ClampedArray(W.buffer,0,l);$.set(t(v),0);r(t($));
$.set(t(v),0);r(t($));$.set(t(v),0);let T=false;let ms=1;let R=16;let f=(1000/Rn);
function M(){if(T){return;}r(t($));$.set(t(v),0);let mq=((ms*f)/R);let k=Math.floor(mq);
let y=((k*f)-(k*Rn));if(y>8){R=8;}ms=ms+1;setTimeout(function(){M();},R);}M();
document.getElementById("di").onclick=function(){T=true;t.destroy();r.destroy();g.destroy();S();};return()=>{T=true;};}});
int main(){
EM_ASM({
FS.mkdir('/snd');
FS.mkdir('/textures');
FS.mkdir('/presets');
});
ma();
app.done=0;
return PROJECTM_SUCCESS;
}