#ifndef MAIN_H
#define MAIN_H

#include "defs.h"


enum TScrMode {smNone,smConfig,smPassword,smPreview,smSaver};

extern TScrMode ScrMode;

extern bool MessagePump();

#endif
