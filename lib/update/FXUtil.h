//******************************************************************************
// FXUTIL.H -- FlasherX utility functions
//******************************************************************************
#ifndef FXUTIL_H_
#define FXUTIL_H_

#include <Arduino.h>
#include <stdint.h>

void read_ascii_line (Stream *serial, char *line, int maxbytes);
void update_firmware (Stream *in, Stream *out,
                      uint32_t buffer_addr, uint32_t buffer_size);

bool isFab2x();
#endif
