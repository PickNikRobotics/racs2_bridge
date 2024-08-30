/*
**   Include Files:
*/

#include "event_talker.h"
#include "event_talker_perfids.h"
#include "event_talker_msgids.h"
#include "event_talker_msg.h"
#include "racs2_user_msg.h"
#include "event_talker_events.h"
#include "event_talker_version.h"
#include "RACS2Brdige_std_msgs.pb-c.h"

/*
** global data
*/

event_talker_hk_tlm_t EVENT_TALKER_HkTelemetryPkt;
racs2_user_msg_t      RACS2_UserMsgPkt;
CFE_SB_PipeId_t       EVENT_TALKER_CommandPipe;
CFE_SB_MsgPtr_t       EVENT_TALKER_MsgPtr;

static CFE_EVS_BinFilter_t  EVENT_TALKER_EventFilters[] =
       {  /* Event ID    mask */
          {EVENT_TALKER_STARTUP_INF_EID,       0x0000},
          {EVENT_TALKER_COMMAND_ERR_EID,       0x0000},
          {EVENT_TALKER_COMMANDNOP_INF_EID,    0x0000},
          {EVENT_TALKER_COMMANDRST_INF_EID,    0x0000},
       };

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* EVENT_TALKER_Main() -- Application entry point and main process loop          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void EVENT_TALKER_Main( void )
{
    int32  status;
    uint32 RunStatus = CFE_ES_RunStatus_APP_RUN;

    OS_printf("EVENT_TALKER_Main starts.\n");

    CFE_ES_PerfLogEntry(EVENT_TALKER_PERF_ID);

    EVENT_TALKER_Init();

    /*
    ** EVENT_TALKER Runloop
    */
    int count = 0;
    while (CFE_ES_RunLoop(&RunStatus) == true)
    {
        CFE_ES_PerfLogExit(EVENT_TALKER_PERF_ID);

        // /* Pend on receipt of command packet -- timeout set to 500 millisecs */
        // status = CFE_SB_RcvMsg(&EVENT_TALKER_MsgPtr, EVENT_TALKER_CommandPipe, 500);

        CFE_ES_PerfLogEntry(EVENT_TALKER_PERF_ID);

        // if (status == CFE_SUCCESS)
        // {
        //     EVENT_TALKER_ProcessCommandPacket();
        // }
        sleep(2);

        // send message
        // set topic name
        strcpy(RACS2_UserMsgPkt.ros2_topic_name, "/Recv/RACS2Bridge");
        // define serialized body data
        void *buffer;
        int len=0;
        RACS2BridgeStdMsgs *message;
        message=(RACS2BridgeStdMsgs *)malloc(sizeof(RACS2BridgeStdMsgs));
        racs2_bridge_std_msgs__init(message);
        int string_length = 22;
        char* buf[32];
        sprintf(buf, "Message To ROS2 :%5d", count);
        message->string_data = (char *)malloc(sizeof(string_length));
        OS_printf("EVENT_TALKER: [Send][MsgID=0x%x][%s]\n", RACS2_BRIDGE_MID, buf);
        strncpy(message->string_data, buf, string_length);

        len = racs2_bridge_std_msgs__get_packed_size(message);
        buffer=malloc(len);
        racs2_bridge_std_msgs__pack(message, buffer);

        // set body data
        strncpy(RACS2_UserMsgPkt.body_data, buffer, len);
        // set body data length
        RACS2_UserMsgPkt.body_data_length = len;

        // send data
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &RACS2_UserMsgPkt);
        status = CFE_SB_SendMsg((CFE_SB_Msg_t *) &RACS2_UserMsgPkt);
        // OS_printf("EVENT_TALKER: Sent message, MID = [0x%x], event_talker_command_count = %d\n",
        //     CFE_SB_GetMsgId((CFE_SB_MsgPtr_t) &RACS2_UserMsgPkt),
        //     RACS2_UserMsgPkt.event_talker_command_count
        //     );
        if (status != CFE_SUCCESS) {
            OS_printf("EVENT_TALKER: Error: sending is failed. status = 0x%x\n", status);
        }

        free(buffer);
        free(message->string_data);
        free(message);
        memset(buf, '\0', sizeof(buf));

        count++;
    }

    CFE_ES_ExitApp(RunStatus);

} /* End of EVENT_TALKER_Main() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* EVENT_TALKER_Init() --  initialization                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void EVENT_TALKER_Init(void)
{
    /*
    ** Register the app with Executive services
    */
    CFE_ES_RegisterApp() ;

    /*
    ** Register the events
    */
    CFE_EVS_Register(EVENT_TALKER_EventFilters,
                     sizeof(EVENT_TALKER_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_EventFilter_BINARY);

    EVENT_TALKER_ResetCounters();

    CFE_SB_InitMsg(&EVENT_TALKER_HkTelemetryPkt,
                   EVENT_TALKER_HK_TLM_MID,
                   EVENT_TALKER_HK_TLM_LNGTH, true);

    CFE_SB_InitMsg(&RACS2_UserMsgPkt, RACS2_BRIDGE_MID, RACS2_USER_MSG_LNGTH, false);

    CFE_EVS_SendEvent (EVENT_TALKER_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
               "EVENT_TALKER App Initialized. Version %d.%d.%d.%d",
                EVENT_TALKER_MAJOR_VERSION,
                EVENT_TALKER_MINOR_VERSION,
                EVENT_TALKER_REVISION,
                EVENT_TALKER_MISSION_REV);

} /* End of EVENT_TALKER_Init() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  EVENT_TALKER_ProcessCommandPacket                                 */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the EVENT_TALKER */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void EVENT_TALKER_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(EVENT_TALKER_MsgPtr);

    switch (MsgId)
    {
        case EVENT_TALKER_CMD_MID:
            EVENT_TALKER_ProcessGroundCommand();
            break;

        case EVENT_TALKER_SEND_HK_MID:
            EVENT_TALKER_ReportHousekeeping();
            break;

        default:
            EVENT_TALKER_HkTelemetryPkt.event_talker_command_error_count++;
            CFE_EVS_SendEvent(EVENT_TALKER_COMMAND_ERR_EID,CFE_EVS_EventType_ERROR,
            "EVENT_TALKER: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End EVENT_TALKER_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* EVENT_TALKER_ProcessGroundCommand() -- EVENT_TALKER ground commands      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void EVENT_TALKER_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(EVENT_TALKER_MsgPtr);

    /* Process "known" EVENT_TALKER app ground commands */
    switch (CommandCode)
    {
        case EVENT_TALKER_NOOP_CC:
            EVENT_TALKER_HkTelemetryPkt.event_talker_command_count++;
            CFE_EVS_SendEvent(EVENT_TALKER_COMMANDNOP_INF_EID,
                        CFE_EVS_EventType_INFORMATION,
            "EVENT_TALKER: NOOP command");
            break;

        case EVENT_TALKER_RESET_COUNTERS_CC:
            EVENT_TALKER_ResetCounters();
            break;

        /* default case already found during FC vs length test */
        default:
            break;
    }
    return;

} /* End of EVENT_TALKER_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  EVENT_TALKER_ReportHousekeeping                                   */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void EVENT_TALKER_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &EVENT_TALKER_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &EVENT_TALKER_HkTelemetryPkt);
    return;

} /* End of EVENT_TALKER_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  EVENT_TALKER_ResetCounters                                        */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void EVENT_TALKER_ResetCounters(void)
{
    /* Status of commands processed by the EVENT_TALKER App */
    EVENT_TALKER_HkTelemetryPkt.event_talker_command_count       = 0;
    EVENT_TALKER_HkTelemetryPkt.event_talker_command_error_count = 0;

    CFE_EVS_SendEvent(EVENT_TALKER_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION,
        "EVENT_TALKER: RESET command");
    return;

} /* End of EVENT_TALKER_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* EVENT_TALKER_VerifyCmdLength() -- Verify command packet length            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool EVENT_TALKER_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{
    bool result = true;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID   = CFE_SB_GetMsgId(msg);
        uint16         CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(EVENT_TALKER_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = false;
        EVENT_TALKER_HkTelemetryPkt.event_talker_command_error_count++;
    }

    return(result);

} /* End of EVENT_TALKER_VerifyCmdLength() */

