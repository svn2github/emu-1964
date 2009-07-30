/*$T profiler.c GC 1.136 02/28/02 08:12:37 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    This file contains functions to do CPU profiling, to calculate CPU usage among r4300i CPU core, video plugin, audio
    plugin, compiler, cpu idle. We may also support RSP and RDP later
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


/*
 * 1964 Copyright (C) 1999-2004 Joel Middendorf, <schibo@emulation64.com> This
 * program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. To contact the
 * authors: email: schibo@emulation64.com, rice1964@yahoo.com ?
 */
#include "stdafx.h"

char			*profiler_process_names[] = { "R4300i", "Video", "Audio", "Compiler", "Idle", "RSP", "RDP", "Netplay", "Kaillera" };
uint64			profiler_timer_count[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static int		process_being_profiling = R4300I_PROF;
static uint32	start_timer_h, stop_timer_h, start_timer_l, stop_timer_l;

BOOL			profiling_started = FALSE;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void start_profiling(int proc)
{
	if(profiling_started)
	{
		stop_profiling();
	}

	process_being_profiling = proc;

	/* start profiler */
	_asm
	{
		pushad
		rdtsc
		mov start_timer_h, edx	/* high DWORD */
		mov start_timer_l, eax	/* low DWORD */
		popad
	}

	profiling_started = TRUE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void stop_profiling(void)
{
	/* get current timer */
	_asm
	{
		pushad
		rdtsc
		mov stop_timer_h, edx	/* high DWORD */
		mov stop_timer_l, eax	/* low DWORD */
		popad
	}

	/* calculate timer elapse from the starting timer */
	profiler_timer_count[process_being_profiling] +=
		(
			(((uint64) (stop_timer_h - start_timer_h)) * 0x100000000) +
			stop_timer_l -
			start_timer_l
		);
	profiling_started = FALSE;
}

int get_current_profiling_process(void)
{
	return process_being_profiling;
}
/*
 =======================================================================================================================
 =======================================================================================================================
 */

unsigned int cpuIdlePercentages[4]={0,0,0,0};
int cpuIdlePercentageIdx=0;
void format_profiler_result_msg(char *msg)
{
	uint64	totaltimer = 0;
	int		i;

	for(i = 0; i < MAX_PROF; i++)
	{
		totaltimer += profiler_timer_count[i];
	}

    if(totaltimer == 0) totaltimer = 1;

	if( Kaillera_Is_Running )
	{
		sprintf
			(
			msg,
			"%s:%2.1f%% %s:%2.1f%% %s:%2.1f%% %s:%2.1f%% %s:%2.1f%% %s:%2.1f%% %s: %2.1f%%",
			TranslateStringByString("core"),
			(profiler_timer_count[R4300I_PROF] * 100.0f / totaltimer),
			TranslateStringByString("video"),
			(profiler_timer_count[VIDEO_PROF] * 100.0f / totaltimer),
			TranslateStringByString("audio"),
			(profiler_timer_count[AUDIO_PROF] * 100.0f / totaltimer),
			TranslateStringByString("compiler"),
			(profiler_timer_count[COMPILER_PROF] * 100.0f / totaltimer),
			TranslateStringByString("idle"),
			(profiler_timer_count[CPU_IDLE_PROF] * 100.0f / totaltimer),
			TranslateStringByString("netplay"),
			(profiler_timer_count[NETPLAY_PROF] * 100.0f / totaltimer),
			TranslateStringByString("kaillera"),
			(profiler_timer_count[KAILLERA_PROF] * 100.0f / totaltimer)
			);
	}
	else
	{
		sprintf
			(
			msg,
			"%s:%2.1f%% %s:%2.1f%% %s:%2.1f%% %s:%2.1f%% %s:%2.1f%%",
			TranslateStringByString("core"),
			(profiler_timer_count[R4300I_PROF] * 100.0f / totaltimer),
			TranslateStringByString("video"),
			(profiler_timer_count[VIDEO_PROF] * 100.0f / totaltimer),
			TranslateStringByString("audio"),
			(profiler_timer_count[AUDIO_PROF] * 100.0f / totaltimer),
			TranslateStringByString("compiler"),
			(profiler_timer_count[COMPILER_PROF] * 100.0f / totaltimer),
			TranslateStringByString("idle"),
			(profiler_timer_count[CPU_IDLE_PROF] * 100.0f / totaltimer)
			);
	}


	cpuIdlePercentageIdx++;
	cpuIdlePercentageIdx %= 4;
	cpuIdlePercentages[cpuIdlePercentageIdx] = (uint32)(((double)(__int64)profiler_timer_count[CPU_IDLE_PROF] * 100.0 / (double)(__int64)totaltimer)+0.5);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void reset_profiler(void)
{
	/*~~*/
	int i;
	/*~~*/

	for(i = 0; i < MAX_PROF; i++)
	{
		profiler_timer_count[i] = 0;
	}
}
