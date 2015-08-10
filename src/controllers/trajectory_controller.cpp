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
 *  trajectory_controller.hpp
 *
 *  The TrajectoryController class allows users to directly interface with ALE.
 *
 **************************************************************************** */

#include "trajectory_controller.hpp"
#include "time.hxx"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>

TrajectoryController::TrajectoryController(OSystem* osystem):
  ALEController(osystem),
  m_max_num_frames(0),
  m_episode_score(0),
  m_episode_number(0)
 {

  m_max_num_frames = m_osystem->settings().getInt("max_num_frames");
  m_max_num_episodes = m_osystem->settings().getInt("max_num_episodes");
  
  string trajectory_filename = m_osystem->settings().getString("state_trajectory_filename", true);
  
  std::ifstream input( trajectory_filename.c_str() );

  std::string state_seq((std::istreambuf_iterator<char>(input)),
                 std::istreambuf_iterator<char>());


  size_t pos = 0;
  std::string state;
  std::string delimiter = "<endstate>";
  while ((pos = state_seq.find(delimiter)) != std::string::npos) {
	  state = state_seq.substr(0, pos);
	  //std::cout << state << std::endl;  
	  m_string_trajectory.push( state );
	  state_seq.erase(0, pos + delimiter.length());
  }

  std::cout << m_string_trajectory.size() << std::endl;
  input.close();
  createAgents();
}

void TrajectoryController::createAgents() {
}

bool TrajectoryController::isDone() {
  // Die once we reach enough samples
	return ((m_max_num_frames > 0 && m_environment.getFrameNumber() >= m_max_num_frames) ||
		(m_max_num_episodes > 0 && m_episode_number > m_max_num_episodes) ||
		m_string_trajectory.empty());
}

void TrajectoryController::run() {
  Action action_a, action_b;

  bool firstStep = true;

  std::cout << "press any key to start...";
  getchar();
  while (!isDone()) {
    // Start a new episode if we're in a terminal state... assume these agents need to be told
    //  about episode-end
    if (m_environment.isTerminal()) {
      episodeEnd();
      firstStep = true;
    }

    else {
      if (firstStep) {
        // Start a new episode; obtain actions
        episodeStart(action_a, action_b);
        firstStep = false;
      }
      else
        // Poll agents for actions
        episodeStep(action_a, action_b);
    
      // Apply said actions
      m_episode_score += applyActions(action_a, action_b);
    }

    // Display if necessary
    display();
  }
  episodeEnd();
}

void TrajectoryController::episodeStep(Action& action_a, Action& action_b) {
 
    usleep(16667);
    ALEState new_state = m_environment.cloneState();

    string serialized_state = m_string_trajectory.front();
    m_string_trajectory.pop();

    new_state.set_serialized(  serialized_state );
    m_environment.restoreState( new_state );

    action_a =  PLAYER_A_NOOP;
    action_b =  PLAYER_B_NOOP;
}

void TrajectoryController::episodeStart(Action& action_a, Action& action_b) {

    episodeStep(action_a, action_b);
    
    // Some bookkeeping
    m_episode_score = 0;
    m_episode_number++;
    m_episode_t0 = aptk::time_used();
}

void TrajectoryController::episodeEnd() {
	m_episode_tf = aptk::time_used();
	
	// Reset environment
	m_environment.reset();

	// Produce some meaningful output
	fprintf (stderr, "Episode %d ended, score: %d\n", m_episode_number, m_episode_score);

	std::stringstream filename;
	filename << "episode." << m_episode_number;
	
	std::ofstream output( filename.str().c_str() );
	output << "elapsed_time=" << m_episode_tf - m_episode_t0 << ",score=" << m_episode_score << std::endl;
	output.close();	
}

