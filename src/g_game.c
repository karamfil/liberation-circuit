#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>

#include "m_config.h"

#include "g_header.h"

#include "m_maths.h"
#include "g_world.h"
#include "e_slider.h"
#include "e_header.h"
#include "i_display.h"
#include "i_input.h"
#include "i_console.h"
#include "i_view.h"
#include "i_sysmenu.h"
#include "g_command.h"
#include "g_misc.h"
#include "g_motion.h"
#include "g_method.h"
#include "g_proc_run.h"
#include "g_proc_new.h"
#include "g_packet.h"
#include "g_cloud.h"
#include "m_globvars.h"
#include "m_input.h"
#include "c_header.h"
#include "e_editor.h"


//#include "s_turn.h"
#include "h_story.h"
#include "h_mission.h"
#include "t_template.h"
#include "x_sound.h"

#include "p_panels.h"
#include "p_init.h"

ALLEGRO_EVENT_QUEUE* event_queue; // these queues are initialised in main.c
ALLEGRO_EVENT_QUEUE* fps_queue;

extern struct view_struct view; // defined in i_view.c. TO DO: think about putting a pointer to this in the worldstruct
extern struct world_init_struct w_init; // this is the world_init generated by world setup menus - declared in s_menu.c
extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
struct game_struct game;
extern struct command_struct command;

void run_game(void);
void init_main_loop(void);
void main_game_loop(void);
void close_any_edit_window(void);
static int run_custom_game(void);
static int run_mission(void);
static void set_game_over(void);
static int run_game_over(void);

void start_game(void);

static void run_pregame(void);
static void update_vision_area(void);
//static void vision_block_check(struct block_struct* bl, int dist);
//static void vision_block_check(struct block_struct* bl, int base_pos, int* subblock_pos);
//static void vision_block_check_corner(struct block_struct* bl, int base_pos_x, int base_pos_y);



void start_game(void)
{


 game.phase = GAME_PHASE_PREGAME;
 game.spawn_fail = -1; // no failure yet.

 game.total_time = 255; // total time spent playing, *including* any time spent paused (w.world_time is actual in-game time). Will overflow after a bit more than 2 years of constant uptime without a restart, which is unlikely to occur in practice.
 game.pause_soft = 0;
// game.pause_hard = 0;
 game.fast_forward = FAST_FORWARD_OFF;
 game.fast_forward_type = FAST_FORWARD_TYPE_SMOOTH;
// game.force_turn_finish = 0;

 game.game_over_status = GAME_END_BASIC; // ignored until game.phase is set to GAME_PHASE_OVER
 game.game_over_value = 0;


	if (game.type == GAME_TYPE_MISSION)
  game.vision_mask = 1;
   else
    game.vision_mask = 0;

#ifdef DEBUG_MODE
    game.vision_mask = 0;
#endif



 game.user_player_index = 0;

 init_panels_for_new_game(); // currently this just sets panel element highlight times to zero

}

/*
Call this function before starting the main loop.
need_to_initialise is 1 if starting a new game or from a gamefile, 0 if loading from a saved game (as saved games contain various initialise stuff)
playing_mission is the index of the mission (MISSION_MISSION1 etc) if playing a mission, 0 otherwise
*/
void init_main_loop(void)
{

// settings.edit_window = EDIT_WINDOW_CLOSED;
// settings.keyboard_capture = INPUT_WORLD;
/*
 int i;

 for (i = 0; i < MODE_BUTTONS; i ++)
	{
		inter.mode_button_available [i] = 1;
	}
*/
// open_sysmenu();
 resize_display_window(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
 reset_panel_positions();
// init_ex_control();

 init_commands();

 clear_sound_list();
//fpr("\n iml: ai %i riai %i", game.area_index, game.region_in_area_index);
 if (game.type == GAME_TYPE_MISSION)
  reset_music(game.area_index, game.region_in_area_index, rand());
   else
    reset_music(game.area_index, -1, rand());

 flush_game_event_queues();

// game.play_sound = 0;
// game.play_sound_counter = 0;


}


// set need_to_initialise to zero if loading from disk (in which case the world has been initialised by the load function)
//  * currently need_to_initialise should always be 0
void run_game(void)
{

 init_main_loop();


 main_game_loop();

// ends when game over (or user quit)

 if (game.type == GAME_TYPE_MISSION)
	{
		w.players = 1; // this affects the template panel (prevents opening the old player 1 templates in the mission select screen)

		if (game.game_over_status == GAME_END_MISSION_COMPLETE)
	 {
 		story_mission_defeated();
 	}
	}

	close_all_panels();

 deallocate_world(); // must be called at end of game (it frees memory allocated by init_main_loop())

 reset_music(1, 0, rand());

}


void main_game_loop(void)
{

 ALLEGRO_EVENT ev;

 int fps = 0;
 int cps = 0;
 int force_display_update = 0; // display always runs at least once each second
 int playing = 1;

 view.fps = 0;
 view.cycles_per_second = 0;


 int skip_frame = 0; // if game is running too slowly, it will skip frames to save drawing time.
// if (game.phase == GAME_PHASE_PREGAME)
//  skip_frame = 1; // display is not updated during pregame phase.

 do // main game loop
 {
//  if (game.phase != GAME_PHASE_PREGAME) // no input in pregame phase
   get_ex_control(0, (game.pause_soft == 0)); // ex_control needs to be updated even when halted (control will not be updated)

   run_input();

  	run_panels();

   run_editor();
/*
// should probably run display at the end of the loop rather than the start
//  - so that the game has run through at least once when loading game
  if (!skip_frame || force_display_update)
  {
   run_display();
   fps ++;
   force_display_update = 0;
  }
*/
  game.total_time++;

//  if (game.pause_hard == 0)
//  {


//   if (view.focus_proc != NULL
//    && view.focus_proc->exists != 1) // will be -1 if proc has just been deallocated
//   {
//    view.focus_proc = NULL;
  //  reset_proc_box_height(NULL);
//   } // need to find a better place to put this

   if (game.phase == GAME_PHASE_WORLD
				|| game.phase == GAME_PHASE_OVER) // game continues to run while over.
			{
				if (game.pause_soft == 0)
    {
//    if (run_turns()) // returns 0 if game has left world phase (e.g. because time has run out), 1 if still in world phase
     {
      run_world(); // runs the world and also the mission, if this is a mission. Can end the game.
// should run_world be after the next three function calls? Maybe.

//      run_clouds(); clouds don't need to be run
      run_fragments();
      run_cores_and_procs();
      run_packets();

      cps ++;
      w.world_time ++;
      w.world_seconds = (w.world_time - BASE_WORLD_TIME) / 60;

      update_vision_area(); // update fog of war after w.world_time is incremented so that the vision_time timestamps are up to date

      if (game.phase != GAME_PHASE_OVER)
						{
       if (game.type == GAME_TYPE_BASIC)
							 run_custom_game();
						   else
									 run_mission(); // for now ignore return values
						}

      play_sound_list();

     }
    }
   }

   if (game.phase == GAME_PHASE_OVER)
			{
					game.fast_forward = 0;
     if (run_game_over())
						break; // user clicked game over button
			}

   if (game.phase == GAME_PHASE_PREGAME)
			{
					game.fast_forward = 0;
     run_pregame();
			}

//   w.total_time ++; // total_time is affected by hard pause, but unlike world_time is not affected by soft pause (so it can only be used for things that don't affect gameplay, like console line highlighting)

//   run_input(); // this derives control (available to programs) from ex_control. doesn't need to be run for user to be able to use mode buttons and interact with sysmenu/editor/templates etc (as these use ex_control directly)

   if (game.phase == GAME_PHASE_FORCE_QUIT)
				break; // can be set in run_input() if user clicks on quit in system menu

   run_commands();

   view.just_resized = 0;

// fps_queue generates an event once each second - used for calculating fps and forcing a display update at least once per second
  if (al_get_next_event(fps_queue, &ev))
  {
   view.fps = fps;
   fps = 0;
   view.cycles_per_second = cps;
   cps = 0;
//   if (game.phase != GAME_PHASE_PREGAME)
//    force_display_update = 1; // this is checked next time through this loop (see display call above)
  }

//  if (!skip_frame)
  {
//  	run_panels();

//     run_editor();
// may need to set playing to 0 here if player quits using sysmenu

/*   switch(settings.edit_window)
   {
    case EDIT_WINDOW_EDITOR:
     run_editor(); break;
    case EDIT_WINDOW_TEMPLATES:
     run_templates(); break;
    case EDIT_WINDOW_SYSMENU:
     playing = run_sysmenu(); break;
//    case EDIT_WINDOW_PROGRAMS:
//     run_programs_menu(); break;
   }*/
  }

  if (playing == 0)
   break;

  run_consoles(); // I think it's okay to call this even when halted

  skip_frame = 0;

// check for fast-forward (skip). Ignore FF if not in world, or if paused
  if (game.fast_forward > 0
   && game.phase == GAME_PHASE_WORLD // let's not allow ff in GAME_PHASE_OVER
   && game.pause_soft == 0)
//   && game.pause_hard == 0)
   {
    switch (game.fast_forward_type)
    {
     case FAST_FORWARD_TYPE_NO_DISPLAY:
      if (game.fast_forward == FAST_FORWARD_JUST_STARTED)
       game.fast_forward = FAST_FORWARD_ON; // don't skip this frame (to prevent a delay before "FAST FORWARD" message appears on screen)
        else
								{
/*									if (fps == 0)
									{
          fps = 1;
									}
									 else*/
           skip_frame = 1;
//           else
								}
      break;

     case FAST_FORWARD_TYPE_SMOOTH:
      game.fast_forward = FAST_FORWARD_ON;
      al_flush_event_queue(event_queue);
      break;
     case FAST_FORWARD_TYPE_SKIP:
      game.fast_forward = FAST_FORWARD_ON;
      if (w.world_time % 8 != 0)
							skip_frame = 1;
      break;
/*     case FAST_FORWARD_TYPE_8X:
      game.fast_forward = FAST_FORWARD_ON;
      if (w.world_time % 8 != 0)
							skip_frame = 1;
      break;*/
    }
   }

// now check whether the timer has expired during game processing. If it has, don't generate a display this tick (unless force_display_update==1)
  if (al_get_next_event(event_queue, &ev))
  {
//   switch(ev.type)
//   {
//    case ALLEGRO_EVENT_TIMER: this should be the only possible type.
     skip_frame = 1;
//     break;
//   }
//   al_flush_event_queue(event_queue);
  }

  if (!skip_frame || force_display_update)
  {
   run_display();
   fps ++;
   force_display_update = 0;
  }


// wait for the timer so we can go to the next tick (unless we're fast-forwarding or the timer has already expired)
  if (!skip_frame
   && (game.fast_forward == FAST_FORWARD_OFF
				|| game.pause_soft)) // don't skip frames if paused, even if fast-forwarding
  {
   al_wait_for_event(event_queue, &ev);
//   al_flush_event_queue(event_queue);
  }

 } while (TRUE); // end main game loop

// if game has been finished or quit, we can end up here outside the loop

}

/*
// closes editor, template window or system window, if open
// see also mode_button() in m_input.c
void close_any_edit_window(void)
{

 switch(settings.edit_window)
 {
  case EDIT_WINDOW_CLOSED:
   break;
  case EDIT_WINDOW_EDITOR:
   close_editor();
   resize_display_window(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
   break;
  case EDIT_WINDOW_TEMPLATES:
   close_templates();
   resize_display_window(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
   break;
  case EDIT_WINDOW_SYSMENU:
   close_sysmenu();
   resize_display_window(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
   break;
  case EDIT_WINDOW_PROGRAMS:
//   close_programs_window();
   resize_display_window(settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);
   break;
 }

}
*/

static void run_pregame(void)
{

 int i,j;
 struct template_struct* spawn_templ;


// make a visible area around player's spawn position
//  - actually this probably isn't necessary as I don't think it does anything in pregame mode (it doesn't affect the display, which is processed separately)
//    but it can't really hurt.
	int min_x, max_x, min_y, max_y;

	min_x = fixed_to_block(w.player[game.user_player_index].spawn_position.x) - VISION_BLOCKS;
	if (min_x < 1)
		min_x = 1;
	min_y = fixed_to_block(w.player[game.user_player_index].spawn_position.y) - VISION_BLOCKS;
	if (min_y < 1)
		min_y = 1;
	max_x = fixed_to_block(w.player[game.user_player_index].spawn_position.x) + VISION_BLOCKS;
	if (max_x >= w.blocks.x - 2)
		max_x = w.blocks.x - 2;
	max_y = fixed_to_block(w.player[game.user_player_index].spawn_position.y) + VISION_BLOCKS;
	if (max_y >= w.blocks.y - 2)
		max_y = w.blocks.y - 2;


		for (j = min_y; j < max_y;	j ++)
		{
		 for (i = min_x; i < max_x;	i ++)
		 {

		   w.vision_area[game.user_player_index][i][j].vision_time = w.world_time;

		 }
		}



//    if (game.phase == GAME_PHASE_PREGAME)
//     run_pregame();
 if (control.mouse_panel == PANEL_MAIN
		&& control.mouse_x_screen_pixels > view.window_x_unzoomed / 2 - 180
	 && control.mouse_x_screen_pixels < view.window_x_unzoomed / 2 + 180
	 && control.mouse_y_screen_pixels > view.window_y_unzoomed / 2 - 60
	 && control.mouse_y_screen_pixels < view.window_y_unzoomed / 2 + 30
// for button dimensions, see also code in i_
		&& control.mbutton_press [0] == BUTTON_JUST_PRESSED)
	{
  for (i = 0; i < w.players; i ++)
		{
			spawn_templ = &templ[i][0];
			if (spawn_templ->active == 0
				|| !lock_template(spawn_templ))
			{
				game.spawn_fail = i;
				game.spawn_fail_reason = SPAWN_FAIL_LOCK;
				return;
			}
			if (spawn_templ->data_cost > w.player[i].data)
			{
				game.spawn_fail = i;
				game.spawn_fail_reason = SPAWN_FAIL_DATA;
				return;
			}
		}


// Now we start (shouldn't be possible to fail after this)
		game.phase = GAME_PHASE_WORLD;

		struct core_struct* unused_collided_core;

		for (i = 0; i < w.players; i ++)
		{
// At this point we *should* be able to assume that the process can be created:
 	 int new_process_index = create_new_from_template(&templ[i][0], i, w.player[i].spawn_position, int_angle_to_fixed(w.player[i].spawn_angle), &unused_collided_core);

 	 w.player[i].data -= templ[i][0].data_cost;

 	 w.core[new_process_index].next_execution_timestamp = w.world_time + 15 + (new_process_index & (EXECUTION_COUNT - 1)); // to avoid the delay for a newly built process
 	 w.core[new_process_index].construction_complete_timestamp = w.core[new_process_index].next_execution_timestamp;

		}

 	if (game.type == GAME_TYPE_MISSION)
			mission_spawn_extra_processes();

// make sure the click on the start game button doesn't also select the first process (messes up the tutorial):
  control.mbutton_press [0] = BUTTON_HELD;

	}

}

#define VISION_AREA_MAP_SIZE (SCAN_RANGE_BASE_BLOCKS * 2 + 1)

// lookup table for the shape of an octagonal vision area
int octagonal_min_max [VISION_AREA_MAP_SIZE] [2];


//int vision_area_map [VISION_AREA_MAP_SIZE] [VISION_AREA_MAP_SIZE];

void init_vision_area_map(void)
{
	int i, j;

	al_fixed centre_fixed = (BLOCK_SIZE_FIXED * VISION_AREA_MAP_SIZE) / 2;

	for (i = 0; i < VISION_AREA_MAP_SIZE; i ++)
	{
		octagonal_min_max [i] [0] = 0 - (SCAN_RANGE_BASE_BLOCKS + 1);
		octagonal_min_max [i] [1] = SCAN_RANGE_BASE_BLOCKS + 1;
		for (j = 0; j < VISION_AREA_MAP_SIZE; j ++)
		{
   al_fixed dist = distance_oct_xyxy(centre_fixed, centre_fixed, i * BLOCK_SIZE_FIXED + (BLOCK_SIZE_FIXED / 2), j * BLOCK_SIZE_FIXED + (BLOCK_SIZE_FIXED / 2));
   if (dist > SCAN_RANGE_BASE_FIXED)
			{
				if (j < SCAN_RANGE_BASE_BLOCKS)
					octagonal_min_max [i] [0] = j - SCAN_RANGE_BASE_BLOCKS;
				  else
						{
							if (octagonal_min_max [i] [1] > j - SCAN_RANGE_BASE_BLOCKS)
								octagonal_min_max [i] [1] = j - SCAN_RANGE_BASE_BLOCKS;
						}
			}

		}
	}
/*
	for (i = 0; i < VISION_AREA_MAP_SIZE; i++)
	{
		fpr("\n oct_mm %i, %i", octagonal_min_max [i] [0], octagonal_min_max [i] [1]);
	}*/


}

static void update_vision_area(void)
{
	struct core_struct* core;
	int c;
	int base_min_x, base_max_x, base_min_y, base_max_y;
	int min_x, max_x, min_y, max_y;
//	int clipped_left, clipped_right, clipped_top, clipped_bottom;
	int i,j;

 int p;

for (p = 0; p < w.players; p ++)
{
	for (c = w.player[p].core_index_start; c < w.player[p].core_index_end; c ++)
	{
		if (w.core[c].exists <= 0
			&& w.core[c].destroyed_timestamp < w.world_time - DEALLOCATE_COUNTER) // this means deallocating cores are still seen for a little while
			continue;
		core = &w.core[c];

		int base_x = w.proc[core->process_index].block_position.x;
		int base_y = w.proc[core->process_index].block_position.y;

		base_min_x = base_x - SCAN_RANGE_BASE_BLOCKS;
		base_min_y = base_y - SCAN_RANGE_BASE_BLOCKS;
		base_max_x = base_x + SCAN_RANGE_BASE_BLOCKS + 1;
		base_max_y = base_y + SCAN_RANGE_BASE_BLOCKS + 1;

		if (base_min_x < 0)
			min_x = 0;
			 else
					min_x = base_min_x;
	 if (base_min_y < 0)
			min_y = 0;
			 else
					min_y = base_min_y;
		if (base_max_x > w.blocks.x)
			max_x = w.blocks.x;
			 else
  			max_x = base_max_x;
		if (base_max_y > w.blocks.y)
			max_y = w.blocks.y;
			 else
  			max_y = base_max_y;


		for (j = min_y; j < max_y;	j ++)
		{
			int column_offset = j - base_y + SCAN_RANGE_BASE_BLOCKS;
			int row_min_x = min_x;
			int row_max_x = max_x;
			if (row_min_x < base_x + octagonal_min_max [column_offset] [0])
				row_min_x = base_x + octagonal_min_max [column_offset] [0];
			if (row_max_x > base_x + octagonal_min_max [column_offset] [1])
				row_max_x = base_x + octagonal_min_max [column_offset] [1];
//			fpr("\n j %i min_y %i max_y %i column_offset %i min_x %i max_x %i row_min_x %i row_max_x %i", j, min_y, max_y, column_offset, min_x, max_x, row_min_x, row_max_x);
		 for (i = row_min_x; i < row_max_x;	i ++)
		 {

		   w.vision_area[p][i][j].vision_time = w.world_time;

		 }
		}
	} // end for c
} // end for p

}


extern struct mission_state_struct mission_state; // just need one struct for the current mission

enum
{
// phases for mission 1
MPHASE1_START,
MPHASE1_SELECT,
MPHASE1_BUILD,
MPHASE1_FIND_WELL,
MPHASE1_BUILD_ATTACKERS,
MPHASE1_ATTACK,
MPHASE1_ATTACKING
};


enum
{
// phases for mission 2
MPHASE2_START,
MPHASE2_SELECT,
MPHASE2_BUILD,
MPHASE2_FIND_WELL,
MPHASE2_BUILD_ATTACKERS,
MPHASE2_BUILD_ATTACKERS2,
MPHASE2_ATTACK,
MPHASE2_ATTACKING
};

static void write_mission_text(int col, char* mtext);

// runs the current mission - called each tick. Don't call in non-mission mode.
// Returns 1 if the game continues, 0 if game over (although this is currently ignored and the game over status is set by this function)
static int run_mission(void)
{

// int i;

// special mission-specific code:
 switch(game.mission_index)
 {
 	case MISSION_TUTORIAL1:
 		switch(mission_state.phase)
 		{
			 case MPHASE1_START:
   	 clear_console(CONSOLE_SYSTEM);
//   	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
			 	write_mission_text(PRINT_COL_WHITE,
/*"Program initialised. Spawning process 0.\n\*/
"\nWelcome!\n\
To begin, select the blue base process near the centre of the display (by left-clicking on it).");

			 	write_mission_text(PRINT_COL_LBLUE,
"\n\n(For information about controls, open the system panel at any time by clicking on the Sy button in the top right of the display).");

    	mission_state.phase = MPHASE1_SELECT;
    	break;
				case MPHASE1_SELECT:
					if (command.selected_core [0] == 0) // can assume builder is process 0
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"This process is a builder/harvester base.\
\nIt harvests raw data from the nearby data well and allocates it. Then it uses allocated data to \
build new processes.\
\n\nBut the data from a single well won't be enough! Build a mobile harvester by clicking on the \"harvester\" button on the left of the display.");

    	 mission_state.phase = MPHASE1_BUILD;
					}
					break;
				case MPHASE1_BUILD:
					if (w.core[1].exists > 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"The harvester will take a few seconds to finish constructing.\
\nSelect it and tell it to move near a different data well. Data wells are shown in yellow on the map at the bottom right of the display.");
// \n\nSelect it and tell it to move near a different data well. Data wells are shown in yellow on the map at the bottom right of the display.");
			 	write_mission_text(PRINT_COL_LBLUE,
"\n\n(Give movement commands by right-clicking on the display or the map. You don't need to click exactly on the well, just nearby.)");
    	 mission_state.phase = MPHASE1_FIND_WELL;
					}
					break;
				case MPHASE1_FIND_WELL:
					if (distance_oct_xyxy(w.core[1].core_position.x, w.core[1].core_position.y,
																											w.data_well[1].position.x, w.data_well[1].position.y) < al_itofix(600)
						|| distance_oct_xyxy(w.core[1].core_position.x, w.core[1].core_position.y,
																											w.data_well[2].position.x, w.data_well[2].position.y) < al_itofix(600))
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"You can leave the harvester alone, and it will gather data to take back to the base.\
\n\nNow find the base again (you can click on the map to move the display around) and tell it to build an \"attacker\" process.");
    	 mission_state.phase = MPHASE1_BUILD_ATTACKERS;
					}
					break;
				case MPHASE1_BUILD_ATTACKERS:
					if (w.core[2].exists > 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"Now you have an attacker. But you'll need more than one! Tell the base to build more attackers - five or so should be enough. (If you've built something else by mistake, just tell the base to start building attackers.)");
 			 	write_mission_text(PRINT_COL_LBLUE,
"\n\nBuild commands queue automatically.\nYou can also hold shift while clicking the \"attacker\" button to build multiple attackers.");
    	 mission_state.phase = MPHASE1_ATTACK;
					}
					break;
				case MPHASE1_ATTACK:
					if (w.core[6].exists > 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"When you have a reasonably large attack fleet, seek out the enemy and defeat them! (Hint: try searching to the right)");
 			 	write_mission_text(PRINT_COL_LBLUE,
"\n\nDrag a box around several processes to select them all.\
\nDouble-clicking on a process selects all visible processes of the same type.\
\nHolding control while giving a movement command tells a process to attack anything it meets on the way.");
    	 mission_state.phase = MPHASE1_ATTACKING;
					}
					break;
				case MPHASE1_ATTACKING:
					if (w.player[1].processes == 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"Well done! Mission complete.");
					 set_game_over();
					 game.game_over_status = GAME_END_MISSION_COMPLETE;
					 return 0;
					}
					break;

 		}
 		break; // end for mission 1




		case MISSION_TUTORIAL2:
 		switch(mission_state.phase)
 		{
			 case MPHASE2_START:
   	 clear_console(CONSOLE_SYSTEM);
//   	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
			 	write_mission_text(PRINT_COL_WHITE,
/*"Program initialised. Spawning process 0.\n\*/
"\nMission 1 showed how to harvest from data wells and build a fleet. Now let's try expanding.\
\n\nTo begin, select your base process.");

			 	write_mission_text(PRINT_COL_LBLUE,
"\n\n(Remember: open the system panel, by clicking on the Sy button in the top right of the display, for information about controls. Or to quit.)");

    	mission_state.phase = MPHASE2_SELECT;
    	break;
				case MPHASE2_SELECT:
					if (command.selected_core [0] == 0) // can assume builder is process 0
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"Your base is static (it can't move), so to expand you'll need a mobile builder process.\
\n\nBuild a mobile builder process by clicking on the \"mbuild\" button on the left of the display.\n");

    	 mission_state.phase = MPHASE2_BUILD;
					}
					break;
				case MPHASE2_BUILD:
					if (w.core[1].exists > 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"The best place to build a new base is usually near a different data well, to gain control of that well's data.");
// 			 	write_mission_text(PRINT_COL_LBLUE,
//"\n(Only some of a well's data is available to harvest at any time. Available data is gradually replenished from reserves.)");
 			 	write_mission_text(PRINT_COL_WHITE,
"\n\nTell the mobile builder to move to one of the other wells. It will follow your command when it's finished constructing.");
    	 mission_state.phase = MPHASE2_FIND_WELL;
					}
					break;
				case MPHASE2_FIND_WELL:
					if (distance_oct_xyxy(w.core[1].core_position.x, w.core[1].core_position.y,
																											w.data_well[1].position.x, w.data_well[1].position.y) < al_itofix(600)
						|| distance_oct_xyxy(w.core[1].core_position.x, w.core[1].core_position.y,
																											w.data_well[2].position.x, w.data_well[2].position.y) < al_itofix(600))
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"Now tell the mbuild process to build a new base near the edge of the data well.\
\n\nA static (immobile) process like the base can't be built too close to a data well, but it can't be too far away or its harvester won't work.");
 			 	write_mission_text(PRINT_COL_LBLUE,
"\n\nLeft-click to place the new base. Right-click to cancel.");
    	 mission_state.phase = MPHASE2_BUILD_ATTACKERS;
					}
					break;
				case MPHASE2_BUILD_ATTACKERS:
					if (w.core[2].exists > 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"Now you should have two bases capable of harvesting data and building new processes.\
\nIt's time for a new fleet! Start building attackers, destroyers and commanders.\
\n(you could also build a harvester to send to the other data well).");
 			 	write_mission_text(PRINT_COL_LBLUE,
"\n\nHold shift to repeat-build, or control to go to the front of the queue. The queue buttons (above the build command buttons) let you re-order (by dragging) or cancel queued processes.");
    	 mission_state.phase = MPHASE2_BUILD_ATTACKERS2;
					}
					break;
				case MPHASE2_BUILD_ATTACKERS2:
					if (w.core[3].exists > 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"You'll need a reasonably large fleet for this mission. Keep building!");
 			 	write_mission_text(PRINT_COL_LGREEN,
"\nAttackers are small, fast and cheap.\
\nDestroyers are designed to attack large targets.\
\nCommanders are large and have strong defences.");

 			 	write_mission_text(PRINT_COL_WHITE,
"\nOr you can use the designer to design your own units! (click on the De button in the top right of the screen; the designer has help buttons that tell you how to use it.)");
    	 mission_state.phase = MPHASE2_ATTACK;
					}
					break;
				case MPHASE2_ATTACK:
					if (w.core[7].exists > 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"When you have a large fleet of processes, seek out the enemy and defeat them!");
 			 	write_mission_text(PRINT_COL_LBLUE,
"\n\nYou can set waypoints by holding shift.\
\nAlso, to help your fleet move together you can tell fast units to guard slow units by selecting the small units then right-clicking on the large ones.");
    	 mission_state.phase = MPHASE2_ATTACKING;
					}
					break;
				case MPHASE2_ATTACKING:
					if (w.player[1].processes == 0)
					{
    	 clear_console(CONSOLE_SYSTEM);
    	 play_interface_sound(SAMPLE_BLIP3, TONE_2C);
 			 	write_mission_text(PRINT_COL_WHITE,
"Well done! Mission complete.\
\n\nYou've finished the tutorial and completed initialisation.");
					 set_game_over();
					 game.game_over_status = GAME_END_MISSION_COMPLETE;
					 return 0;
					}
					break;


 		}
/*
Plan for mission 2 tutorial:

hello!
this mission is about building etc...

select base ->

need to expand
build mbuild

built ->

move to data well

moved ->

build base

built ->

now build attackers!
destroy =
large =
or design your own

build ->

now attack


*/


					break; // end for mission 2


				default:
					if ((game.story_type == STORY_TYPE_HARD
						 || game.story_type == STORY_TYPE_ADVANCED_HARD)
						&& (w.world_time & 31) == 0)
					  w.player[1].data ++;
					break;

 }



// General victory conditions
/* switch(game.mission_index) - don't switch; these apply in all cases (at least for now)
 {
	 default:
	 	{*/
		if (game.phase == GAME_PHASE_WORLD) // if already GAME_OVER, don't check for other game end states
		{
	 		if (w.player[0].processes <= 0)
				{
					set_game_over();
					game.game_over_status = GAME_END_MISSION_FAILED;
					goto finished_game_over;
				}
	 		if (w.player[1].processes <= 0)
				{
					set_game_over();
					game.game_over_status = GAME_END_MISSION_COMPLETE;
					goto finished_game_over;
				}
	   if (w.world_seconds >= 3600) // 1 hour time limit
	   {
		   set_game_over();
		   game.game_over_status = GAME_END_MISSION_FAILED_TIME;
					goto finished_game_over;
	   }
		}

		finished_game_over:
			{

	   int i;

	   if ((w.world_time & 63) == 0)
				{
// to prevent annoying endgames where you have to hunt down a single harvester or something,
//  remove the vision mask if p1 has no static processes.
	    int found_static = 0;
	    for (i = w.player[1].core_index_start; i < w.player[1].core_index_end; i ++)
				 {
					 if (w.core[i].exists > 0
						 && !w.core[i].mobile)
						{
								found_static = 1;
								mission_state.reveal_player1 = 0;
								break;
						}
				 }
				 if (!found_static)
					{
						mission_state.reveal_player1 = 1;
					}
				}
			}
/*	   }
	 	}
	 	break;



 }*/

 return 1;

}


int mtext_pos, write_pos;
static int mtext_line_wraps(int col, char* mtext, char* write_text);

// mtext can be as long as needed.
static void write_mission_text(int col, char* mtext)
{
	mtext_pos = 0;
	write_pos = 0;

	char write_text [SYSTEM_CONSOLE_WIDTH_LETTERS + 4]; // the 4 is just a safe buffer (could probably be 1)
	write_text [0] = 0;

	while(TRUE)
	{
		write_text [write_pos] = mtext [mtext_pos];
		write_text [write_pos + 1] = 0;

		switch(mtext [mtext_pos])
		{
		 case 0: // end
		 	if (!mtext_line_wraps(col, mtext, write_text))
 			{
 				if (write_pos > 0)
    	 write_text_to_console(CONSOLE_SYSTEM, col, -1, 0, write_text);
		 	 return;
				}
 // if the line wrapped, mtext_pos and write_pos will have been pushed back to valid text, so we can continue printing the last word.
		 	break;
			case '\n': // end of line
		 	if (mtext_line_wraps(col, mtext, write_text))
		 	 break;
   	write_text [write_pos] = 0;
   	write_text_to_console(CONSOLE_SYSTEM, col, -1, 0, write_text);
   	console_newline(CONSOLE_SYSTEM, col);
   	write_pos = 0;
   	write_text [0] = 0;
				mtext_pos ++;
   	break;
			default:
		 	if (mtext_line_wraps(col, mtext, write_text))
				{
// 				mtext_pos ++;
		 	 break;
				}
				write_pos ++;
				mtext_pos ++;
				break;

		}

	}

}

static int mtext_line_wraps(int col, char* mtext, char* write_text)
{

	if (write_pos >= SYSTEM_CONSOLE_WIDTH_LETTERS - 3)
	{
		while(mtext [mtext_pos] != ' ')
		{
			write_pos --;
			mtext_pos --;
#ifdef SANITY_CHECK
if (write_pos < 0 || mtext_pos < 0)
{
	fpr("\n Error: g_game.c: mtext_line_wraps(): block of characters too long in mission text?");
	error_call();
// this should never happen in practice unless the mission text arrays are broken somehow (which should also never happen)
}
#endif

		}
// found space, so terminate line, write line to console and return:
  write_text [write_pos] = 0;
//  write_text [write_pos + 1] = 0;
 	write_text_to_console(CONSOLE_SYSTEM, col, -1, 0, write_text);
 	console_newline(CONSOLE_SYSTEM, col);
 	write_pos = 0;
 	write_text [0] = 0;
 	mtext_pos ++;
 	return 1;
	}

	return 0; // doesn't wrap so just keep writing, or finish

}

// runs a game set up by the user - called each tick. Don't call in mission mode.
// Returns 1 if the game continues, 0 if game over
static int run_custom_game(void)
{

	int player_index;

	int players_left = w.players;
	int last_player = 0;

	for (player_index = 0; player_index < w.players; player_index ++)
	{
		if (w.player[player_index].processes <= 0) // really just 0
			players_left --;
			 else
				 last_player = player_index; // if there's only one player list, this will be that player
	}

	if (players_left <= 1)
	{
		set_game_over();
		if (players_left == 0) // all players must have been eliminated at the same time
		{
			game.game_over_status = GAME_END_DRAW; // ignores last_player value
			return 0;
		}
// must be exactly one player left
  game.game_over_status = GAME_END_PLAYER_WON;
  game.game_over_value = last_player;
  return 0;
	}

	if (w.world_seconds >= 3600) // maximum game length 1 hour
	{
		set_game_over();
		game.game_over_status = GAME_END_DRAW_OUT_OF_TIME;
		return 0;
	}

 return 1;

}

static void set_game_over(void)
{

  game.phase = GAME_PHASE_OVER; // details of game over status should be set by calling function

	 game.fast_forward = 0;
  game.pause_soft = 0;
//  game.pause_hard = 0;

  game.game_over_time = 0;

  turn_music_off(); // it will start again later

}


// returns 1 if player clicked quit, 0 otherwise
static int run_game_over(void)
{

 game.game_over_time ++;

 if (game.game_over_time > 60
		&& control.mouse_panel == PANEL_MAIN
		&& control.mouse_x_screen_pixels > view.window_x_unzoomed / 2 - 180
	 && control.mouse_x_screen_pixels < view.window_x_unzoomed / 2 + 180
	 && control.mouse_y_screen_pixels > view.window_y_unzoomed / 2 - 60
	 && control.mouse_y_screen_pixels < view.window_y_unzoomed / 2 + 30
// for button dimensions, see also code in i_
		&& control.mbutton_press [0] == BUTTON_JUST_PRESSED)
	{
  return 1;
	}

 return 0;

}

