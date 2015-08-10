#ifndef __IW_DIJKSTRA_SEARCH_HPP__
#define __IW_DIJKSTRA_SEARCH_HPP__

#include "IW1Search.hpp"

class IW1DijkstraSearch : public IW1Search {
    public:
	IW1DijkstraSearch(RomSettings *, Settings &settings, ActionVect &actions, StellaEnvironment* _env);

	virtual ~IW1DijkstraSearch();


    class TreeNodeComparer
    {
    public:
	
	bool operator()( TreeNode* a, TreeNode* b ) const 
	{
	    if ( b->fn < a->fn ) return true;
	    return false;
	}
    };

	virtual int  expand_node( TreeNode* n, std::priority_queue<TreeNode*, std::vector<TreeNode*>, TreeNodeComparer >& q ); 

protected:	


	virtual void expand_tree(TreeNode* start);

	reward_t		m_max_reward;
};



#endif // __IW_DIJKSTRA_SEARCH_HPP__
