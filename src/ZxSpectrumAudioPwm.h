#pragma once

#include <pico/stdlib.h>

uint32_t pwm_audio_init();

void pwm_audio_handler(uint32_t vA, uint32_t vB, uint32_t vC, int32_t s, uint32_t buzzer, bool mute);

bool pwm_audio_ready();
