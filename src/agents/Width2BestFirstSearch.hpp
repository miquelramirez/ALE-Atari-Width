#ifndef __WIDTH_2_BEST_FIRST_SEARCH_HPP__
#define __WIDTH_2_BEST_FIRST_SEARCH_HPP__

#include "IW1Search.hpp"
#include "IW2Search.hpp"

class Width2BestFirstSearch : public IW1Search  {
public:
    Width2BestFirstSearch(RomSettings *, Settings &settings, ActionVect &actions, StellaEnvironment* _env);

	virtual ~Width2BestFirstSearch();


    class TreeNodeComparerExploration
    {
    public:
	
	bool operator()( TreeNode* a, TreeNode* b ) const 
	{
		if ( b->novelty < a->novelty ) return true;
		else if( b->novelty == a->novelty && b->fn < a->fn ) return true;
		//else if( b->novelty == a->novelty && b->m_depth < a->m_depth ) return true;
		return false;
	}
    };


    class TreeNodeComparerExploitation
    {
    public:
	
	bool operator()( TreeNode* a, TreeNode* b ) const 
	{
	    if ( b->fn < a->fn ) return true;
	    //else if( b->fn == a->fn && b->m_depth < a->m_depth ) return true;
	    else if( b->fn == a->fn && b->novelty < a->novelty ) return true;
	    return false;
	}
    };

    virtual int  expand_node( TreeNode* n ); 

    void clear_queues(){
	    delete q_exploration;
	    delete q_exploitation;
	    q_exploration = new std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploration >;
	    q_exploitation = new std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploitation >;
    }

protected:	
	void	update_novelty_table_2( const ALERAM &machine_state );
	bool	check_novelty_2( const ALERAM& machine_state );

    void reset_branch(TreeNode* node);
    int  reuse_branch(TreeNode* node);
    unsigned size_branch(TreeNode* node);

    virtual void expand_tree(TreeNode* start);

    std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploration >* q_exploration;
    std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparerExploitation >* q_exploitation;

	virtual		void	clear();

    	reward_t		m_max_reward;
    	unsigned m_gen_count_novelty2;
    	unsigned m_gen_count_novelty1;
    	unsigned m_exp_count_novelty2;
    	unsigned m_exp_count_novelty1;
	
	aptk::Bit_Array		m_ram_novelty_table_2;

};



#endif // __IW_DIJKSTRA_SEARCH_HPP__
