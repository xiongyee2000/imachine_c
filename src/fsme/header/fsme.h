/* ---------------------------------------------------------
 * Finite State Machine Engine
 * 
 * Characteristics:
 * - Naked Finite State Machine Engine
     (State Machine without any actions)
 * - Action Registration Mechanism
 * 
 * Limitation:
 * - Guard function not supported
 * - Shallow History/Deep History not supported
 * - Single-threaded only
 * - Parallel state Machine not supported
 * ---------------------------------------------------------*/
#ifndef FSME_H
#define FSME_H


#include "fsm.h"


struct fsme_action;
typedef struct fsme_action* fsme_action_ptr_t;



/* ---------- TYPE DEFINITIONS ---------- */
/**
 * The engine state type
 */
typedef struct fsme_state
{
	int							id; 
	boolean						isFinal;

	/**
	 * The header of the entry action list
	 */
	fsme_action_ptr_t			entryAction;

	/**
	 * The header of the exit action list
	 */
	fsme_action_ptr_t 			exitAction;

	/** 
	 * The pointer to the sub engine. 
	 * NULL if the state is not a sub machine. 
	 */
	fsme_engine_ptr_t			subEngine; 
} fsme_state_t;


/**
 * The engine transition type
 */
typedef struct fsme_transition
{
	int							id;

	/**
	 * The guard function
	 */
	fsme_guardFuncPtr_t			guard;

	/**
	 * The header of the transition action list
	 */
	fsme_action_ptr_t			action;
	fsme_state_t*				sourceState;
	fsme_state_t*				targetState;
} fsme_transition_t;


/**
 * The engine trigger type
 */
typedef struct 
{
	/**
	 * The event id. Event id must start from 0
	 * and grows incrementally.
	 */
	int							eventId;
	unsigned int				transitionNum;


	/**
	 * The transition list of the trigger.
	 * Each element of the list is a pointer
	 * to the transition.
	 */
	fsme_transition_t**			transitions;
} fsme_trigger_t;


/**
 * Type definition of the state machine engine 
 */
typedef struct fsme_engine
{
	int							id;

	/**
	 * engine state table 
	 */
	fsme_state_t*				stateTable;

	/** 
	 * number of engine states 
	 */
	int							stateNum;

	/** 
	 * engine transition table 
	 */
	fsme_transition_t*			transitionTable;

	/** 
	 * number of engine transitions
	 */
	int							transitionNum;

	/** 
	 * Engine trigger table. 
	 * Triggers must be put into this table 
	 * in order of its event id. 
	 */
	fsme_trigger_t*				triggerTable;

	/** 
	 * number of events
	 */
	int							eventNum;

	/**
	 * is a sub state machine or not
	 */
	boolean						isSubStateMachine;
	
	/**
	 * Parent state machine engine. NULL if 
	 * the engine does not have a parent.
	 */
	struct fsme_engine*			parent;

	/** 
	 * the init engine state 
	 */
	fsme_state_t *				entryState;

	/** 
	 * the current active state 
	 */
	fsme_state_t *				activeState;

	/**
	 * the header of the entry action list 
	 */
	fsme_action_ptr_t			entryAction;

	/**
	 * the header of the exit action list 
	 */
	fsme_action_ptr_t			exitAction;

	/** 
	 * is event disabled or not (internal use only) 
	 */
	boolean						eventDisabled;

} fsme_engine_t;



/* ------------- CONSTANTS ------------- */



/* ------------- FUNCTION PROTOTYPES ------------- */
/**
 * Clear up all registered actions. 
 *
 * When an engine is created by fsme_newEngine(), the 
 * user can always release the dynamically allocated
 * memory by invoking fsme_deleteEngine(). 
 * However, if the user choose to create an engine
 * by other means (e.g. from the stack), he should 
 * call this function to avoid possible memory leak.
 * 
 * @return
 *
 * @param
 * engine		- the engine to be cleared
 */
void
fsme_clearActions(fsme_engine_ptr_t engine);

#endif
