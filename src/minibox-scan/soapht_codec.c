#include "soapht_codec.h"

/* Set only by a verified HorseThief codec implementation.  Keeping this NULL
 * makes production scanning fail closed before any unverified command is sent
 * to the M1522. */
const struct soapht_codec *minibox_soapht_codec = 0;
