'''
To keep version consistencies (for submitty) as much as possible, 
try to use built-in libraries only


'''


import maths
import collections
from collections import OrderedDict # the keys will not be sorted





class cpu_scheduling:
	'''
	A cpu simulator.

	termiologies:

	* CPU burst time - for a process actually using the CPU 
	* (I/O burst) - 
	* Turnaround time - |-1/2 context switch (in)-|___cpu___|--context switch---|__wait___...|1/2 context switch (out)| 
	* Wait time - time spent in the ready queue


	'''

	def __init__(kwargs):
		'''


		'''

		# line arguments
		self.seed = kwargs["seed"] 
			# argv[1] 
			# 48-bit linear congruential generater
			# We will use a random number generator to determine the interarrival times of CPU bursts.
			# use srand48() with this given seed before each scheduling algorithm 
			# and drand48() to obtain the next value in the range [0.0,1.0). 
			# For other languages, implement an equivalent 48-bit linear congruential generator, 
			# as described in the man page for these functions. 
		self.lambda_ = kwargs["lambda"]
			# argv[2]
			# interarrival time as exponential distribution
			# use inverse transformation method: 
			# 	u ~ U(0, 1), get u and calculate x using u = CDF(x), CDF(x) = 1 - exp(-lambda * x)
			# 	then we get x ~ E(lambda) as the interarrival time  
		self.upper_bound_t = kwargs["upper_bound_t"]
			# argv[3]
			# if the x > upper_bound_t, then discard this x (we don't want infinitely small time intervals)
		self.n = kwargs["n"] 
			# argv[4]
			# number of processes to simulate
		 	# Process IDs are assigned in alphabetical order A through Z. 
		 	# Therefore, at most you will have 26 processes to simulate
		self.tcs = kwargs["tcs"]  
			# argv[5]
			# time takes to perform a context switch
			# a context switch occurs each time a process leaves the CPU and is replaced by another process
			# Note that the ﬁrst half of the context switch time (i.e., tcs 2 ) is the time required to remove 
			# the given process from the CPU; 
			# the second half of the context switch time is the time required to bring the next process in 
			# to use the CPU. Therefore, expect tcs to be a positive even integer
		self.exp_avg = kwargs["exp_avg"]
			# argv[6]
			# for SJF and SRT
			# exponential averaging to predict CPU burst times
			# initial guess is t_0 = 1/lambda
		self.t_slice = kwargs["t_slice"]
			# argv[7]
			# time slice for RR

		self.rr_add = kwargs["rr_add"]
			# argv[8]
			# define whether arriving jobs or I/O completed jobs are added to the beginning/end of the ready queue
			# default: end

		# data structures
		self.ready_queue = []
		self.mini_pcb = OrderedDict()
			'''
			{<PID: namedTuple("	cpu_burst_no 		int 	simulated	const
								interarrival_t   	int 	simulated 	const
								actual_cpu_burst_t  int 	simulated	const
								actual_io_burst_t 	int 	simulated	const

								expect_cpu_burst_t  int 	calculated; 
								state  				(0 for ready; 1 for running; 2 for I/O blocked)
								
								remain_cpu_t 		int 	calculated; update
								remain_cpu_burst_no	int 	calculated; update
								remain_cpu_burst_t	int 	calculated; update

								arrival_t			int 	system; debugging
								leave_t				int 	system; debugging
								total_io_t			int 	update; debugging
								total_cpu_t			int 	update; debugging; output
								total_wait_t		int 	update; debugging; output
								total_turnaround	int 	system; debugging; output
								total_cxtswitch_no	int 	update; debugging; output

								preempted no.       int 	update; output

								")>}
			'''


	def jobs_simulation():
		'''
		Generating 4 block data entries for a new job.

		For each of the n processes, in order A through Z:
		(* 1)1. Identify the initial process arrival time as the next random number in the sequence
		(* 1)2. Identify the number of CPU bursts for the given process as the next random number multiplied by 100 and truncated (e.g., 0.454928 becomes 45, 0.087188 becomes 8, etc.)
		(* 2)3. For each of these CPU bursts, identify the actual CPU burst time and the I/O burst time as the next two random numbers in the sequence; 
		   for the last CPU burst, do not generate an I/O burst time (since each process ends with a ﬁnal CPU burst)
		'''

		# get the 4 block data for the process coming in 
		t_arrive = inter_arrive_time(self.rand_queue.next())	
		cpu_burst_no = int(self.rand_queue.next() * 100)
		cpu_burst_time = self.rand_queue.next() 
		io_burst_time = self.rand_queue.next()

		job = [t_arrive, cpu_burst_no, cpu_burst_time, io_burst_time]

			
		return job

	def pcb_add_job(job, pid):
		'''
		:param job []
		:param pid

		add these data entries:		
		cpu_burst_no 		int 	simulated	const
		interarrival_t   	int 	simulated 	const
		actual_cpu_burst_t  int 	simulated	const
		actual_io_burst_t 	int 	simulated	const

		arrival_t			int 	system; debugging
		
		remain_cpu_t  		int 	calculated; update
		remain_cpu_burst_no	int 	calculated; update
		remain_cpu_burst_t	int 	calculated; update



		'''
		

		self.mini_pcb[pid] ...




	def inter_arrive_time(s)

		return - math.log(s)/float(self.lambda_)


	def tie_handler():
		'''
		Ties:
		(a) CPU burst completion; 
		(b) I/O burst completion (i.e., back to the ready queue); and then 
		(c) new process arrival.

		Break-rule:
		process ID order (alphabetically)

		'''

	def firstComeFirstServed():
		'''
		Non-preemptive

		No waiting queue is maintained
		'''
		job, pid = getNewJob()

		while job:

			pcb_add_job(job, pid)

			while(1):
				if self.mini_pcb[pid].remaining_cpu_burst != 0:
					# switch in CPU: Q: about the first process?
					# clock in 
					while(1):
						# context switch
						break  
					# clock out; update "total_cxtswitch_no"
					
					# One CPU burst
					# clock in 
					while(1):
						# cpu doing some execution of the job
						break 
					# clock out; update "total_cpu_t, remain_cpu_t, remain_cpu_burst_no, remain_cpu_burst_t(0, non-preemptive)"

					# an cpu burst finish, switch to I/O
					# clock in
					while(1):
						# context switch
						break
					# clock out; update "total_cxtswitch_no"

					# I/O blocked
					# clock in 
					while(1):
						# I/O
						break 
					# clock out; update "total_io_t"

				else:
					# update these pcb data entries
					# leave_t, total_turnaround
					self.mini_pcb[pid].leave_t = ''
					break # job finished

				self.mini_pcb[pid].remaining_cpu_burst -= 1
				
			job, pid = getNewJob()


	def ShortestJobFirst(, PID_list):
		'''
		Preemptive

		Cannot know the actual CPU burst times before hand

		:param 

		'''


		interarrival_t = 0
		while(1):
			if interarrival_t == 0
				job = getNewJob()








	def ShortestRemainingTime():
		'''
		Preemptive

		Cannot know the actual CPU burst times before hand

		'''




	def RoundRubin():
		'''
		Preemptive

		give definite time-slice


		'''



	def run():





if __name__ == "__main__":

	# read argvs from terminal

	schedulor = cpu_scheduling(kwargs)




