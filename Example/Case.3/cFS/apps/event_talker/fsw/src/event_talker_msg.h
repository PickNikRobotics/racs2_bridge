#ifndef _event_talker_msg_h_
#define _event_talker_msg_h_

/*
** SAMPLE App command codes
*/
#define EVENT_TALKER_NOOP_CC                 0
#define EVENT_TALKER_RESET_COUNTERS_CC       1

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

} EVENT_TALKER_NoArgsCmd_t;

/*************************************************************************/
/*
** Type definition (SAMPLE App housekeeping)
*/
typedef struct
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              event_talker_command_error_count;
    uint8              event_talker_command_count;
    uint8              spare[2];

}   OS_PACK event_talker_hk_tlm_t  ;

#define EVENT_TALKER_HK_TLM_LNGTH   sizeof ( event_talker_hk_tlm_t )
#define EVENT_TALKER_LISTENER_LNGTH   sizeof ( event_talker_hk_tlm_t )

#endif /* _event_talker_msg_h_ */
