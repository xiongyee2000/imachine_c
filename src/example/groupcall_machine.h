#ifndef GROUPCALL_MACHINE_H
#define GROUPCALL_MACHINE_H

#include "fsm.h"
#include "connected_machine.h"

/*==================================================
 * GROUPCALL state machine
 *==================================================*/
/*--------- states --------------*/
typedef enum
{
	GROUPCALL_S_DISAFFILIATED,
	GROUPCALL_S_AFFILIATING,
	GROUPCALL_S_AFFILIATED,
	GROUPCALL_S_CONNECTING,
	GROUPCALL_S_CONNECTED,
	GROUPCALL_S_DISCONNECTING
} groupcall_state_t;

const fsm_state_t GROUPCALL_STATES[] =
{
  {
	/* stateId */		GROUPCALL_S_DISAFFILIATED,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		GROUPCALL_S_AFFILIATING,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		GROUPCALL_S_AFFILIATED,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		GROUPCALL_S_CONNECTING,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		GROUPCALL_S_CONNECTED,
	/* isFinal */		FALSE,
	/* sub */			CONNECTED_MACHINE
  },
  {
	/* stateId */		GROUPCALL_S_DISCONNECTING,
	/* isFinal */		FALSE,
	/* sub */			NULL
  }
};

/*--------- transitions --------------*/
typedef enum
{
	GROUPCALL_T_DISAFFILIATED_TO_AFFILIATING,
	GROUPCALL_T_AFFILIATING_TO_AFFILIATED,
	GROUPCALL_T_AFFILIATED_TO_CONNECTING,
	GROUPCALL_T_CONNECTING_TO_CONNECTED,
	GROUPCALL_T_CONNECTED_TO_DISCONNECTING,
	GROUPCALL_T_DISCONNECTING_TO_AFFILIATED,
} groupcall_transition_t;

const fsm_transition_t GROUPCALL_TRANSITIONS[] =
{
	{
		/* id */		GROUPCALL_T_DISAFFILIATED_TO_AFFILIATING,
		/* source */	GROUPCALL_S_DISAFFILIATED,
		/* target */	GROUPCALL_S_AFFILIATING,
	},
	{
		/* id */		GROUPCALL_T_AFFILIATING_TO_AFFILIATED,
		/* source */	GROUPCALL_S_AFFILIATING,
		/* target */	GROUPCALL_S_AFFILIATED,
	},
	{
		/* id */		GROUPCALL_T_AFFILIATED_TO_CONNECTING,
		/* source */	GROUPCALL_S_AFFILIATED,
		/* target */	GROUPCALL_S_CONNECTING,
	},
	{
		/* id */		GROUPCALL_T_CONNECTING_TO_CONNECTED,
		/* source */	GROUPCALL_S_CONNECTING,
		/* target */	GROUPCALL_S_CONNECTED,
	},
	{
		/* id */		GROUPCALL_T_CONNECTED_TO_DISCONNECTING,
		/* source */	GROUPCALL_S_CONNECTED,
		/* target */	GROUPCALL_S_DISCONNECTING,
	},
	{
		/* id */		GROUPCALL_T_DISCONNECTING_TO_AFFILIATED,
		/* source */	GROUPCALL_S_DISCONNECTING,
		/* target */	GROUPCALL_S_AFFILIATED,
	}
};

/*--------- events --------------*/
typedef enum
{
	GROUPCALL_E_AFFILIATE_EVENT = 0,
	GROUPCALL_E_AFFILIATED_EVENT,
	GROUPCALL_E_INVITE_EVENT,
	GROUPCALL_E_INVITE_ACCEPTED_EVENT,
	GROUPCALL_E_DISCONNECT_EVENT,
	GROUPCALL_E_DISCONNECTED_EVENT
} GROUPCALL_EVENTS;
#define GROUPCALL_EVENT_NUM 6

/*--------- triggers --------------*/
const fsm_trigger_t GROUPCALL_TRIGGERS[] =
{
	{
		/* state */			GROUPCALL_S_DISAFFILIATED, 
		/* event */			GROUPCALL_E_AFFILIATE_EVENT, 
		/* transition */	GROUPCALL_T_DISAFFILIATED_TO_AFFILIATING
	},
	{
		/* state */			GROUPCALL_S_AFFILIATING, 
		/* event */			GROUPCALL_E_AFFILIATED_EVENT, 
		/* transition */	GROUPCALL_T_AFFILIATING_TO_AFFILIATED
	},
	{
		/* state */			GROUPCALL_S_AFFILIATED, 
		/* event */			GROUPCALL_E_INVITE_EVENT, 
		/* transition */	GROUPCALL_T_AFFILIATED_TO_CONNECTING
	},
	{
		/* state */			GROUPCALL_S_CONNECTING, 
		/* event */			GROUPCALL_E_INVITE_ACCEPTED_EVENT, 
		/* transition */	GROUPCALL_T_CONNECTING_TO_CONNECTED
	},
	{
		/* state */			GROUPCALL_S_CONNECTED, 
		/* event */			GROUPCALL_E_DISCONNECT_EVENT, 
		/* transition */	GROUPCALL_T_CONNECTED_TO_DISCONNECTING
	},
	{
		/* state */			GROUPCALL_S_CONNECTING, 
		/* event */			GROUPCALL_E_DISCONNECTED_EVENT, 
		/* transition */	GROUPCALL_T_DISCONNECTING_TO_AFFILIATED
	}
};


/*--------- state machine --------------*/
const fsm_machine_t GROUPCALL_MACHINE[] =
{
	/* id */				1,
	/* stateTable */		GROUPCALL_STATES,
	/* stateNum */			sizeof(GROUPCALL_STATES)/sizeof(GROUPCALL_STATES[0]),
	/* transitionTable */	GROUPCALL_TRANSITIONS,
	/* transitionNum */		sizeof(GROUPCALL_TRANSITIONS)/sizeof(GROUPCALL_TRANSITIONS[0]),
	/* eventNum */			GROUPCALL_EVENT_NUM,
	/* triggerTable */		GROUPCALL_TRIGGERS,
	/* transitionNum */		sizeof(GROUPCALL_TRIGGERS)/sizeof(GROUPCALL_TRIGGERS[0]),
	/* entryState */		GROUPCALL_S_DISAFFILIATED
};

#endif