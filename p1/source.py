'''
To keep version consistencies (for submitty) as much as possible, 

try to use built-in libraries only, i.e. don't use Numpy, ...


'''


import maths
import collections # in std lib
from collections import OrderedDict # the keys will not be sorted
import random
import math
import sys


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
		random.seed(self.seed)

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
		self.process_queue = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 
								'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'][:self.n]

		self.process_cpuburstTime = dict(zip(self.process_queue, [[] for i in range(self.n)])) #  these are not stored in the pcb
		self.process_ioburstTime = dict(zip(self.process_queue, [[] for i in range(self.n)])) # 

		self.tcs = kwargs["tcs"]  
			# argv[5]
			# time takes to perform a context switch
			# a context switch occurs each time a process leaves the CPU and is replaced by another process
			# Note that the ﬁrst half of the context switch time (i.e., tcs 2 ) is the time required to remove 
			# the given process from the CPU; 
			# the second half of the context switch time is the time required to bring the next process in 
			# to use the CPU. Therefore, expect tcs to be a positive even integer
		self.alpha = kwargs["alpha"]
			# argv[6]
			# for SJF and SRT
			# exponential averaging to predict CPU burst times
			# initial guess is t_0 = 1/lambda
		self.t_slice = kwargs["t_slice"]
			# argv[7]
			# time slice for RR

		self.rr_add = kwargs["rr_add"]
			# argv[8]
			# for RR: define whether arriving jobs or I/O completed jobs are added to the beginning/end of the ready queue
			# default: end

		# data structures
		self.ready_queue = []
		self.mini_pcb = OrderedDict() 
		#####

		# temporatory variables

		self.system_clock = 0 # to get the absolute time-stamp for a process's arrival

	def check(self):
		'''
		for debug
		'''

		# check: <=1 process is at the cpu, running
		runnning = 0
		for pid, record in self.mini_pcb.items():
			if record["state"] == 0:
				running += 1
		if runnning > 1:
			print("check: more than 1 process at cpu.\n", file=sys.stderr)
			exit(-1)

		# check: 
		
	def at_io_block(self, pid):
		self.mini_pcb[pid]["total_io_t"] += 1
		self.mini_pcb[pid]["remain_current_io_burst_t"] -= 1

	def at_ready_queue(self, pid):
		self.mini_pcb[pid]["total_wait_t"] += 1

	def at_cpu(self, pid):
		self.mini_pcb[pid]["total_cpu_t"] += 1
		self.mini_pcb[pid]["total_current_cpu_t"] += 1
		self.mini_pcb[pid]["remain_current_cpu_burst_t"] -= 1

	def at_context_switch(self, pid):
		'''
		'''
		self.mini_pcb[pid]["remain_current_contextSwitch"] -= 0.001

	def send_to_cpu(self, pid):
		'''
		update cpu
		'''
		self.mini_pcb[pid]["status"] = 0
		record["total_current_cpu_t"] = 0

	def send_to_io_block(self, pid):
		self.mini_pcb[pid]["status"] = 2

	def send_to_context_switch(self, pid, direction):
		'''
		context switch happens:
			- preempted
			- a cpu burst finished and process enter io

		set state
		3: 2cpu 
		4: cpu2ready
		5: cpu2io
		'''  # todo: context switch in millisecond 
		self.mini_pcb[pid]["total_cxtswitch_no"] += 1

		if direction is "2cpu":
			self.mini_pcb[pid]["remain_current_contextSwitch"] = self.tcs # wait for the previous process to be switched out + self switched in
		else:
			self.mini_pcb[pid]["remain_current_contextSwitch"] = self.tcs / 2 # only to be switched out of the cpu

		if direction == "2cpu":
			self.mini_pcb[pid]["state"] = 3	
		elif direction == "cpu2ready":
			self.mini_pcb[pid]["state"] = 4
		elif direction == "cpu2io":
			self.mini_pcb[pid]["state"] = 5
		else:
			print("send_to_context_switch: not implemented! \n")


	def send_to_ready_queue(self, pid):
		self.mini_pcb[pid]['state'] = 1
		self.ready_queue_add(pid)

	def ready_queue_add(self, pid):
		'''
		Only modify the ready queue, do nothing to the pcb
		For pre-emptive scheduling algos:
			SJF, SRF, RR
		'''

		# add to ready queue
		if self.algo == "RR":
			if self.rr_add == 'BEGINNING':
				self.ready_queue = [pid] + self.ready_queue
			elif self.rr_add == 'END':
				self.ready_queue.append(pid)
			else:
				print("ready_queue_add: RR: not implemented!")

		elif self.algo == "SJF":
			l = sorted(zip(self.ready_queue, [self.mini_pcb[pid]["current_cpu_burst_t_est"] for pid in self.ready_queue]), key=lambda t:t[1]) # current burst time
			self.ready_queue = [pid for (pid, t) in l]
		elif self.algo == "SRF":
			l = sorted(zip(self.ready_queue, [self.mini_pcb[pid]["remain_current_cpu_burst_t_est"] for pid in self.ready_queue]), key=lambda t:t[1]) # current remaining burst time
			self.ready_queue = [pid for (pid, t) in l]
		else
			print("ready_queue_add: not implemented, check! \n")

	def preempte(self, pid_in, pid_out):

		self.mini_pcb[pid_out]["preempt_no"] += 1
		self.send_to_context_switch(pid_in, '2cpu')
		self.send_to_context_switch(pid_out, 'cpu2ready')

	def cpu_burst_finish(self, pid):
		self.mini_pcb[pid]["remain_cpu_burst_no"] -= 1

		if self.mini_pcb[pid]["remain_cpu_burst_no"] == 0:
			self.process_finish(pid)
		else:
			# send to context switch
			self.send_to_context_switch(pid, 'cpu2io')

			# get new io burst time
			self.mini_pcb[pid]["current_io_burst_t"] = self.process_cpuburstTime[pid][
																- (self.mini_pcb[pid]["remain_cpu_burst_no"] - 1)]
			self.mini_pcb[pid]["remain_current_io_burst_t"] = self.mini_pcb[pid]["current_io_burst_t"]

	def io_burst_finish(self, pid):
		# get new cpu burst time
		self.mini_pcb[pid]["current_cpu_burst_t_est"] = math.ceil(self.alpha * self.mini_pcb[pid]["current_cpu_burst_t"] + (1 - self.alpha) * self.mini_pcb[pid]["current_cpu_burst_t_est"])
		self.mini_pcb[pid]["current_cpu_burst_t"] = self.process_cpuburstTime[pid][
															- self.mini_pcb[pid]["remain_cpu_burst_no"]]

		self.mini_pcb[pid]["remain_current_cpu_burst_t"] = self.mini_pcb[pid]["current_cpu_burst_t"]
		self.mini_pcb[pid]["remain_current_cpu_burst_t_est"] = self.mini_pcb[pid]["current_cpu_burst_t_est"]

		self.send_to_ready_queue(pid)

	def context_switch_finish(self, pid):
		'''
		set state
		3: 2cpu 
		4: cpu2ready
		5: cpu2io
		'''
		if self.mini_pcb[pid]["state"] == 3:
			self.send_to_cpu(pid)
		elif self.mini_pcb[pid]["state"] == 4:
			self.send_to_ready_queue(pid)
		elif self.mini_pcb[pid]["state"] == 5:
			self.send_to_io_block(pid)
		else:
			print("context_switch_finish: not implemented! \n")

	def process_finish(self, pid):
		'''
		'''

		self.mini_pcb[pid]["leave_t"] = self.system_clock

	def pcb_add_job(self, pid):
		'''
		General for all scheduling algorithms.
		Some entries might be left empty for now 
		For extension, add one job(process) at a time

		:param job []
		:param pid

		add these data entries:		
		cpu_burst_no 		int 	simulated	const
		cpu_burst_t  int 	simulated	const
		io_burst_t 	int 	simulated	const

		arrival_t			int 	system; debugging
		
		remain_cpu_t  		int 	calculated; update
		remain_cpu_burst_no	int 	calculated; update
		remain_cpu_burst_t	int 	calculated; update



		'''

		'''
			mini_pcb

			{PID: dict{"		cpu_burst_no 		int 	simulated	const 	no. of CPU bursts
								interarrival_t   	int 	simulated 	const 	
								
								current_cpu_burst_t  int  	simulated	const 	
								current_cpu_burst_t_est  int  	calculated	const 	
								current_io_burst_t 	int  	simulated	const

								state  				(0 for running; 1 for ready; 2 for I/O blocked; 
														3 for cpu2io-context-switch)
														4 for io2cpu-context-switch)
								
								remain_cpu_burst_no	int 	calculated; update
								
								remain_current_cpu_burst_t	int 	calculated; update
								remain_current_cpu_burst_t_est	int 	calculated; update
								remain_current_io_burst_t	int 	calculated; update
								remain_current_contextSwitch

								arrival_t			int 	system; debugging
								leave_t				int 	system; debugging
								total_io_t			int 	update; debugging
								total_cpu_t			int 	update; debugging; output
								total_current_cpu_t			int 	update; 
								total_wait_t		int 	update; debugging; output
								total_turnaround	int 	system; debugging; output
								total_cxtswitch_no	int 	update; debugging; output

								preempt_no.       int 	update; output

						"}, 
				...
			}

		'''
		# place holders
		self.mini_pcb.setdefault(pid, dict())
		data = ["state",
				"cpu_burst_no", "remain_cpu_burst_no", 
				"interarrival_t", "arrival_t", "leave_t"
				"current_cpu_burst_t", "current_cpu_burst_t_est", 
				"current_io_burst_t", 
				"remain_current_cpu_burst_t", "remain_current_cpu_burst_t_est",
				"remain_current_contextSwitch",
				"total_turnaround", "total_cpu_t", "total_wait_t", "total_io_t", "total_cxtswitch_no", "preempt_no",
				]
		for d in data:
			self.mini_pcb[pid].setdefault(d, -1)

		# initialization
		self.mini_pcb[pid]["interarrival_t"] = math.floor( -math.log(random.random()) / self.lambda_ )
		self.system_clock += self.mini_pcb[pid]["interarrival_t"]
		self.mini_pcb[pid]["arrival_t"] = self.system_clock
		self.mini_pcb[pid]["cpu_burst_no"] = math.floor(random.random() * 100) + 1 # [1, 100]
		self.mini_pcb[pid]["remain_cpu_burst_no"] = self.mini_pcb[pid]["cpu_burst_no"]


		for i in range(self.mini_pcb[pid]["cpu_burst_no"]): # todo: check if the burst times are pre-generated for each process this way; or all the processes getting from a same pool 
			self.process_cpuburstTime[pid].append( math.ceil( -math.log(random.random()) / self.lambda_ )) # cpu burst
			self.process_ioburstTime[pid].append( math.ceil( -math.log(random.random()) / self.lambda_ )) # io burst
		self.process_ioburstTime[pid] = self.process_ioburstTime[pid][:-1]# io burst

		self.mini_pcb[pid]["current_cpu_burst_t_est"] = math.ceil(1/self.lambda_)
		self.mini_pcb[pid]["current_cpu_burst_t"] = self.process_cpuburstTime[pid][0]
		self.mini_pcb[pid]["current_io_burst_t"] = self.process_ioburstTime[pid][0]
		self.mini_pcb[pid]["remain_current_cpu_burst_t_est"] = self.mini_pcb[pid]["current_cpu_burst_t_est"]
		self.mini_pcb[pid]["remain_current_cpu_burst_t"] = self.mini_pcb[pid]["current_cpu_burst_t"]


	def inter_arrive_time(self)

		while(1):
			t = math.floor( - math.log(random.random()) / self.lambda_ )
			if t < self.upper_bound_t:
				break

		return t

	def tie_handler(self, pid1, pid2):
		'''
		Ties:
		(a) CPU burst completion; 
		(b) I/O burst completion (i.e., back to the ready queue); and then 
		(c) new process arrival.

		Break-rule:
		process ID order (alphabetically)

		return: pid_1 to preempte, pid_2 send to the cpu

		'''

		if pid1 < pid2:
			pid_1, pid_2 = pid2, pid1
		else:
			pid_1, pid_2 = pid1, pid2

		return pid_1, pid_2

	def compare(self, pid1, pid2, data):
		''' 
		For job pe-emptive algos
		Compare this burst time (not the total duration time!)
		and Dealing with ties

		return: pid_1 to preempte, pid_2 to be send to cpu
		'''
		if data == "job_time":
			if self.mini_pcb[pid1]["current_cpu_burst_t_est"] > self.mini_pcb[pid2]["current_cpu_burst_t_est"]:
				pid_1, pid_2 = pid1, pid2
			elif self.mini_pcb[pid1]["current_cpu_burst_t_est"] == self.mini_pcb[pid2]["current_cpu_burst_t_est"]:
				pid_1, pid_2 = self.tie_handler(pid1, pid2)
			else:
				pid_1, pid_2 = pid2, pid1

		elif data == "remain_job_time":
			if self.mini_pcb[pid1]["remain_current_cpu_burst_t_est"] > self.mini_pcb[pid2]["remain_current_cpu_burst_t_est"]:
				pid_1, pid_2 = pid1, pid2
			elif self.mini_pcb[pid1]["remain_current_cpu_burst_t_est"] == self.mini_pcb[pid2]["remain_current_cpu_burst_t_est"]:
				pid_1, pid_2 = self.tie_handler(pid1, pid2)
			else:
				pid_1, pid_2 = pid2, pid1

		else:
			print("compare: not implemented! \n")

		return pid_1, pid_2


	def compete(self, running_pid, arrive_pid):
		'''
		For job pre-emptive algos
		'''
		# first give an estimation of the arrive_pid's cpu burst time

		if self.algo == 'SJF':
			pid_out, pid_in = self.compare(running_pid, arrive_pid, 'job_time')
			self.preempte(pid_in, pid_out) 


		elif self.algo == 'SRF':
			pid_out, pid_in = self.compare(running_pid, arrive_pid, 'remain_job_time')
			self.preempte(pid_in, pid_out)

		else:
			print("compete: not implemented for these algos, check.\n")

	def job_preempte_strategy(self, running_pid, arrive_pid):
		'''
		dealing with new job arrival
		'''
		if self.algo in ['SRF', 'SJF']: # arrive pre-emptive 
			# compare with the current running one
			if running_pid != -1: # pre-empte
				self.compete(running_pid, arrive_pid)
		else:
			self.send_to_ready_queue(arrive_pid)

	def time_preempte_strategy(self, running_pid):
		if self.algo == 'RR': # time pre-emptive; 
			self.preempte(running_pid)
			self.send_to_cpu(self.mini_pcb[self.ready_queue[0]])
		else:
			pass

	def pcb_update(self, kwargs):
		'''
		what happened in the last sec:
		update at 1 sec granularity

		update the system clock as well as the pcb records

		:param event: the event to deal with now
				1  a new arrival process
		:param pid: if event = 1, the new arrival's pid

		'''

		# general update (for the last sec)
		# a "for-loop" update

		for pid, record in self.mini_pcb.items():
			if record["state"] == 0: # at cpu
				self.at_cpu(pid)
				if record["remain_current_cpu_burst_t"] == 0:
					self.cpu_burst_finish(pid)
						
			elif record["state"] == 1: # ready
				self.at_ready_queue()

			elif record["state"] == 2: # i/o blocked
				self.at_io_block()
				if record["remain_current_io_burst_t"] == 0: # finish io burst, add to ready queue
					self.io_burst_finish(pid)

			elif record["state"] in [3, 4, 5]: # context switch
				self.at_context_switch(pid)
				if record["remain_current_contextSwitch"] == 0:
					self.context_switch_finish(pid)
			else:
				print("pcb_update: Error state, check! \n")

		# perform some check
		self.check()

		# dealing with the running process
		# get the running pid
		running_pid = -1
		for pid, record in self.mini_pcb.items():
			if record["state"] == 0:
				running_pid = pid

		# 1. new arrivals (arrival pre-emptive)
		if kwargs[event] == 1: # a new process has arrived
			self.pcb_add_job(kwargs['pid'])
			self.job_preempte_strategy(running_pid, kwargs['pid'])
			
		# 2. time slice limit (time pre-emptive)
		if self.mini_pcb[running_pid]["total_current_cpu_t"] >= self.t_slice: # pre-empte
			self.time_preempte_strategy(running_pid)
			
		# If no process is running, send the head of the ready queue to cpu
		if running_pid == -1:
			self.send_to_cpu(self.mini_pcb[self.ready_queue[0]])

		return 

	def cpu_simulator(self, algo):
		'''

		'''
		self.algo = algo
		self.system_clock = 0
		random.seed(self.seed)

		job_coming = iter(self.process_queue)

		job_arrive = job_coming.next() # at time 0 
		self.pcb_add_job(job_arrive)
		self.pcb_update({'event':1, 'pid':job_arrive})
		next_job_in_sec = self.inter_arrive_time()

		while(1): # 1 millionsec(context-switch granularity) loop

			if next_job_in_sec == 0
				try:
					job_arrive = job_coming.next()
				except StopIteration:
					break
				self.pcb_update({'event':1, 'pid':job_arrive})
				next_job_in_sec = self.inter_arrive_time()
				continue

			self.pcb_update({'event':0, 'pid':0})

			self.system_clock += 0.001
			next_job_in_sec -= 0.001


if __name__ == "__main__":

	# read argvs from terminal
	# Example:
	# 	a.out 70   We will use a random number generator to determine the interarrival times of CPU bursts. 
			  # 0.001  To determine interarrival times, we will use an exponential distribution; therefore, the second command-line argument is parameter λ. 
			  # 3000   As part of the exponential distribution, the third command-line argument represents the upper bound for valid pseudo-random numbers
			  # 10   Deﬁne n as the number of processes to simulate.
			  # 8  Deﬁne tcs as the time, in milliseconds,  that it takes to perform a context switch
			  # 0.5   For the SJF and SRT algorithms, since we cannot know the actual CPU burst times beforehand, we will rely on estimates determined via exponential averaging (as discussed in class on 2/11). As such, this command-line argument is the constant α.
			  # 80 For the RR algorithm, deﬁne the time slice value, tslice, measured in milliseconds
			  # (additional one)   Also for the RR algorithm, deﬁne whether processes are added to the end or the beginning of the ready queue when they arrive or complete I/O. 
	# 


	schedulor = cpu_scheduling(kwargs)




