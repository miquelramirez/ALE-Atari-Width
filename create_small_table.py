#!/usr/bin/python
import os
import sys
import csv
import math
import numpy

class Episode :
	
	def __init__( self, game, algorithm ) :
		self.game = game
		self.algorithm = algorithm
		self.score = None
		self.time = None
		self.expanded = None
		self.generated = None
		self.pruned = None
		self.depth = None
		self.tree_size = None
		self.decision_time = None
		self.branch_reward = None
		self.avg_num_branch_reward = None
		self.max_num_branch_reward = None
		self.crashed = False

	def __lt__( self, other ) :
		if self.score < other.score : return True
		return False
	def __str__(self):
		return  "game: "+str(self.game)+", alg: "+str(self.algorithm)+", time: "+str(self.time)+", score: "+str(self.score)+", expanded: "+str(self.expanded)+", generated : "+str(self.generated)+", pruned: "+str(self.pruned)+", depth: "+str(self.depth)+", tree_size: "+str(self.tree_size)+", decision_time: "+str(self.decision_time)+", branch reward: "+str(self.branch_reward)

def check_trace( score_file, trace_file, game, algorithm  ):

        e = Episode( game, algorithm )
        expanded = []
        generated = []
        pruned = []
        depth = []
        tree_size = []
        decision_time = []
        branch_reward = []
        num_branch_reward = []
        if os.path.exists(score_file) is True:
                with open( score_file ) as instream:
                        for line in instream :
                                if "score" in line: e.score = float(line.strip().split("score=")[1]) 
                                if "elapsed_time" in line: e.time = float(line.strip().split("elapsed_time=")[1].split(',')[0] )

        if os.path.exists(trace_file) is True:
                with open( trace_file ) as instream:
                        for line in instream :       
                                if "expanded" in line:
                                        if line.count("frame") > 1: continue #error due to printing two lines twice.
                                        expanded.append( int(line.strip().split("expanded=")[1].split(',')[0]) )
                                        generated.append( int(line.strip().split("generated=")[1].split(',')[0]) )
                                        depth.append( int(line.strip().split("depth_tree=")[1].split(',')[0] ) )
                                        tree_size.append( int(line.strip().split("tree_size=")[1].split(',')[0] ) )
                                        decision_time.append( float(line.strip().split("elapsed=")[1].split(',')[0] ) )
                                        branch_reward.append( float(line.strip().split("branch_reward=")[1].split(',')[0] ) )
                                        num_branch_reward.append(int(line.strip().split("num_branches_with_reward=")[1].split(',')[0] ) )

                                if "pruned" in line: pruned.append( int( line.strip().split("pruned=")[1].split(',')[0] )  )  
        e.expanded = numpy.mean(expanded)
        e.generated = numpy.mean(generated)
        e.pruned = numpy.mean(pruned)
        e.depth = numpy.mean(depth)
        e.tree_size = numpy.mean(tree_size)
        e.decision_time = numpy.mean(decision_time)
        e.branch_reward = numpy.mean(branch_reward)
        e.avg_num_branch_reward = numpy.mean(num_branch_reward)
        e.max_num_branch_reward = max(num_branch_reward)
        return e
def retrieve_episodes( experiments_folder ) :
	episodes = []
	for root, dirs, files in os.walk('experiments_300_reuse') :
		game = os.path.split( root )[-2]
                if "/" in game:
                        game = game.split("/")[1]               

		algorithm = os.path.split(root)[-1]
		for f in files :
			if "trace" in f : continue
			if "fulllog" in f:
				full_path = os.path.join( root, f )
				with open( full_path ) as instream :
					e = Episode( game, algorithm )
					expanded = []
					generated = []
					pruned = []
					depth = []
					tree_size = []
					decision_time = []
					branch_reward = []
                                        num_branch_reward = []

					for line in instream :
						if "expanded" in line:
							expanded.append( int(line.strip().split("expanded=")[1].split(',')[0]) )
							generated.append( int(line.strip().split("generated=")[1].split(',')[0]) )
							depth.append( int(line.strip().split("depth_tree=")[1].split(',')[0] ) )
							tree_size.append( int(line.strip().split("tree_size=")[1].split(',')[0] ) )
							decision_time.append( float(line.strip().split("elapsed=")[1].split(',')[0] ) )
							branch_reward.append( float(line.strip().split("branch_reward=")[1].split(',')[0] ) )
                                                        num_branch_reward.append(int(line.strip().split("num_branches_with_reward=")[1].split(',')[0] ) )

						if "pruned" in line: pruned.append( int( line.strip().split("pruned=")[1].split(',')[0] )  )
						if "score" in line: e.score = float(line.strip().split("score: ")[1]) 
						if "Time spent" in line: e.time = float(line.strip().split("Time spent: ")[1].split(' ')[0] )
				e.expanded = numpy.mean(expanded)
				e.generated = numpy.mean(generated)
				e.pruned = numpy.mean(pruned)
				e.depth = numpy.mean(depth)
				e.tree_size = numpy.mean(tree_size)
				e.decision_time = numpy.mean(decision_time)
				e.branch_reward = numpy.mean(branch_reward)
                                e.avg_num_branch_reward = numpy.mean(num_branch_reward)
                                e.max_num_branch_reward = max(num_branch_reward)
				if e.score is None: 
                                        e = check_trace( root+"/episode."+f.split(".")[1], root+"/episode."+f.split(".")[1]+".trace",  game, algorithm)
                                        if e.score is None: 
                                                e.crashed = True


				episodes.append(e)
				#print e
			# if "episode" in f :
			# 	full_path = os.path.join( root, f )
			# 	with open( full_path ) as instream :
			# 		for line in instream :
			# 			if "Agent crashed" in line :
			# 				e = Episode( game, algorithm )
			# 				e.crashed = True
			# 				episodes.append( e )
			# 				break
			# 			time = line.strip().split('=')[1].split(',')[0]
			# 			score = line.strip().split('=')[2]
			# 			e = Episode( game, algorithm )
                        #                         e.score = float(score)
                        #                         e.time = time
			# 			episodes.append(e)
	return episodes

class Algorithm_Performance :

	def __init__( self ) :
		self.average = 0.0
		self.median = 0.0
		self.std_dev = 0.0
		self.avg_expanded = 0.0
		self.avg_generated = 0.0
		self.avg_pruned = 0.0
		self.avg_depth = 0.0
		self.avg_decision_time = 0.0
		self.avg_branch_reward = 0.0
		self.max_num_branch_reward = 0.0
		self.avg_num_branch_reward = 0.0
		self.episodes = []
		self.crashes = []

	def num_episodes( self ) :
		return len(self.episodes)
	
	def num_crashes( self ) :
		return len(self.crashes)

	def add_episode( self, episode ) :
		if episode.crashed :
			self.crashes.append(episode)
		else :
			self.episodes.append(episode)

	def compute_stats( self ) :
		if len( self.episodes ) == 0 : # no stats available
			self.average = 'n/a'
			self.median = 'n/a'
			self.std_dev = 'n/a'
			return
		# 1. Sort non-crashing episodes
		self.episodes.sort()
		# 2. compute average
		self.average = 0.0
		for e in self.episodes :
			self.average += e.score
		self.average /= float(self.num_episodes())

		self.avg_expanded = 0.0
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.expanded ) is False:
                                count+=1.0
                                self.avg_expanded += e.expanded	
		if count == 0.0: 
                        self.avg_expanded = 0 
                else:
                        self.avg_expanded /= count	

		self.avg_generated = 0.0
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.generated ) is False:
                                count+=1.0
                                self.avg_generated += e.generated	
		if count == 0.0: 
                        self.avg_generated = 0 
                else:
                        self.avg_generated /= count

		self.avg_pruned = 0.0
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.pruned ) is False:
                                count+=1.0
                                self.avg_pruned += e.pruned
                if count == 0.0: 
                        self.avg_pruned = 0 
                else:
                        self.avg_pruned /= count

		self.avg_depth = 0.0
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.depth ) is False:
                                count+=1.0
                                self.avg_depth += e.depth	
		if count == 0.0: 
                        self.avg_depth = 0 
                else:
                        self.avg_depth /= count

		self.avg_decision_time = 0.0
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.decision_time ) is False:
                                count+=1.0
                                self.avg_decision_time += e.decision_time
		if count == 0.0: 
                        self.avg_decision_time = 0 
                else:
                        self.avg_decision_time /= count

		self.avg_branch_reward = 0.0
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.branch_reward ) is False:
                                count+=1.0
                                self.avg_branch_reward += e.branch_reward
                if count == 0.0: 
                        self.avg_branch_reward = 0 
                else:
                        self.avg_branch_reward /= count

		self.avg_num_branch_reward = 0.0
                
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.avg_num_branch_reward ) is False:
                                count+=1.0
                                self.avg_num_branch_reward += e.avg_num_branch_reward
                if count == 0.0: 
                        self.avg_num_branch_reward = 0 
                else:
                        self.avg_num_branch_reward /= count

                self.max_num_branch_reward = 0.0
                count = 0.0
		for e in self.episodes :
                        if math.isnan( e.max_num_branch_reward ) is False:
                                count+=1.0
                                self.max_num_branch_reward += e.max_num_branch_reward
                if count == 0.0: 
                        self.avg_num_branch_reward = 0 
                else:
                        self.avg_num_branch_reward /= count

		# 3. compute median
		self.median = self.episodes[ self.num_episodes()/2 ].score
		# 4. compute standard deviation
		if len( self.episodes ) == 1 :
			self.std_dev = 'n/a'
			return
		self.std_dev = 0.0
		for e in self.episodes :
			# a. substract average
			s = e.score - self.average
			# b. square
			s *= s
			# c. sum
			self.std_dev += s
		# d. divide by n-1 to obtain the sample standard deviation
		self.std_dev /= float(self.num_episodes()-1)
		self.std_dev = math.sqrt( self.std_dev )
		

if __name__ == '__main__':

	episodes = retrieve_episodes('experiments_300_reuse')
	print >> sys.stdout, len(episodes), 'episodes retrieved'
	# the experimental data set is represented as a nested dictionary
	# { game, { algorithm, instance of Algorithm Performance } }
	games = {}
	
	for e in episodes :
		algorithms = None
		try :
			algorithms = games[e.game]
		except KeyError :
			games[e.game] = {}
			algorithms = games[e.game]
		perf = None
		try :
			perf = algorithms[e.algorithm]
		except KeyError :
			algorithms[e.algorithm] = Algorithm_Performance()
			perf = algorithms[e.algorithm]
		perf.add_episode( e )

	# now we compute the statistics
	for _, algs in games.iteritems() :
		for _, perf in algs.iteritems() :
			perf.compute_stats()

	#algorithms = [ 'random', 'brfs', 'iw1', 'uct', 'bfs' ]#, 'ucs' ] #,'iw1-ucs'
	algorithms = [ 'iw1', 'uct', 'bfs' ]#, 'ucs' ] #,'iw1-ucs'
	# and finally, we write the table summarizing the results
	with open( 'results.csv', 'w' ) as outstream :
		writer = csv.writer( outstream, delimiter = ',' )
		header = [ 'Game' ]
		for alg_name in algorithms :
			header += [ alg_name ] + [ '' ] * (10 )#+ len(algorithms)-1)
		writer.writerow( header )
		header = [ '' ]
		for alg_name in algorithms :
			header += [ 'runs', 'crashes', 'avg expanded','avg generated','avg pruned','avg depth', 'avg decision time', 'avg branch reward', 'max NUM branches with reward', 'avg NUM branches with reward','avg', 'median', 'std. dev.' ]
                        compare_to = algorithms[:]
                        compare_to.remove(alg_name)
                        #for c in compare_to:
                        #        header.append( alg_name +" > "+ c)
		writer.writerow( header )


		for game_name, algs in games.iteritems() :
                        #if 'bfs' not in algs.keys(): continue
			row = [ game_name ]
			for alg_name in algorithms :
				try :
                                        compare_to = algorithms[:]
                                        compare_to.remove(alg_name)

					perf = algs[ alg_name ]
					row += [ str( perf.num_episodes() ) ]
					row += [ str( perf.num_crashes() ) ]
					row += [ str( round(perf.avg_expanded,2) ) ]
					row += [ str( round(perf.avg_generated,2) ) ]
					row += [ str( round(perf.avg_pruned,2) ) ]
					row += [ str( round(perf.avg_depth,2) ) ]
					row += [ str( round(perf.avg_decision_time,2) ) ]
					row += [ str( round(perf.avg_branch_reward,2) ) ]
					row += [ str( perf.max_num_branch_reward ) ]
					row += [ str( round(perf.avg_num_branch_reward,2) ) ]
					row += [ str( perf.average ) ]
					row += [ str( perf.median ) ]                                        
					if perf.std_dev != 'n/a':
						row += [ str( round(perf.std_dev,2) ) ]
					else:
						row += [ str( perf.std_dev ) ]
                                        #for c in compare_to:
                                        #        row += [str( int(perf.median > games[ game_name ][ c ].median ) )]
				except KeyError :
					row += [ 'n/a' ] * 11
			writer.writerow( row )


