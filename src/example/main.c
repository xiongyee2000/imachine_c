#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fsm.h"
#include "groupcall_machine.h"


#define IN_CONTEXT_VALUE 11
#define OUT_CONTEXT_VALUE 22

//-------------- ACTIONS -------------------//
void onStart_1(int id, const void* inContext, void* outContext)
{
	printf("#1: Engine Started. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onStart_2(int id, const void* inContext, void* outContext)
{
	printf("#2: Engine Started. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onShutdown_1(int id, const void* inContext, void* outContext)
{
	printf("#1: Engine Shut down. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onShutdown_2(int id, const void* inContext, void* outContext)
{
	printf("#2: Engine Shut down. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onInvite_1(int id, const void* inContext, void* outContext)
{
	printf("#1: Connecting ... \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onInvite_2(int id, const void* inContext, void* outContext)
{
	printf("#2: Connecting ...\n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onEnterConnected_1(int id, const void* inContext, void* outContext)
{
	printf("#1 Entered state 'CONNECTED'. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onEnterConnected_2(int id, const void* inContext, void* outContext)
{
	printf("#2 Entered state 'CONNECTED'. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onExitConnected_1(int id, const void* inContext, void* outContext)
{
	printf("#1 Exited state 'CONNECTED'. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}


void onExitConnected_2(int id, const void* inContext, void* outContext)
{
	printf("#2 Exited state 'CONNECTED'. \n");
	assert(IN_CONTEXT_VALUE == *((const int*)inContext));
	*((int *)outContext) = OUT_CONTEXT_VALUE;
}

boolean affiliationGuard(int id, const void* inContext, void* outContext)
{
    int in = 0;
	in = *((int *)inContext);
    if (in) {
        printf("#Affiliation guard check succeeded. \n");
    } else {
        printf("#Affiliation guard check failed. \n");
    }
    return (in ? TRUE : FALSE);
}


int main()
{
	int input = 0;
	int event = 0;
	int in = IN_CONTEXT_VALUE;
	int out = 0;
	fsme_engine_ptr_t groupcall_engine = fsme_newEngine(GROUPCALL_MACHINE);

	fsme_addMachineEntryAction(groupcall_engine, onStart_1);
	fsme_addMachineEntryAction(groupcall_engine, onStart_2);
	fsme_addMachineExitAction(groupcall_engine, onShutdown_1);
	fsme_addMachineExitAction(groupcall_engine, onShutdown_2);
	fsme_addTransitionAction(groupcall_engine, GROUPCALL_T_AFFILIATED_TO_CONNECTING, onInvite_1);
	fsme_addTransitionAction(groupcall_engine, GROUPCALL_T_AFFILIATED_TO_CONNECTING, onInvite_2);
	fsme_addStateEntryAction(groupcall_engine, GROUPCALL_S_CONNECTED, onEnterConnected_1);
	fsme_addStateEntryAction(groupcall_engine, GROUPCALL_S_CONNECTED, onEnterConnected_2);
	fsme_addStateExitAction(groupcall_engine, GROUPCALL_S_CONNECTED, onExitConnected_1);
	fsme_addStateExitAction(groupcall_engine, GROUPCALL_S_CONNECTED, onExitConnected_2);
    fsme_setGuard(groupcall_engine, GROUPCALL_T_AFFILIATING_TO_AFFILIATED, affiliationGuard);

	for (;;)
	{
		printf("\nGroupCall Engine>Make your choices:\n");
		printf("0-EXIT\n");
		printf("1-START MACHINE\n");
		printf("2-AFFILIATE\n");
		printf("3-AFFILIATED\n");
		printf("4-INVITE\n");
		printf("5-INVITE_ACCEPTED\n");
		printf("6-DISCONNECT\n");
		printf("7-DISCONNECTED\n");
		printf("9-SHUTDOWN\n");
		printf("100-LAUNCH_CALL\n");
		printf("101-INCOMING_CALL\n");
		printf("102-REQUEST_CONFIRMED\n");
		printf("103-IDLE\n");
		printf("104-END_CALL\n");
		scanf("%d", &input);
		getchar();


		if (0 == input)
		{
			break;
		}

		if (1 == input) 
		{
			fsme_startEngine(groupcall_engine, &in, &out);
			assert(out == OUT_CONTEXT_VALUE);
			continue;
		}

		if (9 == input)
		{
			fsme_shutdownEngine(groupcall_engine, &in, &out);
			assert(out == OUT_CONTEXT_VALUE);
			continue;
		}

		switch (input)
		{
			case 2:
				event = GROUPCALL_E_AFFILIATE_EVENT;
				break;
			case 3:
				event = GROUPCALL_E_AFFILIATED_EVENT;
				break;
			case 4:
				event = GROUPCALL_E_INVITE_EVENT;
				break;
			case 5:
				event = GROUPCALL_E_INVITE_ACCEPTED_EVENT;
				break;
			case 6:
				event = GROUPCALL_E_DISCONNECT_EVENT;
				break;
			case 7:
				event = GROUPCALL_E_DISCONNECTED_EVENT;
				break;
			case 100:
				event = CONNECTED_E_LAUNCH_CALL_EVENT;
				break;
			case 101:
				event = CONNECTED_E_INCOMING_CALL_EVENT;
				break;
			case 102:
				event = CONNECTED_E_REQUESTING_CONFIRMED_EVENT;
				break;
			case 103:
				event = CONNECTED_E_IDLE_EVENT;
				break;
			case 104:
				event = CONNECTED_E_END_CALL_EVENT;
				break;
			default:
				event = -1;
				break;
		};

		if (input < 100) {
			fsme_postEvent(groupcall_engine, event, &in, &out);
		} else {
			fsme_postEvent(fsme_getSubEngine(groupcall_engine, 4), event, &in, &out);
		}
		assert(out == OUT_CONTEXT_VALUE);
	}

	fsme_deleteEngine(groupcall_engine);
	
	return 0;
}
