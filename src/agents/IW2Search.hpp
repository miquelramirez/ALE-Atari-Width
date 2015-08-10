#ifndef __IW_2_SEARCH_HPP__
#define __IW_2_SEARCH_HPP__

#include "SearchTree.hpp"
#include "bit_matrix.hxx"
#include "../environment/ale_ram.hpp"

#include <queue>
#include <map>
#include <utility>

typedef std::pair< unsigned char, unsigned char>	Fluent;

inline
bool operator<( const Fluent& lhs, const Fluent& rhs ) {

	if ( lhs.first < rhs.first ) return true;
	if ( lhs.first == rhs.first ) {
		if ( lhs.second < rhs.second ) return true;
		return false;
	}
	return false;

}

inline
unsigned fluent_index( const Fluent& p ) {

	register unsigned x = p.first;
	x = x << 8;
	x = x | p.second;
	return x;
}

inline
unsigned fluent_pair_index( Fluent p1, Fluent p2 ) {
	// sort fluents
	if ( p2 < p1 ) std::swap( p1, p2 );
	register unsigned x = fluent_index(p1);
	x = x << 16;
	x = x | fluent_index(p2);
	return x;
}

class IW2Search : public SearchTree {
    public:
	IW2Search(RomSettings *, Settings &settings, ActionVect &actions, StellaEnvironment* _env);

	virtual ~IW2Search();

	virtual void build(ALEState & state);
		
	virtual void update_tree();
	virtual int  expand_node( TreeNode* n, queue<TreeNode*>& q ); 

	int expanded() const { return m_expanded_nodes; }
	int generated() const { return m_generated_nodes; }
	int pruned() const { return m_pruned_nodes; }
	virtual unsigned max_depth(){ 	
		for (size_t c = 0; c < p_root->v_children.size(); c++)
			if(m_max_depth <  p_root->v_children[c]->branch_depth)
				m_max_depth =   p_root->v_children[c]->branch_depth;

			return m_max_depth;
	}
	virtual	void print_frame_data( int frame_number, float elapsed, Action curr_action, std::ostream& output );
protected:	

	void print_path(TreeNode *start, int a);

	virtual void expand_tree(TreeNode* start);

	void update_branch_return(TreeNode* node);

    	void set_terminal_root(TreeNode* node); 

	void	update_novelty_table( const ALERAM &machine_state );
	bool	check_novelty( const ALERAM& machine_state );

	virtual void	clear();
	virtual void	move_to_best_sub_branch();
	
	ALERAM 				m_ram;
	unsigned			m_pruned_nodes;
	bool				m_stop_on_first_reward;
	unsigned			m_reward_horizon;	

	typedef 			std::map< unsigned, bool >		Novelty_Table;
	aptk::Bit_Array			m_ram_novelty_table;
	aptk::Bit_Matrix*		m_ram_novelty_table_bool;
	bool                            m_novelty_boolean_representation;
};



#endif // __IW_2_SEARCH_HPP__
