#!/usr/bin/python

import os
import sys
import glob
import pp
import benchmark

def run_instance( folder, command, agent, rom_path, i ) :    
        res_filename = os.path.join( folder, 'episode.%d'%(i+1) ) 
        log_filename = os.path.join( folder, 'fulllog.%d'%(i+1) ) 

        log = benchmark.Log( '%s'%(log_filename) )
	benchmark.run( command, 0,4096, log )        
        if not os.path.exists( 'episode.1' ) :
                with open(res_filename,'w') as output :
                        print >> output, "Agent crashed"

        else :
                os.system( 'mv episode.1 %s'%res_filename )

        if agent == 'random' : return
        trace_filename = 'episode.%d.trace'%(i+1)
        trace_filename = os.path.join( folder, trace_filename ) 
        os.system( 'mv %(agent)s.search-agent.trace %(trace_filename)s'%locals() )

def main() :

	if len(sys.argv) < 3 :
		print >> sys.stderr, "Missing parameters!"
		print >> sys.stderr, "Usage: ./evaluate_agents.py <path to ROMs> <num_cpus>"
		sys.exit(1)

	rom_path = sys.argv[1]
	num_cpus = int(sys.argv[2])
        
	if not os.path.exists( rom_path ) :
		print >> sys.stderr, "Could not find ROM:", rom_path
		sys.exit(1)

	# tuple of all parallel python servers to connect with
	ppservers = ()#address of server, default=localhost
	job_server = pp.Server(num_cpus,ppservers=ppservers,socket_timeout=2, secret="password") 

	print "Starting pp with", job_server.get_ncpus(), "workers"


	rom_bin_files = glob.glob( os.path.join( rom_path, '*.bin' ) )
	print >> sys.stdout, 'Found %d ROMs in %s'%(len(rom_bin_files), rom_path )

	games = []
	for filename in rom_bin_files :
		game_name = os.path.split(filename)[-1].replace('.bin','')
		games.append( (game_name, filename) )
		
	num_runs = 5

	agents = [ 	
			( 'iw1', '-player_agent search_agent -search_method iw1' )

		 ]



        command_template = './ale -display_screen false -discount_factor 0.995 -randomize_successor_novelty true -record_trajectory %(record)s -max_sim_steps_per_frame 150000 -random_seed %(i)d %(agent_cmd)s %(rom_path)s'

        inputs = []
	for game, rom_path in games :
		for agent, agent_cmd in agents :
			#if 'iw1' in agent and 'pong' not in game: continue
			folder = 'experiments_150k_reuse/%(game)s/%(agent)s'%locals()
			if not os.path.exists( folder ) :
				os.system( 'mkdir -p %(folder)s'%locals() )
			for i in range( 0, num_runs ) :
				record = "false"
				if i == 0:
					record = "true"
				if not os.path.exists(  os.path.join( folder, 'episode.%d'%(i+1) ) ):					
					inputs.append( (folder, command_template%locals(), agent, rom_path, i ) );
				
        print "jobs_scheduled:"
        print '\n'.join( str(input) for input in inputs )        
        print '\n'
        jobs = [ (input, job_server.submit(run_instance, input,(),("benchmark",)) )for input in inputs]
	for input, job in jobs:
		print "Output", input, "is", job()

	job_server.print_stats()
                            
if __name__ == '__main__' :
	main()
