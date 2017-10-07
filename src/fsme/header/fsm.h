/* ---------------------------------------------------------
 * Finite State Machine
 * 
 * Characteristics:
 * - Naked Finite State Machine Engine
     (State Machine without any actions)
 * - Action Registration Mechanism
 * 
 * Limitation:
 * - Shallow History/Deep History not supported
 * - Single-threaded only
 * - Parallel state Machine not supported
 * ---------------------------------------------------------*/
#ifndef FSM_H
#define FSM_H


#include "fsme_defs.h"


/* --------------- MACROS --------------- */
#define FSME_FINAL_STATE_ID -1



/* ---------- TYPE DEFINITIONS ---------- */
typedef enum
{
	/** 
	 * No error
	 */
	FSME_OK = 0,

	/** 
	 * Returned when the transition
	 * fails because of the failure of
	 * the transition guard.
	 */
	FSME_TRANSITION_FAILURE,

	/**
	 * Returned in case of one of the followings:
	 * 1. Trying to start an engine when it is 
	 *    already started;
	 * 2. Trying to shut down an engine when it
	 *    is already shut down;
	 * 3. Trying to post an event to a engine
	 *    when it has not been started yet;
	 * 4. Trying to start/shutdown a sub engine manually;
	 */
	FSME_FORBIDDEN,

	/** 
	 * Returned when the event is not acceptable 
	 * by the current active state.
	 */
	FSME_INVALID_EVENT,

	/** 
	 * Returned when trying to post an event 
	 * to the engine while it is processing 
	 * actions. Because doing so may change the 
	 * current active state of the state machine.
	 */
	FSME_ENGINE_FROZEN,
	
	/**
	 * Fatal error.
	 */
	FSME_ERROR_FATAL,
} fsme_return_t;


/**
 * Prototype of the action function.
 */
typedef void (* fsme_func_t)(int id, 
							 const void* inContext, 
							 void* outContext);


/**
 * Prototype of the guard function. (Not used now)
 */
typedef boolean (* fsme_guardFuncPtr_t)(int id, 
										const void* 
										inContext, 
										void* outContext);


/**
 * The state type
 */
typedef struct fsm_state
{
	const int					id; 
	const boolean				isFinal;

	/**
	 * The pointer to the sub machine. 
	 * NULL if the state is not a sub machine. 
	 */
	struct fsm_machine const * const	subMachine;		
} fsm_state_t;


/**
 * The transition type
 */
typedef struct fsm_transition
{
	const int					id;
	const int					sourceStateId;
	const int					targetStateId;
} fsm_transition_t;


/**
 * The trigger type
 */
typedef struct fsm_trigger
{
	const int					stateId; //for future use
	const int					eventId;
	const int					transitionId;
} fsm_trigger_t;


/** 
 * Type definition of the State Machine 
 */
typedef struct fsm_machine
{
	const int					id;

	/**
	 * state table 
	 */
	fsm_state_t const * const	stateTable;

	/** 
	 * number of states 
	 */
	const int					stateNum;

	/** 
	 * transition table 
	 */
	fsm_transition_t const * const	transitionTable;

	/** 
	 * number of transitions
	 */
	const int					transitionNum;

	/** 
	 * number of events
	 */
	const int					eventNum;

	/** 
	 * trigger table. 
	 * Triggers must be put into this table 
	 * in order of its event id. 
	 */
	fsm_trigger_t const * const triggerTable;

	/** 
	 * number of triggers
	 */
	const int					triggerNum;
	
	/** 
	 * the init state 
	 */
	const int					entryStateId;
} fsm_machine_t;


typedef struct fsme_engine* fsme_engine_ptr_t;

struct fsm_machine;
struct fsme_engine;
struct fsme_state;


/* ------------- CONSTANTS ------------- */
extern const fsm_state_t FSM_FINAL_STATE;



/* ------------- FUNCTION PROTOTYPES ------------- */
/**
 * New an state machine engine instance.
 *
 * @Return
 * The pointer to the new engine instance.
 *
 * @param
 * stateMachine		- The state machine from which
 *                    the engine is to be created
 */
fsme_engine_ptr_t
fsme_newEngine(const fsm_machine_t* stateMachine);


/**
 * Delete new-ed state machine engine.
 *
 * @Return
 *
 * @param 
 * engine		- The engine to be deleted
 */
void
fsme_deleteEngine(fsme_engine_ptr_t engine);


/** 
 * Start the state machine engine.
 *
 * @Return
 * Refer to fsme_return_t.
 * 
 * @param
 * engine		- The engine to be started
 * inContext	- The input context
 * outContext	- The output context
 */
fsme_return_t
fsme_startEngine(fsme_engine_ptr_t engine,
				 const void* inContext,
				 void* outContext);


/** 
 * Start the state machine engine. This is an 
 * impolite shutdown.
 *
 * @Return
 * Refer to fsme_return_t.
 * 
 * @param
 * engine		- The engine to be shut down
 * inContext	- The input context
 * outContext	- The output context
 */
fsme_return_t
fsme_shutdownEngine(fsme_engine_ptr_t engine,
				 const void* inContext,
				 void* outContext);


/** 
 * Post event to a state machine engine.
 *
 * @Return
 * Refer to fsme_return_t.
 * 
 * @param
 * engine		- The engine the event posted to
 * event		- The event to be posted.
 * inContext	- The input context
 * outContext	- The output context
 */
fsme_return_t
fsme_postEvent(fsme_engine_ptr_t engine,
				 int event,
				 const void* inContext,
				 void* outContext);


/** 
 * Get the current state of a state machine engine.
 *
 * @Return
 * The pointer to the current engine-state of 
 * the engine.
 * NULL if the engine is not started.
 * 
 * @param
 * engine		- The state machine engine
 */
struct fsme_state*
fsme_getCurrentState(fsme_engine_ptr_t engine);


/**
 * Get the parent engine of a state machine engine.
 *
 * @Return
 * The pointer to the parent engine of the given 
 * engine, or NULL if the given engine is a root 
 * engine.
 * 
 * @param
 * engine		- The state machine engine
 */
fsme_engine_ptr_t
fsme_getParent(fsme_engine_ptr_t engine);


/**
 * Get the sub engine of a state if it has.
 *
 * @Return
 * The pointer to the sub engine of the given 
 * state, or NULL if the given state is not a 
 * sub state machine.
 * 
 * @param
 * parent		- The engine the state belongs to.
 * stateId		- The id of the state.
 */
fsme_engine_ptr_t
fsme_getSubEngine(fsme_engine_ptr_t parent, 
				  int stateId);


/**
 * Register machine entry action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be registered to
 * action		- The action to be registered
 */
void
fsme_addMachineEntryAction(fsme_engine_ptr_t engine, 
						   fsme_func_t action);


/**
 * Register machine exit action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be registered to
 * action		- The action to be registered
 */
void
fsme_addMachineExitAction(fsme_engine_ptr_t engine, 
						  fsme_func_t action);


/**
 * Remove machine entry action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be removed from
 * action		- The action to be removed
 */
void
fsme_removeMachineEntryAction(fsme_engine_ptr_t engine, 
							  fsme_func_t action);


/**
 * Remove machine exit action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be removed from
 * action		- The action to be removed
 */
void
fsme_removeMachineExitAction(fsme_engine_ptr_t engine, 
							 fsme_func_t action);


/**
 * Register state entry action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be registered to
 * stateId		- The id of the state
 * action		- The action to be registered
 */
void
fsme_addStateEntryAction(fsme_engine_ptr_t engine, 
						 int stateId, 
						 fsme_func_t action);


/**
 * Remove state entry action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be removed from
 * stateId		- The id of the state
 * action		- The action to be removed
 */
void
fsme_removeStateEntryAction(fsme_engine_ptr_t engine, 
							int stateId, 
							fsme_func_t action);


/**
 * Register state exit action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be registered to
 * stateId		- The id of the state
 * action		- The action to be registered
 */
void
fsme_addStateExitAction(fsme_engine_ptr_t engine, 
						int stateId, 
						fsme_func_t action);


/**
 * Remove state exit action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be removed from
 * stateId		- The id of the state
 * action		- The action to be removed
 */
void
fsme_removeStateExitAction(fsme_engine_ptr_t engine, 
						   int stateId, 
						   fsme_func_t action);


/**
 * Register transition action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be registered to
 * transitionId	- The id of the transition
 * action		- The action to be registered
 */
void
fsme_addTransitionAction(fsme_engine_ptr_t engine, 
						 int transitionId, 
						 fsme_func_t action);


/**
 * Remove state exit action. 
 *
 * @Return
 *
 * @param
 * engine		- The engine to be removed from
 * transitionId	- The id of the transition
 * action		- The action to be removed
 */
void
fsme_removeTransitionAction(fsme_engine_ptr_t engine, 
							int transitionId, 
							fsme_func_t action);


/**
 * Set guard function for a transition. 
 *
 * @Return
 * TRUE if the guard function is set successfully, 
 * or FALSE otherwise.
 *
 * @param
 * engine		- The state machine engine
 * transitionId	- The id of the transition
 * guardFunc	- The guard fucntion to be set
 */
boolean 
fsme_setGuard(fsme_engine_ptr_t engine,
			  int transitionId, 
			  fsme_guardFuncPtr_t guardFunc);

#endif
