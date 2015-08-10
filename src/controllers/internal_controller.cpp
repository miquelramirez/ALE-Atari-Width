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
 *  internal_controller.hpp
 *
 *  The InternalController class allows users to directly interface with ALE.
 *
 **************************************************************************** */

#include "internal_controller.hpp"
#include "time.hxx"
#include <fstream>
#include <sstream>

InternalController::InternalController(OSystem* osystem):
  ALEController(osystem),
  m_max_num_frames(0),
  m_episode_score(0),
  m_episode_number(0),
  m_agent_left(NULL),
  m_agent_right(NULL) {

  m_max_num_frames = m_osystem->settings().getInt("max_num_frames");
  m_max_num_episodes = m_osystem->settings().getInt("max_num_episodes");

  createAgents();
}

void InternalController::createAgents() {
    m_agent_left.reset(PlayerAgent::generate_agent_instance(m_osystem, m_settings.get(), &m_environment));

  // Right agent is set to NULL. While this isn't necessary, all of the currently implemented
  //  agents return actions for player A. One easy fix would be to add PLAYER_B_NOOP to actions
  //  returned by such agents... we'll get around to it.
    if (m_osystem->settings().getString("search_method_B",false) != "")
	    m_agent_right.reset(PlayerAgent::generate_agent_instance(m_osystem, m_settings.get(), &m_environment, true));
    else
	    m_agent_right.reset(NULL);
}

bool InternalController::isDone() {
  // Die once we reach enough samples
  return ((m_max_num_frames > 0 && m_environment.getFrameNumber() >= m_max_num_frames) ||
    (m_max_num_episodes > 0 && m_episode_number > m_max_num_episodes) ||
    (m_agent_left.get() != NULL && m_agent_left->has_terminated()) ||
    (m_agent_right.get() != NULL && m_agent_right->has_terminated()));
}

void InternalController::run() {
  Action action_a, action_b;

  bool firstStep = true;

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

void InternalController::episodeStep(Action& action_a, Action& action_b) {
  // Request a new action
    if ( m_agent_left.get() != nullptr ){
	ALEState new_state = m_environment.cloneState();
	ALEState* p_new_state = new ALEState(new_state, new_state.serialized() );
	string kk = new_state.serialized();
	m_agent_left->update_state( p_new_state );
    }
    if ( m_agent_right.get() != nullptr ){
	ALEState new_state = m_environment.cloneState();
	ALEState* p_new_state = new ALEState(new_state, new_state.serialized() );
	m_agent_right->update_state( p_new_state );
    }
  action_b = (m_agent_right.get() != NULL) ? m_agent_right->agent_step() : PLAYER_B_NOOP;

  action_a = (m_agent_left.get() != NULL) ? m_agent_left->agent_step() : PLAYER_A_NOOP;
}

void InternalController::episodeStart(Action& action_a, Action& action_b) {
  // Poll the agents for their first action
    if ( m_agent_left.get() != nullptr ){
	ALEState new_state = m_environment.cloneState();
	ALEState* p_new_state = new ALEState(new_state, new_state.serialized() );
	m_agent_left->update_state( p_new_state );
    }
    if ( m_agent_right.get() != nullptr ){
	ALEState new_state = m_environment.cloneState();
	ALEState* p_new_state = new ALEState(new_state, new_state.serialized() );
	m_agent_right->update_state( p_new_state );
    }
  
  action_a = (m_agent_left.get() != NULL) ? m_agent_left->episode_start() : PLAYER_A_NOOP;
  action_b = (m_agent_right.get() != NULL) ? m_agent_right->episode_start() : PLAYER_B_NOOP;

  // Some bookkeeping
  m_episode_score = 0;
  m_episode_number++;
  m_episode_t0 = aptk::time_used();
}

void InternalController::episodeEnd() {
	m_episode_tf = aptk::time_used();
	// Notify the agents that this is the	 end
	if (m_agent_left.get() != NULL) m_agent_left->episode_end();
	if (m_agent_right.get() != NULL) m_agent_right->episode_end();
	
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

