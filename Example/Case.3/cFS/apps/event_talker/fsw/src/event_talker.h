#ifndef _event_talker_h_
#define _event_talker_h_

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define SAMPLE_PIPE_DEPTH                     32

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (EVENT_TALKER_Main), these
**       functions are not called from any other source module.
*/
void EVENT_TALKER_Main(void);
void EVENT_TALKER_Init(void);
void EVENT_TALKER_ProcessCommandPacket(void);
void EVENT_TALKER_ProcessGroundCommand(void);
void EVENT_TALKER_ReportHousekeeping(void);
void EVENT_TALKER_ResetCounters(void);

bool EVENT_TALKER_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _event_talker_h_ */
