#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsme.h"


/* ------------------- Local Macros -------------------------------- */
//////////////////////////////
//Engine functions
//////////////////////////////
#define fsmeEngineStarted(engine)	\
	(NULL != ((fsme_engine_ptr_t)engine)->activeState)

#define fsmeEngineGetEntryAction(engine)	\
	(((fsme_engine_ptr_t)engine)->entryAction)

#define fsmeEngineGetExitAction(engine)		\
	(((fsme_engine_ptr_t)engine)->exitAction)

#define fsmeEngineGetAction(engine, wantEntryAction)	\
	((boolean)wantEntryAction ? fsmeEngineGetEntryAction(engine): \
	fsmeEngineGetExitAction(engine))

#define fsmEngineGetEntryState(engine)    \
    (((fsme_engine_ptr_t)engine)->entryState)

#define fsmeEngineGetActiveState(engine)	\
	(((fsme_engine_ptr_t)engine)->activeState)

#define fsmeEngineSetActiveState(engine, state)	\
	(((fsme_engine_ptr_t)engine)->activeState = state)

#define fsmeEngineGetEventCount(engine)    \
    (((fsme_engine_ptr_t)engine)->eventNum)

#define fsmeEngineFindTrigger(engine,event) \
	(&(((fsme_engine_ptr_t)engine)->triggerTable[event]))


//////////////////////////////
//State functions
//////////////////////////////
#define fsmeStateGetId(state)	\
	(((fsme_state_ptr_t)state)->id)

#define fsmeStateIsFinal(state)	\
	(FSME_FINAL_STATE_ID == fsmeStateGetId(state))

#define fsmeStateGetEntryAction(state)	\
	(state->entryAction)

#define fsmeStateGetExitAction(state)	\
	(state->exitAction)

#define fsmeStateGetAction(state, wantEntryAction)	\
	(wantEntryAction ? \
	fsmeStateGetEntryAction(state) : fsmeStateGetExitAction(state))


//////////////////////////////
//Transition functions
//////////////////////////////
#define fsmeTransitionGetGuard(transition)	\
	(((fsme_transition_t*)transition)->guard)

#define fsmeTransitionHasGuard(transition)	\
	(NULL != fsmeTransitionGetGuard(transition))

#define fsmeTransitionGetAction(transition)	\
	(((fsme_transition_t*)transition)->action)
	
#define fsmeTransitionHasAction(transition)	\
	(NULL != fsmeTransitionGetAction(transition))

#define fsmeTransitionGetSourceState(transition)	\
	(((fsme_transition_t*)transition)->sourceState)

#define fsmeTransitionGetTargetState(transition)	\
	(((fsme_transition_t*)transition)->targetState)



/* ------------------- local type definitions --------------------- */
typedef fsme_state_t * fsme_state_ptr_t;

/* the action node type */
typedef struct fsme_action
{
	fsme_func_t action;
	struct fsme_action * next;
} fsme_action_t;

const fsm_state_t FSM_FINAL_STATE = 
{
    FSME_FINAL_STATE_ID,
	TRUE, /* is final state */
	NULL  /* sub state machine */
};



/* --------------- local function prototypes ------------------- */
static fsme_engine_ptr_t
fsmeDoNewEngine(const fsm_machine_t* stateMachine, 
				fsme_engine_ptr_t parent);
static void
fsmeEnterEngine(fsme_engine_ptr_t engine, 
				const void* inContext,
				void* outContext);
static void
fsmeExitEngine(fsme_engine_ptr_t engine, 
				const void* inContext,
				void* outContext);

static void
fsmeEnterState(fsme_engine_ptr_t engine,
			   fsme_state_t const * targetState,
			   const void* inContext,
			   void* outContext);
static void
fsmeExitState(fsme_engine_ptr_t engine,
			  fsme_state_ptr_t  srcState,
			  const void* inContext,
			  void* outContext);
static fsme_state_t*
fsmeGetStateById(const fsme_engine_t* engine, 
				 int id);
static fsme_transition_t*
fsmeGetTransitionById(const fsme_engine_t* engine, 
					  int id);

static fsme_action_ptr_t
fsme_getLastAction(fsme_action_ptr_t headNode);
static fsme_action_ptr_t
fsme_createAction(fsme_func_t func);
static void
fsme_addEngineAction(fsme_engine_ptr_t engine,
					 boolean wantEntryAction,
					 fsme_func_t action);
static void
fsme_addStateAction(fsme_engine_ptr_t engine,
					int stateId,
					boolean wantEntryAction,
					fsme_func_t action);
static void
fsme_appendAction(fsme_action_ptr_t action, 
				  fsme_func_t func);
static void
fsme_removeAction(fsme_action_ptr_t *head, 
				  fsme_func_t func);
static fsme_return_t
fsmeProcessTransition(fsme_engine_ptr_t engine,
					  fsme_transition_t* transition,
					  const void* inContext,
					  void* outContext);
static void 
fsme_processActions(fsme_engine_ptr_t engine, 
					fsme_action_ptr_t actionNode, 
					int id, 
					const void* inContext, 
					void* outContext);

static void
fsme_clearActionList(fsme_action_ptr_t *head);



/* ------------------ Implementations --------------------------- */
fsme_engine_ptr_t
fsme_newEngine(const fsm_machine_t* stateMachine)
{
	return fsmeDoNewEngine(stateMachine, NULL);
}


void
fsme_deleteEngine(fsme_engine_ptr_t engine)
{
	int i = 0, j = 0;

	if (NULL == engine) return;

	//clear all registered actions
	fsme_clearActions(engine);

	//release transition group tables
	for (i=0; i<engine->eventNum; i++) {
		free(engine->triggerTable[i].transitions);
	}
	
	//release the trigger table
	free(engine->triggerTable);

	//release the transition table
	free(engine->transitionTable);

	//release the state table
	for (i=0; i<engine->stateNum; i++) {
		if (NULL != engine->stateTable[i].subEngine)
		{
			fsme_deleteEngine(engine->stateTable[i].subEngine);
		}
	}
	free(engine->stateTable);
	
	//release the engine
	free(engine);
}


fsme_return_t
fsme_startEngine(fsme_engine_ptr_t engine,
				 const void* inContext,
				 void* outContext)
{
	if (!fsmeEngineStarted(engine) && 
		NULL == engine->parent) {
		fsmeEnterEngine(engine, inContext, outContext);
		return FSME_OK;
	} else {
		return FSME_FORBIDDEN;
	}
}


fsme_return_t
fsme_shutdownEngine(fsme_engine_ptr_t engine,
				 const void* inContext,
				 void* outContext)
{
	if (fsmeEngineStarted(engine) && 
		NULL == engine->parent) {
		fsmeExitEngine(engine, 
                       inContext, 
                       outContext);
		return FSME_OK;
	} else {
		return FSME_FORBIDDEN;
	}
}


fsme_return_t
fsme_postEvent(fsme_engine_ptr_t  engine,
			   int event,
			   const void* inContext,
			   void* outContext)
{
    fsme_state_t const *   srcState = NULL;
    fsme_return_t retVal = FSME_OK;
	fsme_trigger_t const * triggerTable = NULL;
	fsme_trigger_t const * trigger = NULL;
	boolean validEvent = FALSE;
	unsigned int i = 0;

	if (NULL == engine) {
#ifdef FSME_DEBUG
		fprintf(stderr, 
			"[FSME_ERROR]: Engine is NULL! \n");
#endif
		return FSME_ERROR_FATAL;
	}

    /* check if the engine has been started */
	if (!fsmeEngineStarted(engine)) {
#ifdef FSME_DEBUG
		fprintf(stderr, 
			"[FSME_ERROR]: Engine is not started! \n");
#endif
		return FSME_FORBIDDEN;
	}

    /* check if the engine has been frozen */
	if (engine->eventDisabled) {
#ifdef FSME_DEBUG
		fprintf(stderr, 
			"[FSME_ERROR]: Engine is frozen! \n");
#endif
		return FSME_ENGINE_FROZEN;
	}

    /* check if it is an unknown event */
    if (event < 0 || event >= 
		fsmeEngineGetEventCount(engine)) {
#ifdef FSME_DEBUG
		fprintf(stderr, 
			"[FSME_ERROR]: Unknown event! \n");
#endif
        return FSME_INVALID_EVENT;
    }

    /* find trigger */
    trigger = fsmeEngineFindTrigger(engine, event);

    /* get current state */
	srcState = fsmeEngineGetActiveState(engine);

	/* Find corresponding transition and process it */
	for(i = 0; i < trigger->transitionNum; i++) {
		if (fsmeTransitionGetSourceState(trigger->
			transitions[i]) == srcState) {
			/* process transition */
			retVal = fsmeProcessTransition(engine, 
						trigger->transitions[i], 
						inContext, 
						outContext);
			validEvent = TRUE;
			break;
		}
	}

	if (!validEvent) {
		retVal = FSME_INVALID_EVENT;
#ifdef FSME_DEBUG
		fprintf(stdout, 
			"[FSME_DEBUG]: Invalid event! \n");
#endif
	}

	return retVal;
}


fsme_engine_ptr_t
fsme_getSubEngine(fsme_engine_ptr_t parent, 
				  int stateId)
{
	fsme_engine_ptr_t subEngine = NULL;
	fsme_state_ptr_t state = NULL;

	if (NULL != parent) {
		state = fsmeGetStateById(parent, stateId);
		if (NULL != state) {
			subEngine = state->subEngine;
		}
	}
	return subEngine;
}


struct fsme_state*
fsme_getCurrentState(fsme_engine_ptr_t engine)
{
	if (NULL != engine) {
		return engine->activeState;
	} else {
		return NULL;
	}
}


fsme_engine_ptr_t
fsme_getParent(fsme_engine_ptr_t engine)
{
	if (NULL != engine) {
		return engine->parent;
	} else {
		return NULL;
	}
}


void
fsme_addMachineEntryAction(fsme_engine_ptr_t engine, 
						   fsme_func_t action)
{
	fsme_addEngineAction(engine, TRUE, action);
}


void
fsme_addMachineExitAction(fsme_engine_ptr_t engine, 
						  fsme_func_t action)
{
	fsme_addEngineAction(engine, FALSE, action);
}


void
fsme_removeMachineEntryAction(fsme_engine_ptr_t engine, 
							  fsme_func_t action)
{
	fsme_removeAction(&fsmeEngineGetEntryAction(engine), 
		action);
}


void
fsme_removeMachineExitAction(fsme_engine_ptr_t engine, 
							 fsme_func_t action)
{
	fsme_removeAction(&fsmeEngineGetExitAction(engine), 
		action);
}


void
fsme_addStateEntryAction(fsme_engine_ptr_t engine, 
						 int stateId, 
						 fsme_func_t action)
{
	fsme_addStateAction(engine, stateId, TRUE, action);
}


void
fsme_addStateExitAction(fsme_engine_ptr_t engine, 
						int stateId, 
						fsme_func_t action)
{
	fsme_addStateAction(engine, stateId, FALSE, action);
}


void
fsme_removeStateEntryAction(fsme_engine_ptr_t engine, 
							int stateId, 
							fsme_func_t action)
{
	fsme_state_t* state = NULL;
	
	if (NULL != engine && NULL != action) {
		state = fsmeGetStateById(engine, stateId);
		if (NULL == state) return;

		fsme_removeAction(&fsmeStateGetEntryAction(state), 
			action);
	}
}



void
fsme_removeStateExitAction(fsme_engine_ptr_t engine, 
						   int stateId, 
						   fsme_func_t action)
{
	fsme_state_t* state = NULL;
	
	if (NULL != engine && NULL != action) {
		state = fsmeGetStateById(engine, stateId);
		if (NULL == state) return;

		fsme_removeAction(&fsmeStateGetExitAction(state), 
			action);
	}
}


void
fsme_addTransitionAction(fsme_engine_ptr_t engine, 
						 int transitionId, 
						 fsme_func_t action)
{
	fsme_transition_t* transition = NULL;
	fsme_action_ptr_t firstAction = NULL;
	
	if (NULL != engine && NULL != action) {
		transition = fsmeGetTransitionById(engine, 
			transitionId);
		if (NULL == transition) return;

		firstAction = fsmeTransitionGetAction(transition);
		if (NULL != firstAction) {
			fsme_appendAction(firstAction, action);
		} else {
			transition->action = 
				fsme_createAction(action);
		}
	}
}


void
fsme_removeTransitionAction(fsme_engine_ptr_t engine, 
							int transitionId, 
							fsme_func_t action)
{
	fsme_transition_t* transition = NULL;
	
	if (NULL != engine && NULL != action) {
		transition = fsmeGetTransitionById(engine, 
			transitionId);
		if (NULL == transition) return;

		fsme_removeAction(&fsmeTransitionGetAction(transition), 
			action);
	}
}


boolean
fsme_setGuard(fsme_engine_ptr_t engine,
			  int transitionId, 
			  fsme_guardFuncPtr_t guardFunc)
{
    boolean rtn = FALSE;
	fsme_transition_t* transition = NULL;
	
	if (NULL != engine) {
		transition = fsmeGetTransitionById(engine, 
			transitionId);
		if (NULL != transition) {
		    transition->guard = guardFunc;
            rtn = TRUE;
        }
    }
    return rtn;
}


void
fsme_clearActions(fsme_engine_ptr_t engine)
{
	int i = 0;
	fsme_action_ptr_t head = NULL;

	if (NULL == engine) return;
	
	//clear state machine actions
	fsme_clearActionList(&engine->entryAction);
	fsme_clearActionList(&engine->exitAction);

	//clear all state actions
	for (i = 0; i<engine->stateNum; i++) {
		fsme_clearActionList(&engine->
			stateTable[i].entryAction);
		fsme_clearActionList(&engine->
			stateTable[i].exitAction);
	}

	//clear all transition actions
	for (i = 0; i<engine->transitionNum; i++) {
		fsme_clearActionList(&engine->
			transitionTable[i].action);
	}
}


/* -------------- Local Function Definitions -------------------- */
static void
fsmeEnterEngine(fsme_engine_ptr_t engine,
				const void* inContext,
				void* outContext)
{
#ifdef FSME_DEBUG
	fprintf(stdout, 
		"[FSME_DEBUG]: Entering engine(id=%d)... \n", 
		engine->id);
#endif

	/* execute entry action */
	fsme_processActions(engine, 
		fsmeEngineGetEntryAction(engine), 
		engine->id, 
		inContext, 
		outContext);

	/* Enter entry state */
	fsmeEnterState(engine, 
				fsmEngineGetEntryState(engine), 
				inContext, 
				outContext); 

#ifdef FSME_DEBUG
	fprintf(stdout, 
		"[FSME_DEBUG]: Engine(id=%d) entered. \n", 
		engine->id);
#endif
}


static void
fsmeExitEngine(fsme_engine_ptr_t engine, 
			   const void* inContext,
			   void* outContext)
{
#ifdef FSME_DEBUG
	fprintf(stdout, 
		"[FSME_DEBUG]: Exiting engine(id=%d)... \n", 
		engine->id);
#endif

	/* execute exit action */
	fsme_processActions(engine, 
		fsmeEngineGetExitAction(engine), 
		engine->id, 
		inContext, 
		outContext);

	/* set active state to NULL */
	fsmeEngineSetActiveState(engine, NULL);

#ifdef FSME_DEBUG
	fprintf(stdout, 
		"[FSME_DEBUG]: Engine(id=%d) exited. \n", 
		engine->id);
#endif
}



static void
fsmeEnterState(fsme_engine_ptr_t engine,
			   fsme_state_t const * targetState,
			   const void* inContext,
			   void* outContext)
{
#ifdef FSME_DEBUG
	fprintf(stdout, 
		"[FSME_DEBUG]: Entering state(id=%d)... \n", 
		targetState->id);
#endif
	
	//set current state
	fsmeEngineSetActiveState(engine, 
		(fsme_state_ptr_t)targetState);

    // execute entry actions
	fsme_processActions(engine, 
		fsmeStateGetAction(targetState, TRUE), 
		targetState->id, 
		inContext, 
		outContext);

     //If the state is asociated with a sub state machine, 
	 //then start the sub state machine.
	if (NULL != targetState->subEngine) {
		fsmeEnterEngine(targetState->subEngine, 
			inContext,
			outContext);
	}    

	//if the target state is the final state,
	//exit the engine
	if (fsmeStateIsFinal(targetState)) {
#ifdef FSME_DEBUG
		fprintf(stdout, 
			"[FSME_DEBUG]: State(id=%d) entered. \n", 
			targetState->id);
#endif
		fsmeExitEngine(engine, inContext, outContext);
	} else {
#ifdef FSME_DEBUG
		fprintf(stdout, 
			"[FSME_DEBUG]: State(id=%d) entered. \n", 
			targetState->id);
#endif
	}
}


static void
fsmeExitState(fsme_engine_ptr_t engine,
			  fsme_state_ptr_t  srcState,
			  const void* inContext,
			  void* outContext)
{
#ifdef FSME_DEBUG
	fprintf(stdout, 
		"[FSME_DEBUG]: Exiting state(id=%d)... \n", 
		srcState->id);
#endif

     //If the state is asociated with a sub state machine, 
	 //then exit the sub state machine.
	if (NULL != srcState->subEngine) {
		fsmeExitEngine(srcState->subEngine, 
			inContext, 
			outContext);
	}

    // execute exit actions
	fsme_processActions(engine, 
		fsmeStateGetAction(srcState, FALSE), 
		srcState->id, 
		inContext, 
		outContext);

#ifdef FSME_DEBUG
	fprintf(stdout, 
		"[FSME_DEBUG]: State(id=%d) exited. \n", 
		srcState->id);
#endif
}


static fsme_return_t
fsmeProcessTransition(fsme_engine_ptr_t engine,
					  fsme_transition_t* transition,
					  const void* inContext,
					  void* outContext)
{
    fsme_return_t retVal = FSME_OK;

	const fsme_state_ptr_t srcState = 
		fsmeTransitionGetSourceState(transition);
	const fsme_state_ptr_t tgtState = 
		fsmeTransitionGetTargetState(transition);
	
	if (fsmeTransitionHasGuard(transition)) {
		if (fsmeTransitionGetGuard(transition)(transition->id, 
		inContext, 
		outContext)) {
#ifdef FSME_DEBUG
		    fprintf(stdout, 
			    "[FSME_DEBUG]: Transition(id=%d) guard check succeeded. \n", 
			    transition->id);
#endif
        } else {
#ifdef FSME_DEBUG
		    fprintf(stdout, 
			    "[FSME_DEBUG]: Transition(id=%d) guard check failed. \n", 
			    transition->id);
#endif
     		retVal = FSME_TRANSITION_FAILURE;
            return retVal;
        }
	} 

	//exit the src state
	fsmeExitState(engine, srcState, inContext, outContext);
		
	//process transition actions
	fsme_processActions(engine, 
		fsmeTransitionGetAction(transition), 
		transition->id, 
		inContext, 
		outContext);

	//enter the target state
	fsmeEnterState(engine, tgtState, inContext, outContext);


	return retVal;
}


static void 
fsme_processActions(fsme_engine_ptr_t engine, 
					fsme_action_ptr_t actionNode, 
					int id, 
					const void* inContext, 
					void* outContext)
{
	engine->eventDisabled = TRUE;
	while (NULL != actionNode) {
		if (NULL != actionNode->action) {
			actionNode->action(id, 
				inContext, 
				outContext);
		}
		actionNode = actionNode->next;
	};
	engine->eventDisabled = FALSE;
}


static fsme_action_ptr_t
fsme_getLastAction(fsme_action_ptr_t headNode)
{
	fsme_action_ptr_t rtn = NULL;
	fsme_action_ptr_t tmpNode = NULL;

	if (NULL != headNode) {
		tmpNode = headNode;
		while (tmpNode)	{
			rtn = tmpNode;
			tmpNode = tmpNode->next;
		}
	}
	return rtn;
}


static fsme_action_ptr_t
fsme_createAction(fsme_func_t func)
{
	fsme_action_ptr_t newAction = NULL;

	if (NULL != func) {
		newAction = (fsme_action_ptr_t)
			malloc(sizeof(struct fsme_action));
		assert(newAction);

		newAction->action = func;
		newAction->next = NULL;
		}
	return newAction;
}


static void
fsme_addEngineAction(fsme_engine_ptr_t engine,
					 boolean wantEntryAction,
					 fsme_func_t action)
{
	fsme_action_ptr_t firstAction = NULL;
	
	if (NULL != engine && NULL != action) {
		firstAction = 
			fsmeEngineGetAction(engine, wantEntryAction);
		if (NULL != firstAction) {
			fsme_appendAction(firstAction, action);
		} else {
			if (wantEntryAction) 
			{
				engine->entryAction = 
					fsme_createAction(action);
			}
			else
			{
				engine->exitAction = 
					fsme_createAction(action);
			}
        }
	}
}


static void
fsme_addStateAction(fsme_engine_ptr_t engine,
					int stateId,
					boolean wantEntryAction,
					fsme_func_t action)
{
	fsme_state_t* state = NULL;
	fsme_action_ptr_t firstAction = NULL;
	
	if (NULL != engine && NULL != action) {
		state = fsmeGetStateById(engine, stateId);
		if (NULL == state) return;

		firstAction = fsmeStateGetAction(state, 
			wantEntryAction);
		if (NULL != firstAction) {
			fsme_appendAction(firstAction, action);
		} else {
			if (wantEntryAction) 
			{
			    state->entryAction = 
				    fsme_createAction(action);
            }
            else
            {
			    state->exitAction = 
				    fsme_createAction(action);
            }
		}
	}
}


static void
fsme_appendAction(fsme_action_ptr_t action, 
				  fsme_func_t func)
{
	fsme_action_ptr_t lastAction = NULL;
	fsme_action_ptr_t newAction = NULL;

	if (NULL != func && NULL != action) {
		lastAction = fsme_getLastAction(action);
		newAction = fsme_createAction(func);
		lastAction->next = newAction;
	}
}


static void
fsme_removeAction(fsme_action_ptr_t *head, 
				  fsme_func_t func)
{
	fsme_action_ptr_t preNode = NULL;
	fsme_action_ptr_t curNode = NULL;

	if (NULL != head && NULL != func) {
		curNode = *head;
		preNode = curNode;
		while (curNode) {
			if (curNode->action == func) {
				if (preNode == curNode) {
					//the first one is the matching one
					*head = curNode->next;
				} else {
					preNode->next = curNode->next;
				}
				free(curNode);
				break;
			};
			preNode = curNode;
			curNode = curNode->next;
		}
	}
}


static void
fsme_clearActionList(fsme_action_ptr_t *head)
{
	fsme_action_ptr_t curNode = NULL;
	fsme_action_ptr_t preNode = NULL;

	if (NULL != head) {
		curNode = *head;
		while (NULL != curNode)	{
			preNode = curNode;
			curNode = curNode->next;
			free(preNode);
		}
		*head = NULL;
	}
}

static fsme_state_t*
fsmeGetStateById(const fsme_engine_t* engine, 
				 int id)
{
	fsme_state_ptr_t en_state = NULL;
	int i;

	for (i = 0; i < engine->stateNum; i++) {
		en_state = &engine->stateTable[i];
		if (NULL != en_state && 
			en_state->id == id) 
			break;
	}
	return en_state;
}

static fsme_transition_t*
fsmeGetTransitionById(const fsme_engine_t* engine, 
					  int id)
{
	fsme_transition_t* en_transition = NULL;
	int i;
	for (i = 0; i < engine->transitionNum; i++)	{
		en_transition = &engine->transitionTable[i];
		if (NULL != en_transition && 
			en_transition->id == id)
			break;
	}
	return en_transition;
}


static fsme_engine_ptr_t
fsmeDoNewEngine(const fsm_machine_t* stateMachine, 
				fsme_engine_ptr_t parent)
{
	fsme_engine_ptr_t engine = NULL;
	const fsm_machine_t* subMachine = NULL;
	const fsm_state_t* tmpState = NULL;
	const fsm_transition_t* tmpTransition = NULL;
	int* triggerCountArray = NULL;
	int eventId = 0;
	int i = 0;

	if (NULL == stateMachine ||
		0 == stateMachine->stateNum ||
		0 == stateMachine->transitionNum ||
		0 == stateMachine->eventNum) {
		return NULL;
	}

	engine = (fsme_engine_t*)malloc(sizeof(fsme_engine_t));
	assert(engine);

	engine->id = stateMachine->id;
	engine->stateTable = NULL;
	engine->stateNum = 
		stateMachine->stateNum;
	engine->transitionTable = NULL;
	engine->transitionNum = 
		stateMachine->transitionNum;
	engine->triggerTable = NULL;
	engine->eventNum = 0;
	engine->parent = parent;
	engine->isSubStateMachine = FALSE;
	engine->entryState = NULL;
	engine->activeState = NULL;
	engine->entryAction = NULL;
	engine->exitAction = NULL;
	engine->eventDisabled = FALSE;


	//////////////////////////////
	//Create state table
	//////////////////////////////
	engine->stateTable = 
		(fsme_state_t*)malloc(sizeof(fsme_state_t) * 
		stateMachine->stateNum);
	assert(engine->stateTable);

	for (i = 0; i<stateMachine->stateNum; i++) {
		tmpState = &stateMachine->stateTable[i];
		engine->stateTable[i].id = 
			tmpState->id;
		engine->stateTable[i].isFinal = 
			tmpState->isFinal;
		engine->stateTable[i].entryAction = NULL; 
		engine->stateTable[i].exitAction = NULL;

		//If the state is a sub machine and it is
		//not a final state, create sub engine.
		subMachine = stateMachine->
			stateTable[i].subMachine;
		if (NULL != subMachine && 
			!(tmpState->isFinal)) {
			engine->stateTable[i].subEngine = 
				fsmeDoNewEngine(subMachine, engine);
		} else {
			engine->stateTable[i].subEngine = NULL;
		}
	};


	//////////////////////////////
	//Set engine entry state
	//////////////////////////////
	engine->entryState = fsmeGetStateById(engine, 
		stateMachine->entryStateId);


	//////////////////////////////
	//Create transition table
	//////////////////////////////
	engine->transitionTable = (fsme_transition_t*)
		malloc(sizeof(fsme_transition_t) * 
		stateMachine->transitionNum);
	assert(engine->transitionTable);

	for (i = 0; i<stateMachine->transitionNum; i++) {
		tmpTransition = &stateMachine->
			transitionTable[i];
		engine->transitionTable[i].id = tmpTransition->id;
		engine->transitionTable[i].action = NULL;
		engine->transitionTable[i].guard = NULL;
		engine->transitionTable[i].sourceState = 
			fsmeGetStateById(engine, 
				tmpTransition->sourceStateId);
		engine->transitionTable[i].targetState = 
			fsmeGetStateById(engine, 
				tmpTransition->targetStateId);
	};


	//////////////////////////////
	//Create trigger table
	//////////////////////////////
	engine->eventNum = stateMachine->eventNum;
	engine->triggerTable = (fsme_trigger_t*)
		malloc(sizeof(fsme_trigger_t) * 
		stateMachine->eventNum);
	assert(engine->triggerTable);

	//Init trigger table
	for (i = 0; i<stateMachine->eventNum; i++) {
		engine->triggerTable[i].eventId = i;
		engine->triggerTable[i].transitionNum = 0;
		engine->triggerTable[i].transitions = NULL;
	}
	
	//Calculate transition count for each trigger
	triggerCountArray = (int *)malloc(sizeof(int) * 
		stateMachine->eventNum);
	assert(triggerCountArray);
	memset(triggerCountArray, 0, 
		sizeof(int) * stateMachine->eventNum);
	for (i = 0; i<stateMachine->triggerNum; i++) {
		eventId = stateMachine->
			triggerTable[i].eventId;
		engine->triggerTable[eventId].transitionNum++;
		//Keep the count in triggerCountArray
		triggerCountArray[eventId]++;
	}

	//Create transition group table for each trigger
	for (i = 0; i<stateMachine->eventNum; i++) {
		if (0 == engine->triggerTable[i].transitionNum) 
			continue;
		engine->triggerTable[i].transitions = 
			(fsme_transition_t**)
			malloc(sizeof(fsme_transition_t*) * 
				engine->triggerTable[i].transitionNum);
		assert(engine->triggerTable[i].transitions);
	};

	//Set transition mappings for each trigger
	for (i = stateMachine->triggerNum-1; i>= 0; i--) {
		eventId = stateMachine->
			triggerTable[i].eventId;

		if (triggerCountArray[eventId] == 0) {
			//All transitions belonging to this trigger 
			//have been covered, so continue.
			continue; 
		}
		engine->triggerTable[eventId].
			transitions[triggerCountArray[eventId] - 1] = 
			fsmeGetTransitionById(engine, 
			stateMachine->triggerTable[i].transitionId);
		//One transition belonging to this trigger 
		//is put into the table as an entry.
		//Go to the next one.
		triggerCountArray[eventId]--;
	}	
	free(triggerCountArray);

	return engine;
}


