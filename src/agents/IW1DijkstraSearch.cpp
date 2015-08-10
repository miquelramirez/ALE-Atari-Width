#include "IW1DijkstraSearch.hpp"
#include "SearchAgent.hpp"
#include <list>

IW1DijkstraSearch::IW1DijkstraSearch(RomSettings *rom_settings, Settings &settings,
			       ActionVect &actions, StellaEnvironment* _env) 
    : IW1Search( rom_settings, settings, actions, _env){
 
	m_max_reward = settings.getInt( "max_reward" );
}

IW1DijkstraSearch::~IW1DijkstraSearch() {

}

class TreeNodeComparer
{
public:

	bool operator()( TreeNode* a, TreeNode* b ) const 
	{
		if ( b->fn < a->fn ) return true;
		return false;
	}
};

int IW1DijkstraSearch::expand_node( TreeNode* curr_node, std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparer >& q )
{
	int num_simulated_steps =0;
	int num_actions = available_actions.size();
	bool leaf_node = (curr_node->v_children.empty());
	m_expanded_nodes++;
	// Expand all of its children (simulates the result)	

	if(leaf_node){
		curr_node->v_children.resize( num_actions );
		curr_node->available_actions = available_actions;
		if(m_randomize_successor)
		    std::random_shuffle ( curr_node->available_actions.begin(), curr_node->available_actions.end() );

	}
	for (int a = 0; a < num_actions; a++) {
		Action act = curr_node->available_actions[a];
		
		TreeNode * child;
	
		// If re-expanding an internal node, don't creates new nodes
		if (leaf_node) {
			m_generated_nodes++;
			child = new TreeNode(	curr_node,	
						curr_node->state,
						this,
						act,
						sim_steps_per_node,
						discount_factor); 
	
			if ( check_novelty_1( child->state.getRAM() ) ) {
					update_novelty_table( child->state.getRAM() );
			}
			else{				
				curr_node->v_children[a] = child;				
				child->is_terminal = true;
				m_pruned_nodes++;
				continue;				
			}
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			num_simulated_steps += child->num_simulated_steps;
					
			curr_node->v_children[a] = child;

		}
		else {
			child = curr_node->v_children[a];
			
			// This recreates the novelty table (which gets resetted every time
			// we change the root of the search tree)
			if ( m_novelty_pruning ){
				if ( check_novelty_1( child->state.getRAM() ) ){
					update_novelty_table( child->state.getRAM() );
					child->is_terminal = false;
				}
				else{
					child->is_terminal = true;
					m_pruned_nodes++;
				}
			}
			child->updateTreeNode();
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();

			if ( !child->is_terminal )
				num_simulated_steps += child->num_simulated_steps;
	
		}
	
		// Don't expand duplicate nodes, or terminal nodes
		if (!child->is_terminal) {
			if (! (ignore_duplicates && test_duplicate(child)) )
				q.push(child);
		}
	}
	return num_simulated_steps;
}

/* *********************************************************************
   Expands the tree from the given node until i_max_sim_steps_per_frame
   is reached
	
   ******************************************************************* */
void IW1DijkstraSearch::expand_tree(TreeNode* start_node) {
	// If the root is terminal, we will not expand any of its children; deal with this
	//  appropriately
	if (start_node->is_terminal) {
		set_terminal_root(start_node);
		return;
	}
	if(!start_node->v_children.empty()) start_node->updateTreeNode();

	std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparer > q;
	std::list< TreeNode* > pivots;
	
	//q.push(start_node);
	pivots.push_back( start_node );

	update_novelty_table( start_node->state.getRAM() );
	int num_simulated_steps = 0;

	m_expanded_nodes = 0;
	m_generated_nodes = 0;

	m_pruned_nodes = 0;

	do {
		
		std::cout << "# Pivots: " << pivots.size() << std::endl;
		std::cout << "First pivot reward: " << pivots.front()->node_reward << std::endl;
		pivots.front()->m_depth = 0;
		int steps = expand_node( pivots.front(), q );
		num_simulated_steps += steps;

		if (num_simulated_steps >= max_sim_steps_per_frame) {
			break;
		}

		pivots.pop_front();

		while(!q.empty()) {
			// Pop a node to expand
			TreeNode* curr_node = q.top();
			q.pop();
	
			if ( curr_node->depth() > m_reward_horizon - 1 ) continue;
			if ( m_stop_on_first_reward && curr_node->node_reward != 0 ) 
			{
				pivots.push_back( curr_node );
				continue;
			}
			steps = expand_node( curr_node, q );	
			num_simulated_steps += steps;
			// Stop once we have simulated a maximum number of steps
			if (num_simulated_steps >= max_sim_steps_per_frame) {
				break;
			}
		
		}
		std::cout << "\tExpanded so far: " << m_expanded_nodes << std::endl;	
		std::cout << "\tPruned so far: " << m_pruned_nodes << std::endl;	
		std::cout << "\tGenerated so far: " << m_generated_nodes << std::endl;	

		if (q.empty()) std::cout << "Search Space Exhausted!" << std::endl;
		// Stop once we have simulated a maximum number of steps
		if (num_simulated_steps >= max_sim_steps_per_frame) {
			break;
		}

	} while ( !pivots.empty() );
    

	
	update_branch_return(start_node);
}

