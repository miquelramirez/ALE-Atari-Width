#include "Width2BestFirstSearch.hpp"
#include "SearchAgent.hpp"
#include <list>

Width2BestFirstSearch::Width2BestFirstSearch(RomSettings *rom_settings, Settings &settings,
			       ActionVect &actions, StellaEnvironment* _env) 
    : IW1Search( rom_settings, settings, actions, _env){
 
	m_max_reward = settings.getInt( "max_reward" );

	q_exploration = new std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploration >;
	q_exploitation = new std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploitation >;
	m_ram_novelty_table_2.resize( 0x80ff80ff );	

}

Width2BestFirstSearch::~Width2BestFirstSearch() {
		    delete q_exploration;
		    delete q_exploitation;	
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

void Width2BestFirstSearch::clear() {
	IW1Search::clear();
	m_ram_novelty_table_2.reset();	
}

void Width2BestFirstSearch::update_novelty_table_2( const ALERAM& machine_state )
{
	for ( size_t i = 0; i < machine_state.size(); i++ ) {
		Fluent p_i = std::make_pair( i, machine_state.get(i) ); 
		for ( size_t j = i ; j < machine_state.size(); j++ ) {
			Fluent p_j = std::make_pair( j, machine_state.get(j) );
			m_ram_novelty_table_2.set( fluent_pair_index( p_i, p_j ) );
		}	
	}
}

bool Width2BestFirstSearch::check_novelty_2( const ALERAM& machine_state )
{
	for ( size_t i = 0; i < machine_state.size(); i++ ) {
		Fluent p_i = std::make_pair( i, machine_state.get(i) ); 
		for ( size_t j = i ; j < machine_state.size(); j++ ) {
			Fluent p_j = std::make_pair( j, machine_state.get(j) );
			if( !m_ram_novelty_table_2.isset( fluent_pair_index( p_i, p_j ) ) )
				return true;
		}	
	}

	return false;
}

int Width2BestFirstSearch::expand_node( TreeNode* curr_node )
{
	int num_simulated_steps =0;
	int num_actions = available_actions.size();
	bool leaf_node = (curr_node->v_children.empty());
	m_expanded_nodes++;
	if(curr_node->novelty == 1)
	    m_exp_count_novelty1++;
	else
	    m_exp_count_novelty2++;
	// Expand all of its children (simulates the result)	
	if(leaf_node){ 
		curr_node->v_children.resize( num_actions );
		curr_node->available_actions = available_actions;
		if(m_randomize_successor)
			std::random_shuffle ( curr_node->available_actions.begin()+1, curr_node->available_actions.end() );

	
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
						sim_steps_per_node
						, discount_factor); 
	
			if ( check_novelty_1( child->state.getRAM() ) ) {
			    	update_novelty_table( child->state.getRAM() );
				update_novelty_table_2( child->state.getRAM() );
			    	child->novelty = 1;
			    	m_gen_count_novelty1++;
			}
			else{
				if ( check_novelty_2( child->state.getRAM() ) ) {
					update_novelty_table( child->state.getRAM() );
					update_novelty_table_2( child->state.getRAM() );
					child->novelty = 2;
					m_gen_count_novelty2++;
				}
				else {
					child->novelty = 3;
				}
			}
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();
			num_simulated_steps += child->num_simulated_steps;
					
			curr_node->v_children[a] = child;

		}
		else {
			
			child = curr_node->v_children[a];
			m_pruned_nodes++;
			// This recreates the novelty table (which gets resetted every time
			// we change the root of the search tree)
			if ( m_novelty_pruning ){
				if ( check_novelty_1( child->state.getRAM() ) ){
					update_novelty_table( child->state.getRAM() );
					child->novelty = 1;
					m_gen_count_novelty1++;
				}
				else{
					child->novelty = 2;
					m_gen_count_novelty2++;
					
				}
			}
			
			child->updateTreeNode();
			child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;

			if (child->depth() > m_max_depth ) m_max_depth = child->depth();

			
			num_simulated_steps += child->num_simulated_steps;
		}
	
		// Don't expand duplicate nodes, or terminal nodes
		if (!child->is_terminal) {
		    if (! (ignore_duplicates && test_duplicate(child)) ){				
				if( child->fn !=  m_max_reward )
					q_exploitation->push(child);
				else
					q_exploration->push(child);
		    }
		}
	}
	

	curr_node->already_expanded = true;
	return num_simulated_steps;
}

/* *********************************************************************
	update novelty_value to 0 to a node and all its children, all the way down the branch
 ******************************************************************* */
void Width2BestFirstSearch::reset_branch(TreeNode* node) {
	if (!node->v_children.empty()) {
		for(size_t c = 0; c < node->v_children.size(); c++) {			
			reset_branch(node->v_children[c]);
			
		}
	}
	node->novelty = 0;
	node->fn = 0;	
	node->already_expanded = false;
}

int Width2BestFirstSearch::reuse_branch(TreeNode* node) {
	int num_simulated_steps = 0;

	node->updateTreeNode();
	update_novelty_table( node->state.getRAM() );
	update_novelty_table_2( node->state.getRAM() );

	queue<TreeNode*> q;
	q.push( node );

	while(!q.empty()) {
		// Pop a node to expand
		TreeNode* curr_node = q.front();
		q.pop();
		
		if ( curr_node->depth() > m_reward_horizon - 1 ) continue;
		if (!node->v_children.empty()) {
			for(size_t c = 0; c < node->v_children.size(); c++) {			
				TreeNode* child = curr_node->v_children[c];
				
				// This recreates the novelty table (which gets resetted every time
				// we change the root of the search tree)
				if ( m_novelty_pruning ){
					if ( check_novelty_1( child->state.getRAM() ) ){
						update_novelty_table( child->state.getRAM() );
						update_novelty_table_2( child->state.getRAM() );
						if(!child->already_expanded)
						    child->novelty = 1;
					}
					else{
						if ( check_novelty_2( child->state.getRAM() ) ) {
							update_novelty_table( child->state.getRAM() );
							update_novelty_table_2( child->state.getRAM() );
							if(!child->already_expanded)
								child->novelty = 2;
						}
						else {
							if ( !child->already_expanded )
								child->novelty = 3;
						}	
						
					}
				}
				
				child->updateTreeNode();
				child->fn += ( m_max_reward - child->discounted_accumulated_reward ); // Miquel: add this to obtain Hector's BFS + m_max_reward * (720 - child->depth()) ;
				
				if (child->depth() > m_max_depth ) m_max_depth = child->depth();
				
				
				num_simulated_steps += child->num_simulated_steps;
				
				// Don't expand duplicate nodes, or terminal nodes
				if (!child->is_terminal) {
					if (! (ignore_duplicates && test_duplicate(child)) ){
						if(!child->already_expanded){
							if( child->fn !=  m_max_reward )
								q_exploitation->push(child);

							q_exploration->push(child);
						}
						else
							q.push(child);

					}
				}
				
			}
		}
		// Stop once we have simulated a maximum number of steps
		if (num_simulated_steps >= max_sim_steps_per_frame) {
			break;
		}
		
	}
	
	return num_simulated_steps;

}

unsigned Width2BestFirstSearch::size_branch(TreeNode* node) {
	unsigned size = 1;

	if (!node->v_children.empty()) {
		for(size_t c = 0; c < node->v_children.size(); c++) {			
			size += size_branch(node->v_children[c]);
			
		}
	}
	return size;	

}

/* *********************************************************************
   Expands the tree from the given node until i_max_sim_steps_per_frame
   is reached
	
   ******************************************************************* */

void Width2BestFirstSearch::expand_tree(TreeNode* start_node) {
    // If the root is terminal, we will not expand any of its children; deal with this
    //  appropriately
    if (start_node->is_terminal) {
	set_terminal_root(start_node);
	return;
    }

    int num_simulated_steps = 0;
    bool explore = true;
    

    clear_queues();
    
    if(!start_node->v_children.empty()) {
	    start_node->updateTreeNode();
	    num_simulated_steps += reuse_branch( start_node );
	    std::cout  << "Num_reused_steps: "<< num_simulated_steps << std::endl;
	    // reset_branch( start_node );
	    // q_exploration->push(start_node);        
	    // update_novelty_table( start_node->state.getRAM() );
    }
    else
	    {
	    q_exploration->push(start_node);        
	    update_novelty_table( start_node->state.getRAM() );
	    update_novelty_table_2( start_node->state.getRAM() );
    }

    m_expanded_nodes = 0;
    m_generated_nodes = 0;
    m_exp_count_novelty1 = 0;
    m_exp_count_novelty2 = 0;
    m_gen_count_novelty1 = 0;
    m_gen_count_novelty2 = 0;
    m_pruned_nodes = 0;
    

    while( ! (q_exploration->empty() && q_exploitation->empty()) ) {
	// Pop a node to expand
	TreeNode* curr_node;
	if( explore ){	    
		if( q_exploration->empty() )
			break;
		curr_node = q_exploration->top();
		q_exploration->pop();
		explore = false;

	}
	else{
		if( q_exploitation->empty() ){
			explore = true;
			continue;
		}

	
	    curr_node = q_exploitation->top();
	    q_exploitation->pop();
	    explore = true;
	}


	if ( curr_node->depth() > m_max_depth ) m_max_depth = curr_node->depth();

	/**
	 * check nodes that have been expanded by other queue
	 */
	 if(  curr_node->already_expanded ) 
	 	continue;
	
		

	//if(curr_node->novelty != 1)
	//	std::cout << curr_node->depth() << " " << curr_node->novelty << " " << curr_node->fn << " " << std::endl;

	if ( curr_node->depth() > m_reward_horizon - 1 ) continue;


	num_simulated_steps +=  expand_node( curr_node );
	// std::cout << "q_exploration size: "<< q_exploration.size() << std::endl;
	// std::cout << "q_exploitation size: "<< q_exploitation.size() << std::endl;
	// Stop once we have simulated a maximum number of steps
	if (num_simulated_steps >= max_sim_steps_per_frame) {
	    break;
	}
		
    }
   
    std::cout << "\tExpanded so far: " << m_expanded_nodes << std::endl;	
    std::cout << "\tExpanded Novelty 1: " << m_exp_count_novelty1 << std::endl;	
    std::cout << "\tExpanded Novelty 2: " << m_exp_count_novelty2 << std::endl;	
    std::cout << "\tPruned so far: " << m_pruned_nodes << std::endl;	
    std::cout << "\tGenerated so far: " << m_generated_nodes << std::endl;	
    std::cout << "\tGenerated Novelty 1: " << m_gen_count_novelty1 << std::endl;	
    std::cout << "\tGenerated Novelty 2: " << m_gen_count_novelty2 << std::endl;	

    if ( q_exploration->empty() && q_exploitation->empty() ) std::cout << "Search Space Exhausted!" << std::endl;
    std::cout << "q_exploration size: "<< q_exploration->size() << std::endl;
    std::cout << "q_exploitation size: "<< q_exploitation->size() << std::endl;

	
    update_branch_return(start_node);
}

