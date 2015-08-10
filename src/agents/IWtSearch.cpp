#include "IWtSearch.hpp"
#include "SearchAgent.hpp"
#include <list>

IWtSearch::IWtSearch(RomSettings *rom_settings, Settings &settings,
			       ActionVect &actions, StellaEnvironment* _env) :
	SearchTree(rom_settings, settings, actions, _env) {
	
	m_stop_on_first_reward = settings.getBool( "iw1_stop_on_first_reward", true );

	int val = settings.getInt( "iw1_reward_horizon", -1 );

	m_reward_horizon = ( val < 0 ? std::numeric_limits<unsigned>::max() : val ); 

	m_max_var_group = 32;
	set_candidates();

}

IWtSearch::~IWtSearch() {
	for ( auto v : m_var_groups)
		delete v;

	for ( auto v : m_candidate_var_groups)
		delete v;
}


void IWtSearch::set_candidates(){
	if( m_var_groups.empty() ){
		for(unsigned byte = 0; byte < RAM_SIZE; byte++)
			for( unsigned bit = 0; bit < 8; bit++)
				m_candidate_var_groups.push_back( new VarGroup( byte, bit) );

		/** 
		 * Original 8 bit
		 */
		
		// for(unsigned byte = 0; byte < RAM_SIZE; byte++){
		// 	VarGroup* new_v = new VarGroup(  );
		// 	for( unsigned bit = 0; bit < 8; bit++){
		// 		new_v->add_bit_index( byte, bit );
		// 	}
		// 	m_candidate_var_groups.push_back( new_v );
		// }
		
		
	}
	else{
		/** 
		 * EXHAUSTIVE
		 */
		// for( auto v : m_var_groups){
		// 	for(unsigned byte = 0; byte < RAM_SIZE; byte++){
		// 		for( unsigned bit = 0; bit < 8; bit++){
		// 			VarGroup* new_v = new VarGroup( *v );
		// 			new_v->add_bit_index( byte, bit );
		// 			m_candidate_var_groups.push_back( new_v );
		// 		}
		// 	}
		// }

		// /** 
		//  * Original 8 bit
		//  */
		
		// for(unsigned byte = 0; byte < RAM_SIZE; byte++){
		// 	VarGroup* new_v = new VarGroup(  );
		// 	for( unsigned bit = 0; bit < 8; bit++){
		// 		new_v->add_bit_index( byte, bit );
		// 	}
		// 	m_candidate_var_groups.push_back( new_v );
		// }
		/**
		 * one ignored * one none ignored
		 */
		// unsigned nvars = m_var_groups.size();
		// unsigned curr_var = 0;
		// for( auto v : m_ignored_candidate_var_groups){
		// 	VarGroup* new_v = new VarGroup( *(m_var_groups[ curr_var ]) );
		// 	for( auto bit_idx : v->bit_indexes() ){
		// 		new_v->add_bit_index( bit_idx.first, bit_idx.second );
		// 	}
		// 	m_candidate_var_groups.push_back( new_v );
		// 	curr_var = ( curr_var == nvars ) ? 0 : curr_var+1;
		// }

		/**
		 * 1 ignored * all non ignored. Like exhaustive but incrementally
		 */
		VarGroup* curr = m_ignored_candidate_var_groups.back();
		m_ignored_candidate_var_groups.pop_back();
		for( auto v : m_var_groups){
			VarGroup* new_v = new VarGroup( *v );
			for( auto bit_idx : curr->bit_indexes() ){
				new_v->add_bit_index( bit_idx.first, bit_idx.second );
			}
			m_candidate_var_groups.push_back( new_v );
		}
	}
	
	
    
}

 bool IWtSearch::keep_candidates(){

	 
	std::cout << "num_candidates: " << m_candidate_var_groups.size() << " OLD_var_groups: " << m_var_groups.size() << std::endl;
	for ( auto v : m_var_groups){
		unsigned num_values = v->count_num_values_reached();
		if( num_values > v->last_num_values_reached() )
			v->set_last_num_values_reached( num_values );
	}
		
	bool new_vars = false;
	for ( auto v : m_candidate_var_groups){
		//std::cout << "Byte:" << v->bit_indexes()[0].first << std::endl;
		unsigned num_values = v->count_num_values_reached();
		if( num_values > v->last_num_values_reached() ){
			v->set_last_num_values_reached( num_values );
			m_var_groups.push_back( v );
			new_vars = true;
			
		}else{
			if( v->bit_indexes().size() == 1)
				m_ignored_candidate_var_groups.push_back( v );
			else{
				delete v;
				v = nullptr;
			}
		}
			
		
	}
	

	//std::exit(0);
	m_candidate_var_groups.clear();
	
	std::cout << "ignored_candidates: " << m_ignored_candidate_var_groups.size() << " NEW_var_groups: " << m_var_groups.size() << std::endl;
	
	 if( ! m_ignored_candidate_var_groups.empty() )
	 	new_vars = true;
	
	return new_vars;
	
	
}
/* *********************************************************************
   Builds a new tree
   ******************************************************************* */
void IWtSearch::build(ALEState & state) {	
	assert(p_root == NULL);
	p_root = new TreeNode(NULL, state, NULL, UNDEFINED, 0);
	update_tree();
	is_built = true;	
}

void IWtSearch::print_path(TreeNode * node, int a) {
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

// void IWtSearch::initialize_tree(TreeNode* node){
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

void IWtSearch::update_tree() {
	m_num_simulated_steps =0;
	do{

		expand_tree(p_root);	    
		if( keep_candidates() )
			set_candidates();
		else
			break;
		
	}while( m_num_simulated_steps < max_sim_steps_per_frame );
}

void IWtSearch::update_novelty_table( const ALERAM& machine_state )
{

 	for(auto v : m_candidate_var_groups){
		v->update_novelty_table( machine_state );			
	}

	for(auto v : m_var_groups){
		v->update_novelty_table( machine_state );			
	}
    
}

bool IWtSearch::check_novelty( const ALERAM& machine_state )
{
	for(auto v : m_candidate_var_groups){
		if( v->is_novel( machine_state ) )
			return true;
	}

	for(auto v : m_var_groups){
		if( v->is_novel( machine_state ) )
			return true;
	}


	return false;
}

int IWtSearch::expand_node( TreeNode* curr_node, queue<TreeNode*>& q )
{
	int num_simulated_steps =0;
	int num_actions = available_actions.size();
	bool leaf_node = (curr_node->v_children.empty());
	static     int max_nodes_per_frame = max_sim_steps_per_frame / sim_steps_per_node;
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
	
			if ( check_novelty( child->state.getRAM() ) ) {
				update_novelty_table( child->state.getRAM() );
					
			}
			else{
				curr_node->v_children[a] = child;
				child->is_terminal = true;
				m_pruned_nodes++;
				//continue;				
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
			{
			    if( child->is_terminal )
			    {
				if ( check_novelty( child->state.getRAM() ) ){
				    update_novelty_table( child->state.getRAM() );
				    child->is_terminal = false;
				}
				else{
				    child->is_terminal = true;
				    m_pruned_nodes++;
				}
			    }
			}
			child->updateTreeNode();

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();

			// DONT COUNT REUSED NODES
			//if ( !child->is_terminal )
			//	num_simulated_steps += child->num_simulated_steps;

		}
	
		// Don't expand duplicate nodes, or terminal nodes
		if (!child->is_terminal) {
			if (! (ignore_duplicates && test_duplicate(child)) )
			    if( child->num_nodes_reusable < max_nodes_per_frame )
				q.push(child);
		}
	}
	return num_simulated_steps;
}

/* *********************************************************************
   Expands the tree from the given node until i_max_sim_steps_per_frame
   is reached
	
   ******************************************************************* */
void IWtSearch::expand_tree(TreeNode* start_node) {

	if(!start_node->v_children.empty()){
	    start_node->updateTreeNode();
	    for (int a = 0; a < available_actions.size(); a++) {
		TreeNode* child = start_node->v_children[a];
		if( !child->is_terminal ){
		        child->num_nodes_reusable = child->num_nodes();
		}
	    }
	}

	queue<TreeNode*> q;
	std::list< TreeNode* > pivots;
	
	//q.push(start_node);
	pivots.push_back( start_node );

	update_novelty_table( start_node->state.getRAM() );

	m_expanded_nodes = 0;
	m_generated_nodes = 0;

	m_pruned_nodes = 0;

	do {
		
		std::cout << "# Pivots: " << pivots.size() << std::endl;
		std::cout << "First pivot reward: " << pivots.front()->node_reward << std::endl;
		pivots.front()->m_depth = 0;
		int steps = expand_node( pivots.front(), q );
		m_num_simulated_steps += steps;

		if (m_num_simulated_steps >= max_sim_steps_per_frame) {
			break;
		}

		pivots.pop_front();

		while(!q.empty()) {
			// Pop a node to expand
			TreeNode* curr_node = q.front();
			q.pop();
	
			if ( curr_node->depth() > m_reward_horizon - 1 ) continue;
			if ( m_stop_on_first_reward && curr_node->node_reward != 0 ) 
			{
				pivots.push_back( curr_node );
				continue;
			}
			steps = expand_node( curr_node, q );	
			m_num_simulated_steps += steps;
			// Stop once we have simulated a maximum number of steps
			if (m_num_simulated_steps >= max_sim_steps_per_frame) {
				break;
			}
		
		}
		std::cout << "\tExpanded so far: " << m_expanded_nodes << std::endl;	
		std::cout << "\tPruned so far: " << m_pruned_nodes << std::endl;	
		std::cout << "\tGenerated so far: " << m_generated_nodes << std::endl;	

		if (q.empty()) std::cout << "Search Space Exhausted!" << std::endl;
		// Stop once we have simulated a maximum number of steps
		if (m_num_simulated_steps >= max_sim_steps_per_frame) {
			break;
		}

	} while ( !pivots.empty() );
    

	
	update_branch_return(start_node);
}

void IWtSearch::clear()
{
	SearchTree::clear();
	
	for ( auto v : m_var_groups)
		v->clear();

	for ( auto v : m_candidate_var_groups)
		v->clear();
}

void IWtSearch::move_to_best_sub_branch() 
{
	SearchTree::move_to_best_sub_branch();

	for ( auto v : m_var_groups)
		v->clear();

	for ( auto v : m_candidate_var_groups)
		v->clear();
}

/* *********************************************************************
   Updates the branch reward for the given node
   which equals to: node_reward + max(children.branch_return)
   ******************************************************************* */
void IWtSearch::update_branch_return(TreeNode* node) {
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

void IWtSearch::set_terminal_root(TreeNode * node) {
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

void	IWtSearch::print_frame_data( int frame_number, float elapsed, Action curr_action, std::ostream& output )
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
