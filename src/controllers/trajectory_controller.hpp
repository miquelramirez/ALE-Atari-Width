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

#ifndef __TRAJECTORY_CONTROLLER_HPP__
#define __TRAJECTORY_CONTROLLER_HPP__

#include "ale_controller.hpp"
#include "../agents/PlayerAgent.hpp"
#include <queue>
#include <string>

class TrajectoryController : public ALEController {
public:
    TrajectoryController(OSystem* osystem);
    virtual ~TrajectoryController() {}

    virtual void run();

private:
    bool isDone();
    void createAgents();
	
    void episodeEnd();
    void episodeStart(Action& action_a, Action& action_b);
    void episodeStep(Action& action_a, Action& action_b);

private:
    int m_max_num_frames; // Maximum number of total frames before we stop
    int m_max_num_episodes; // Maximum number of episodes before we stop

    float m_episode_t0;
    float m_episode_tf;


    int m_episode_score; // Keeping track of score
    int m_episode_number; // Keeping track of episode 

    queue<string> m_string_trajectory;
};

#endif // __TRAJECTORY_CONTROLLER_HPP__
