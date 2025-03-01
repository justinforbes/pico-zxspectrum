#pragma once

#include "ZxSpectrum.h"
#include "ZxSpectrumDisplay.h"
#include "ZxSpectrumMenu.h"

void ZxSt7789LcdRenderLoopInit(ZxSpectrumMenu& picoRootWin);

void ZxSt7789LcdRenderLoop(
    ZxSpectrum &zxSpectrum,
    volatile uint &frames,
    bool &showMenu,
    bool &toggleMenu);

extern void ZxSt7789LcdRenderLoopCallbackLine(uint32_t y);
extern void ZxSt7789LcdRenderLoopCallbackMenu(bool state);
