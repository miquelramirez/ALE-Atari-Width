#include "IW11Search.hpp"
#include "SearchAgent.hpp"
#include "Constants.h"
#include <list>

IW11Search::IW11Search(RomSettings *rom_settings, Settings &settings,
			       ActionVect &actions, StellaEnvironment* _env) :
	SearchTree(rom_settings, settings, actions, _env) {
	
	m_stop_on_first_reward = settings.getBool( "iw1_stop_on_first_reward", true );

	int val = settings.getInt( "iw1_reward_horizon", -1 );
	m_reward_horizon = ( val < 0 ? std::numeric_limits<unsigned>::max() : val ); 

	m_act_novelty_table.resize( PLAYER_A_MAX );

	for ( unsigned act = 0; act < PLAYER_A_MAX; act++ ) 
		m_act_novelty_table[act] = new aptk::Bit_Matrix( RAM_SIZE, 256 );		

	m_ram_novelty_table = nullptr;
}

IW11Search::~IW11Search() {
	for ( unsigned act = 0; act < PLAYER_A_MAX; act++ ) 
		delete m_act_novelty_table[act];
}

/* *********************************************************************
   Builds a new tree
   ******************************************************************* */
void IW11Search::build(ALEState & state) {	
	assert(p_root == NULL);
	p_root = new TreeNode(NULL, state, NULL, UNDEFINED, 0);
	update_tree();
	is_built = true;	
}

void IW11Search::print_path(TreeNode * node, int a) {
	cerr << "Path, return " << node->v_children[a]->branch_return << endl;

	while (!node->is_leaf()) {
		TreeNode * child = node->v_children[a];

		cerr << "\tAction " << a << " Reward " << child->node_reward << 
			" Return " << child->branch_return << 
			" Terminal " << child->is_terminal << endl;
    
		node = child;
		if (!node->is_leaf())
			a = node->best_branch;
	}
}

// void IW11Search::initialize_tree(TreeNode* node){
// 	do {
// 		Action act =  Action::PLAYER_A_NOOP
// 		m_generated_nodes++;

// 		TreeNode* child = new TreeNode(	curr_node,	
// 					curr_node->state,
// 					this,
// 					act,
// 					sim_steps_per_node); 
		
// 		if ( check_novelty_1( child->state.getRAM() ) ) {
// 			update_novelty_table( child->state.getRAM() );
			
// 		}
// 		else{
// 			curr_node->v_children[a] = child;
// 			child->is_terminal = true;
// 			m_pruned_nodes++;
// 			break;
// 		}

// 		num_simulated_steps += child->num_simulated_steps;					
// 		node->v_children[a] = child;
		
// 		node = child;
// 	}while( node->depth() < m_max_depth)
// }

void IW11Search::update_tree() {
	expand_tree(p_root);
}

void IW11Search::update_novelty_table( const ALERAM& machine_state )
{
	for ( size_t i = 0; i < machine_state.size(); i++ )
		m_ram_novelty_table->set( i, machine_state.get(i) );
}

bool IW11Search::check_novelty_1( const ALERAM& machine_state )
{
	for ( size_t i = 0; i < machine_state.size(); i++ )
		if ( !m_ram_novelty_table->isset( i, machine_state.get(i) ) )
			return true;
	return false;
}

int IW11Search::expand_node( TreeNode* curr_node, queue<TreeNode*>& q )
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
						sim_steps_per_node); 
	
			if ( check_novelty_1( child->state.getRAM() ) ) {
				update_novelty_table( child->state.getRAM() );
					
			}
			else{
				curr_node->v_children[a] = child;
				child->is_terminal = true;
				m_pruned_nodes++;
			}
			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			num_simulated_steps += child->num_simulated_steps;
					
			curr_node->v_children[a] = child;
		}
		else {
			child = curr_node->v_children[a];
		
			// This recreates the novelty table (which gets resetted every time
			// we change the root of the search tree)
			if ( m_novelty_pruning )
				if ( check_novelty_1( child->state.getRAM() ) ){
					update_novelty_table( child->state.getRAM() );
					child->is_terminal = false;
				}
				else{
					child->is_terminal = true;
					m_pruned_nodes++;
				}
			
			child->updateTreeNode();

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
void IW11Search::expand_tree(TreeNode* start_node) {
	// If the root is terminal, we will not expand any of its children; deal with this
	//  appropriately
	if (start_node->is_terminal) {
		set_terminal_root(start_node);
		return;
	}

	if(!start_node->v_children.empty()) start_node->updateTreeNode();

	queue<TreeNode*> q;
	
	int num_simulated_steps = 0;

	m_expanded_nodes = 0;
	m_generated_nodes = 0;

	m_pruned_nodes = 0;
	int num_actions = available_actions.size();
	m_expanded_nodes++;
	bool leaf_node = start_node->v_children.empty();
	if ( leaf_node ) {
		start_node->v_children.resize( num_actions );
		start_node->available_actions = available_actions;
		if(m_randomize_successor)
			std::random_shuffle ( start_node->available_actions.begin(), start_node->available_actions.end() );
	}

	for (int a = 0; a < num_actions; a++) {
		Action act = start_node->available_actions[a];

		TreeNode* child;
		if ( leaf_node ) {
		 	child = new TreeNode( start_node, start_node->state, this, act, sim_steps_per_node );
			start_node->v_children[a] = child;
		}
		else
			child = start_node->v_children[a];

		num_simulated_steps += child->num_simulated_steps;
		q.push( child );
		m_ram_novelty_table = m_act_novelty_table[a];
		update_novelty_table( child->state.getRAM() );

		int max_steps_per_child = max_sim_steps_per_frame / num_actions; 
		int num_steps_per_child = 0;
                m_max_depth = 0;
		while(!q.empty()) {
			// Pop a node to expand
			TreeNode* curr_node = q.front();
			q.pop();
	
			int steps = expand_node( curr_node, q );	
			num_simulated_steps += steps;
			num_steps_per_child += steps;
			// Stop once we have simulated a maximum number of steps
			if ( num_steps_per_child >= max_steps_per_child) {
				break;
			}
		
		}
		std::cout << "\tExpanded so far: " << m_expanded_nodes << std::endl;	
		std::cout << "\tPruned so far: " << m_pruned_nodes << std::endl;	
		std::cout << "\tGenerated so far: " << m_generated_nodes << std::endl;	
		std::cout << "Action: " << action_to_string(child->act) << " Depth: " << m_max_depth  <<" NumNodes: " << child->num_nodes() << " Reward: "<< child->branch_return   << std::endl;

		if (q.empty()) std::cout << "Search Space Exhausted!" << std::endl;
		else {
			while (!q.empty())
				q.pop();
		}
    	}

	
	update_branch_return(start_node);
}

void IW11Search::clear()
{
	SearchTree::clear();
	for ( unsigned act = 0; act < PLAYER_A_MAX; act++ ) 
		m_act_novelty_table[act]->clear();
}

void IW11Search::move_to_best_sub_branch() 
{
	SearchTree::move_to_best_sub_branch();
	for ( unsigned act = 0; act < PLAYER_A_MAX; act++ ) 
		m_act_novelty_table[act]->clear();
}

/* *********************************************************************
   Updates the branch reward for the given node
   which equals to: node_reward + max(children.branch_return)
   ******************************************************************* */
void IW11Search::update_branch_return(TreeNode* node) {
	// Base case (leaf node): the return is the immediate reward
	if (node->v_children.empty()) {
		node->branch_return = node->node_reward;
		node->best_branch = -1;
		node->branch_depth = node->m_depth;
		return;
	}

	// First, we have to make sure that all the children are updated
	for (unsigned int c = 0; c < node->v_children.size(); c++) {
		TreeNode* curr_child = node->v_children[c];
			
		if (ignore_duplicates && curr_child->is_duplicate()) continue;
    
		update_branch_return(curr_child);
	}
	
	// Now that all the children are updated, we can update the branch-reward
	float best_return = -1;
	int best_branch = -1;
	return_t avg = 0;

	// Terminal nodes encur no reward beyond immediate
	if (node->is_terminal) {
		node->branch_depth = node->m_depth;
		best_return = node->node_reward;
		best_branch = 0;		
	} else {
	    
		for (size_t a = 0; a < node->v_children.size(); a++) {	
			return_t child_return = node->v_children[a]->branch_return;
			if (best_branch == -1 || child_return > best_return) {
				best_return = child_return;
				best_branch = a;
				avg+=child_return;
			}
			if( node->v_children[a]->branch_depth > node->branch_depth  ) 
				node->branch_depth = node->v_children[a]->branch_depth;

			if( node->v_children.size() ) avg/=node->v_children.size();
		}
	}

	node->branch_return = node->node_reward + best_return * discount_factor; 
	//node->branch_return = node->node_reward + avg * discount_factor; 
	
	node->best_branch = best_branch;
}

void IW11Search::set_terminal_root(TreeNode * node) {
	node->branch_return = node->node_reward; 

	if (node->v_children.empty()) {
		// Expand one child; add it to the node's children
		TreeNode* new_child = new TreeNode(	node, node->state, 
							this, PLAYER_A_NOOP, sim_steps_per_node);

		node->v_children.push_back(new_child);
    	}
  
    	// Now we have at least one child, set the 'best branch' to the first action
    	node->best_branch = 0; 
}

void	IW11Search::print_frame_data( int frame_number, float elapsed, Action curr_action, std::ostream& output )
{
	output << "frame=" << frame_number;
	output << ",expanded=" << expanded_nodes();
	output << ",generated=" << generated_nodes();
	output << ",pruned=" << pruned();
	output << ",depth_tree=" << max_depth();
	output << ",tree_size=" <<  num_nodes(); 
	output << ",best_action=" << action_to_string( curr_action );
	output << ",branch_reward=" << get_root_value();
	output << ",elapsed=" << elapsed;
	m_rom_settings->print( output );
	output << std::endl;
}
