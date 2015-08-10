/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare and 
 *   the Reinforcement Learning and Artificial Intelligence Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  Defaults.cpp 
 *
 *  Defines methods for setting default parameters. 
 *
 **************************************************************************** */
#include "Defaults.hpp"

void setDefaultSettings(Settings &settings) {
    // General settings	


    // Controller settings
    settings.setString("game_controller", "internal");
    settings.setString("player_agent", "random_agent");
    settings.setInt("max_num_episodes", 1); // One episode per run
    settings.setInt("max_num_frames", 18000);
    settings.setInt("max_num_frames_per_episode", 0);
    settings.setInt("system_reset_steps", 4);

    // FIFO controller settings
    settings.setBool("run_length_encoding", true);

    // Environment customization settings
    settings.setBool("record_trajectory", false);
    //settings.setBool("restricted_action_set", false);
    settings.setBool("restricted_action_set", false);
    settings.setBool("use_starting_actions", true);
    settings.setBool("use_environment_distribution", false);
    //settings.setString("random_seed", "time");   
    settings.setString("random_seed", "0");   
    settings.setBool("disable_color_averaging", true);
    settings.setInt("frame_skip", 1);
    settings.setBool("normalize_rewards", true);
    
    // Display Settings
    settings.setBool("display_screen", false);

    // Search settings
    settings.setInt( "sim_steps_per_node", 5 ); // every 5 frames, at 60 frames per second, this means 1 action every 1/12 secs
    settings.setString("search_method", "brfs");

    // UCT settings
    settings.setInt("uct_monte_carlo_steps", -1); // DON'T KNOW WHAT THEY USE IT FOR. Number of times a node is visited in UCT? I cannot find what the 500 number means in Appendix C: simulations per action: 500. In the code they use the same limit as in FullTree: max_sim_steps_per_frame
    settings.setFloat("uct_exploration_constant", 0.1);
    settings.setFloat("uct_search_depth", 300);
    
    	// Agent settings
    	settings.setString("player_agent", "search_agent");
    	settings.setFloat("discount_factor", .995);
	//settings.setFloat("discount_factor", 0.995);

	// Max reward
	settings.setInt( "max_reward", 10000 );

	// Stop IW(1) on the first reward found
	//settings.setBool("iw1_stop_on_first_reward", true);
	//settings.setInt("iw1_reward_horizon", 50 );
	settings.setBool("iw1_stop_on_first_reward", false);
	settings.setInt("iw1_reward_horizon", 300 );	
	settings.setBool("randomize_successor_novelty", true );

	settings.setBool("novelty_boolean", false);
}
