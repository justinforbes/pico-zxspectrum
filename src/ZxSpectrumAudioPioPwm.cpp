#include "ZxSpectrumAudioPioPwm.h"

#if !defined(PICO_PIO_PWM_AUDIO)
#include "ZxSpectrumAudioNull.h"

void pio_pwm_audio_init()
{
  null_audio_init();
}

uint32_t pio_pwm_audio_freq()
{
  return null_audio_freq();
}

void __not_in_flash_func(pio_pwm_audio_handler)(uint32_t vA, uint32_t vB, uint32_t vC, int32_t s, uint32_t buzzer, bool mute)
{
  null_audio_handler(vA, vB, vC, s, buzzer, mute);
}

bool __not_in_flash_func(pio_pwm_audio_ready)()
{
  return null_audio_ready();
}

#else

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/printf.h"
#include "pwm.pio.h"

#include "ZxSpectrumAudioVol.h"

static uint pwm_audio_sm = 0;
static uint32_t pwm_audio_freq = 28000;

// Write `period` to the input shift register
static void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

void pio_pwm_audio_init() {

    gpio_init(SPK_PIN);
    gpio_set_dir(SPK_PIN, GPIO_OUT);
  
    uint offset = pio_add_program(PICO_AUDIO_PWM_PIO, &pwm_program);
    printf("Loaded PWM PIO at %u\n", offset);
    pwm_audio_sm = pio_claim_unused_sm(PICO_AUDIO_PWM_PIO, true);
    printf("Chosen %d for PWM PIO State machine\n", pwm_audio_sm);
    pwm_program_init(PICO_AUDIO_PWM_PIO, pwm_audio_sm, offset, SPK_PIN);
    pio_sm_set_clkdiv(PICO_AUDIO_PWM_PIO, pwm_audio_sm, 2.0); // TODO do we need to calculate this!?!
    pio_pwm_set_period(PICO_AUDIO_PWM_PIO, pwm_audio_sm, 1024);
    printf("Finish configuring PWM PIO\n");

    pwm_audio_freq = clock_get_hz(clk_sys) / (1024 * 2 * 3);
}

uint32_t pio_pwm_audio_freq() {
    return pwm_audio_freq;
}

void __not_in_flash_func(pio_pwm_audio_handler)(uint32_t vA, uint32_t vB, uint32_t vC, int32_t s, uint32_t buzzer, bool mute) {
    const int32_t l = vA + vC + vB + s - (128*3);
    const int32_t v = __mul_instruction(_vol, mute ? 0 : 60);
    const int32_t lr = __mul_instruction(v, l) >> (8 + 6);
    const int32_t k = lr + 512;
    pio_sm_put_blocking(PICO_AUDIO_PWM_PIO, pwm_audio_sm, k < 0 ? 0 : k > 1024 ? 1024 : k);
}

bool __not_in_flash_func(pio_pwm_audio_ready)() {
    return !pio_sm_is_tx_fifo_full(PICO_AUDIO_PWM_PIO, pwm_audio_sm);
}

#endif