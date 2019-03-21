/*
I'm gonna try to use glfw to make the window and then use directx to draw into
the window. If this get's too dificult i'd use win32 api which i used before.
*/
#pragma once
// The following #defines (by planetchili) disable a bunch of unused windows stuff. If you 
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).

#define WIN32_LEAN_AND_MEAN //exclude APIs such as Cryptography, DDE, RPC, Shell, and Windows Sockets.
#define NOGDICAPMASKS
//#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
//#define NOCTLMGR // Control and Dialog routines
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM  // excludes the serial communication API
#define NOKANJI
#define NOHELP
//#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define STRICT


#include <windows.h>
#include <WinInet.h>
#include <wrl.h> //ComPtr
#include <ShlObj.h> //COM base api / IActiveDesktop
#include <d3d11.h> // DirectX 11
#include <d3dcompiler.h> // HLSL
#include <iostream>


#define WIDTH 720
#define HEIGHT 480
#define TITLE "DirectX Test"
