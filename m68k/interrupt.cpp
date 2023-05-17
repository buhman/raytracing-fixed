#include <stdint.h>

#include "scsp.h"

extern void * _jojo_start __asm("_binary_jojo_11025_s16be_1ch_pcm_start");

static volatile int32_t frame = 0;

extern "C"
void auto_vector_1(void) __attribute__ ((interrupt_handler));
void auto_vector_1(void)
{
  // reset TIMER_A interrupt
  scsp.reg.ctrl.SCIRE = INT__TIMER_A;

  frame++;
  //if (frame > 3962) frame = 0;
  
  const uint16_t * jojo_start = reinterpret_cast<uint16_t *>(&_jojo_start);
  const uint32_t frame_addr = reinterpret_cast<uint32_t>(&jojo_start[frame * 8192]);
  
  scsp_slot& slot = scsp.reg.slot[frame % 32];
  slot.LOOP = 0;
  slot.LOOP |= LOOP__KYONEX;
  
  slot.LOOP = LOOP__KYONB | LOOP__SA(frame_addr); // kx kb sbctl[1:0] ssctl[1:0] lpctl[1:0] 8b sa[19:16]
  slot.SA = SA__SA(frame_addr); // start address (bytes)
  slot.LSA = 0; // loop start address (samples)
  slot.LEA = 8192; // loop end address (samples)
  slot.EG = EG__AR(0x1f) | EG__EGHOLD; // d2r d1r ho ar krs dl rr
  slot.VOLUME = 0; // stwinh sdir tl
  slot.FM = 0; // mdl mdxsl mdysl
  slot.PITCH = PITCH__OCT(-2) | PITCH__FNS(0); // oct fns
  slot.LFO = 0; // lfof plfows
  slot.MIXER = MIXER__DISDL(0b101); // disdl dipan efsdl efpan

  slot.LOOP |= LOOP__KYONEX;
  
  return;
}

void main()
{
  for (long i = 0; i < 807; i++) { asm volatile ("nop"); }   // wait for (way) more than 30µs
  
  scsp.reg.ctrl.MIXER = MIXER__MEM4MB | MIXER__MVOL(0xf);

  scsp.reg.ctrl.TIMA = TIMA__TACTL(7);

  // timer A is vector 1 (0b001)
  scsp.reg.ctrl.SCILV2 = 0;
  scsp.reg.ctrl.SCILV1 = 0;
  scsp.reg.ctrl.SCILV0 = SCILV__TIMER_A;
  
  // enable TIMER_A
  scsp.reg.ctrl.SCIRE = INT__TIMER_A;
  scsp.reg.ctrl.SCIEB = INT__TIMER_A;
  
  asm volatile ("move.w #8192,%sr");

  const uint16_t * jojo_start = reinterpret_cast<uint16_t *>(&_jojo_start);
  const uint32_t frame_addr = reinterpret_cast<uint32_t>(&jojo_start[frame * 128]);
  
  scsp_slot& slot = scsp.reg.slot[0];
  slot.LOOP = 0;
  slot.LOOP |= LOOP__KYONEX;
  
  slot.LOOP = LOOP__KYONB | LOOP__SA(frame_addr); // kx kb sbctl[1:0] ssctl[1:0] lpctl[1:0] 8b sa[19:16]
  slot.SA = SA__SA(frame_addr); // start address (bytes)
  slot.LSA = 0; // loop start address (samples)
  slot.LEA = 128; // loop end address (samples)
  slot.EG = EG__AR(0x1f) | EG__EGHOLD; // d2r d1r ho ar krs dl rr
  slot.VOLUME = 0; // stwinh sdir tl
  slot.FM = 0; // mdl mdxsl mdysl
  slot.PITCH = PITCH__OCT(-2) | PITCH__FNS(0); // oct fns
  slot.LFO = 0; // lfof plfows
  slot.MIXER = MIXER__DISDL(0b101); // disdl dipan efsdl efpan

  slot.LOOP |= LOOP__KYONEX;  
}

extern "C"
void start(void)
{
  main();
  while (1);
}

