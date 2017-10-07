#ifndef CONNECTED_MACHINE_H
#define CONNECTED_MACHINE_H

#include "fsm.h"

/*==================================================
 * CONNECTED state machine
 *==================================================*/
/*--------- states --------------*/
typedef enum
{
	CONNECTED_STATE_IDLE,
	CONNECTED_STATE_LISTENING,
	CONNECTED_STATE_REQUESTING,
	CONNECTED_STATE_TALKING,
	CONNECTED_STATE_RELEASING
} connected_state_t;

const fsm_state_t CONNECTED_STATES[] =
{
  {
	/* stateId */		CONNECTED_STATE_IDLE,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		CONNECTED_STATE_LISTENING,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		CONNECTED_STATE_REQUESTING,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		CONNECTED_STATE_TALKING,
	/* isFinal */		FALSE,
	/* sub */			NULL
  },
  {
	/* stateId */		CONNECTED_STATE_RELEASING,
	/* isFinal */		FALSE,
	/* sub */			NULL
  }
};

/*--------- transitions --------------*/
typedef enum
{
	CONNECTED_T_IDLE_TO_REQUESTING,
	CONNECTED_T_IDLE_TO_LISTENING,
	CONNECTED_T_LISTENING_TO_IDLE,
	CONNECTED_T_REQUESTING_TO_IDLE,
	CONNECTED_T_REQUESTING_TO_TALKING,
	CONNECTED_T_TALKING_TO_RELEASING,
	CONNECTED_T_RELEASING_TO_IDLE
} connected_transition_t;

const fsm_transition_t CONNECTED_TRANSITIONS[] =
{
	{
		/* id */		CONNECTED_T_IDLE_TO_REQUESTING,
		/* source */	CONNECTED_STATE_IDLE,
		/* target */	CONNECTED_STATE_REQUESTING,
	},
	{
		/* id */		CONNECTED_T_IDLE_TO_LISTENING,
		/* source */	CONNECTED_STATE_IDLE,
		/* target */	CONNECTED_STATE_LISTENING,
	},
	{
		/* id */		CONNECTED_T_LISTENING_TO_IDLE,
		/* source */	CONNECTED_STATE_LISTENING,
		/* target */	CONNECTED_STATE_IDLE,
	},
	{
		/* id */		CONNECTED_T_REQUESTING_TO_IDLE,
		/* source */	CONNECTED_STATE_REQUESTING,
		/* target */	CONNECTED_STATE_IDLE,
	},
	{
		/* id */		CONNECTED_T_REQUESTING_TO_TALKING,
		/* source */	CONNECTED_STATE_REQUESTING,
		/* target */	CONNECTED_STATE_TALKING,
	},
	{
		/* id */		CONNECTED_T_TALKING_TO_RELEASING,
		/* source */	CONNECTED_STATE_TALKING,
		/* target */	CONNECTED_STATE_RELEASING,
	},
	{
		/* id */		CONNECTED_T_RELEASING_TO_IDLE,
		/* source */	CONNECTED_STATE_RELEASING,
		/* target */	CONNECTED_STATE_IDLE,
	}
};

/*--------- events --------------*/
typedef enum
{
	CONNECTED_E_LAUNCH_CALL_EVENT = 0,
	CONNECTED_E_INCOMING_CALL_EVENT,
	CONNECTED_E_REQUESTING_CONFIRMED_EVENT,
	CONNECTED_E_IDLE_EVENT,
	CONNECTED_E_END_CALL_EVENT,
} CONNECTED_EVENTS;
#define CONNECTED_EVENT_NUM 5

/*--------- triggers --------------*/
const fsm_trigger_t CONNECTED_TRIGGERS[] =
{
	{
		/* state */			CONNECTED_STATE_IDLE, 
		/* event */			CONNECTED_E_LAUNCH_CALL_EVENT, 
		/* transition */	CONNECTED_T_IDLE_TO_REQUESTING
	},
	{
		/* state */			CONNECTED_STATE_IDLE, 
		/* event */			CONNECTED_E_INCOMING_CALL_EVENT, 
		/* transition */	CONNECTED_T_IDLE_TO_LISTENING
	},
	{
		/* state */			CONNECTED_STATE_LISTENING, 
		/* event */			CONNECTED_E_IDLE_EVENT, 
		/* transition */	CONNECTED_T_LISTENING_TO_IDLE
	},
	{
		/* state */			CONNECTED_STATE_REQUESTING, 
		/* event */			CONNECTED_E_REQUESTING_CONFIRMED_EVENT, 
		/* transition */	CONNECTED_T_REQUESTING_TO_TALKING
	},
	{
		/* state */			CONNECTED_STATE_REQUESTING, 
		/* event */			CONNECTED_E_END_CALL_EVENT, 
		/* transition */	CONNECTED_T_REQUESTING_TO_IDLE
	},
	{
		/* state */			CONNECTED_STATE_TALKING, 
		/* event */			CONNECTED_E_END_CALL_EVENT, 
		/* transition */	CONNECTED_T_TALKING_TO_RELEASING
	},
	{
		/* state */			CONNECTED_STATE_RELEASING, 
		/* event */			CONNECTED_E_IDLE_EVENT, 
		/* transition */	CONNECTED_T_RELEASING_TO_IDLE
	}
};


/*--------- state machine --------------*/
const fsm_machine_t CONNECTED_MACHINE[] =
{
	/* id */				0,
	/* stateTable */		CONNECTED_STATES,
	/* stateNum */			sizeof(CONNECTED_STATES)/sizeof(CONNECTED_STATES[0]),
	/* transitionTable */	CONNECTED_TRANSITIONS,
	/* transitionNum */		sizeof(CONNECTED_TRANSITIONS)/sizeof(CONNECTED_TRANSITIONS[0]),
	/* eventNum */			CONNECTED_EVENT_NUM,
	/* triggerTable */		CONNECTED_TRIGGERS,
	/* transitionNum */		sizeof(CONNECTED_TRIGGERS)/sizeof(CONNECTED_TRIGGERS[0]),
	/* entryState */		CONNECTED_STATE_IDLE
};

#endif