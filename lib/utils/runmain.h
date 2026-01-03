//////////////////////////////////////////////////////////////////

// This code provided by AndyA via the PJRC forum
// https://forum.pjrc.com/index.php?threads/teensy-4-1-dual-boot-capability.74479/post-339451

typedef  void (*pFunction) (void);

void runMainTRApp_FromMin();

void runMainTRApp();

void ErrorLoopForever (const char *Msg);

// disable and invalidate both caches. Disable MPU.
// assembled from bits of core_cm7.h
// uses various #defines from that file that will need to be included.
// probably some library functions we could call instead of this.
void disableCache();
