document.getElementById('btn5').addEventListener('click',function(){
Module.ccall("lck");
});
document.getElementById('btn4').addEventListener('click',function(){
Module.ccall("swtch");
});
document.getElementById('btn').addEventListener('click',function(){
Module.ccall('chng');
});

Module.preRun=(function(){
FS.mkdir('/presets');
});
