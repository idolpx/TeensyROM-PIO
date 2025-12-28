
#ifndef MIDIDEVNAME_H
#define MIDIDEVNAME_H

// this code must be placed into a .c file
#include "usb_names.h"

// length must match the number of characters in the name.

#define MIDI_NAME       {'T','e','e','n','s','y','R','O','M'}
#define MIDI_NAME_LEN   9

// Do not change this part.  This exact format is required by USB.

extern struct usb_string_descriptor_struct usb_string_product_name;

#endif // MIDIDEVNAME_H