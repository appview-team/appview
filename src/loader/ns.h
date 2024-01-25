#ifndef __NS_H__
#define __NS_H__

#include <stdbool.h>
#include <unistd.h>

#include "appviewtypes.h"

int nsForkAndExec(pid_t, pid_t, bool);
bool nsSetNsRootDir(const char *, pid_t, const char *);

int nsAttach(pid_t, const char *);

#endif // __NS_H__
