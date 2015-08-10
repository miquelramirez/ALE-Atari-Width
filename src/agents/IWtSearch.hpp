#ifndef __IWt_SEARCH_HPP__
#define __IWt_SEARCH_HPP__

#include "SearchTree.hpp"
#include "bit_matrix.hxx"
#include "../environment/ale_ram.hpp"

#include <queue>

typedef std::pair< unsigned, unsigned> BitIndex;

class VarGroup {
 
public:
	VarGroup( ) : m_novelty_table(1), m_novelty_max_value(1) {
		m_last_num_values_reached = 1;
	};

	VarGroup( unsigned byte, unsigned bit) : m_novelty_table(1), m_novelty_max_value(1) {
		m_bit_indexes.push_back( std::make_pair( byte, bit) );
		m_last_num_values_reached = 1;
	};

	VarGroup( const VarGroup& v){
		m_bit_indexes = v.bit_indexes();
		m_novelty_table = v.novelty_table_size();
		m_last_num_values_reached = v.last_num_values_reached();
		m_novelty_max_value = v.m_novelty_max_value;
	}
	
	
	~VarGroup(){};
	
protected:

	unsigned get_value( const ALERAM& machine_state ){
		unsigned value = 0;
		unsigned bit_pos = 0;
		for( auto idx : m_bit_indexes ){
			value |= (machine_state.get( (size_t) idx.first, idx.second ) << bit_pos);			
			bit_pos++;
		}
		return value;
	}
public:
	       
	std::vector< BitIndex >& bit_indexes()  { return m_bit_indexes; }
	const std::vector< BitIndex >& bit_indexes() const { return m_bit_indexes; }

	unsigned novelty_table_size() const { return m_novelty_table.size(); }
	const aptk::Bit_Array& novelty_table() const { return m_novelty_table; }

	void add_bit_index( unsigned byte, unsigned bit){
		m_bit_indexes.push_back( std::make_pair( byte, bit) );
		m_novelty_max_value *= 2;
		m_novelty_table.resize( m_novelty_max_value );

	}
	
	void clear(){
		m_novelty_table.reset();
	}

	void update_novelty_table( const ALERAM& machine_state){
		m_novelty_table.set( get_value( machine_state ) );
	}

	unsigned is_novel( const ALERAM& machine_state ){
		return (! m_novelty_table.isset( get_value( machine_state ) ) );

	}
	
	void set_last_num_values_reached( unsigned n ){  m_last_num_values_reached = n; }
	const unsigned last_num_values_reached(  ) const {  return m_last_num_values_reached; }

	unsigned count_num_values_reached() { return (unsigned) m_novelty_table.count_elements(); }
	
protected:
	std::vector< BitIndex > m_bit_indexes;
	aptk::Bit_Array	        m_novelty_table;
	unsigned                m_last_num_values_reached;
	unsigned                m_novelty_max_value;
	
};

class IWtSearch : public SearchTree {
    public:
	IWtSearch(RomSettings *, Settings &settings, ActionVect &actions, StellaEnvironment* _env);

	virtual ~IWtSearch();

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

	void    set_candidates();
	bool    keep_candidates();
	void	update_novelty_table( const ALERAM &machine_state );
	bool	check_novelty( const ALERAM& machine_state );

	virtual void	clear();
	virtual void	move_to_best_sub_branch();
	
	ALERAM 			m_ram;
	unsigned		m_pruned_nodes;
	bool			m_stop_on_first_reward;
	unsigned		m_reward_horizon;	
	bool                    m_novelty_boolean_representation;
        unsigned                m_max_var_group;
	unsigned                m_num_simulated_steps;
        std::vector<VarGroup*>  m_var_groups;
	std::vector<VarGroup*>  m_candidate_var_groups;
	std::vector<VarGroup*>  m_ignored_candidate_var_groups;
};



#endif // __IW_SEARCH_HPP__
