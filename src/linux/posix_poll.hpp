//===========================================================================
// This file is part of GRWL(a fork of GLFW) licensed under the Zlib license.
// See file LICENSE.md for full license details
//===========================================================================

#include <poll.h>

GRWLbool _grwlPollPOSIX(struct pollfd* fds, nfds_t count, double* timeout);
