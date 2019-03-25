'''
Manqing Ma
661963918
mam6@rpi.edu

To keep version consistencies (for submitty) as much as possible, 

try to use built-in libraries only, i.e. don't use Numpy, ...

'''


import collections # in std lib
from collections import OrderedDict # the keys will not be sorted
from collections import namedtuple
import random
import math
import sys, getopt
import copy

class cpu_scheduling:
	'''
	A cpu simulator.

	termiologies:

	* CPU burst time - for a process actually using the CPU 
	* (I/O burst) - 
	* Turnaround time - |-1/2 context switch (in)-|___cpu___|--context switch---|__wait___...|1/2 context switch (out)| 
	* Wait time - time spent in the ready queue


	'''

	def __init__(self, kwargs):
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
		self.process_queue = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', \
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
			# could be not given
			# argv[8]
			# for RR: define whether arriving jobs or I/O completed jobs are added to the beginning/end of the ready queue
			# default: end

		# data structures
		self.ready_queue = []
		self.mini_pcb = OrderedDict()
		#####

		# temporatory variables
		self.system_clock = 0 # to get the absolute time-stamp for a process's arrival
		self.running_pid = -1 # exclusive
		self.context_switch = 0 # exclusive
		self.timeslice_notice_flag = -1
		self.submitty = kwargs["submitty"]
		self.preempte_flag = 0

	def init_pcb(self):
		self.mini_pcb = OrderedDict()
		data = {"state":-1,
				"cpu_burst_no":-1, "remain_cpu_burst_no":-1,

				"interarrival_t":-1, "arrival_t":-1, "leave_t":-1,

				"current_cpu_burst_t":-1,
				"current_cpu_burst_t_est":-1,
				"current_io_burst_t":-1,
				"remain_current_io_burst_t":-1,
				"remain_current_cpu_burst_t":-1,
				"remain_current_cpu_burst_t_est":-1,
				"remain_current_contextSwitch":-1,


				# incrementation
				"total_current_cpu_t":0,
				"total_t":0,
				"total_cpu_t":0,
				"total_wait_t":0,
				"total_io_t":0,
				"total_cxtswitch_no":0,

				"preempt_no":0,
				#"preempted":0, # start value

				# some to-check entries
				"tau":-1
				}

		for pid in self.process_queue:
			# place holders
			#self.mini_pcb.setdefault(pid, dict())
			self.mini_pcb.setdefault(pid, copy.deepcopy(data))

	def msg(self, pid, info):
		'''
    	:param info:
			stdout:
			proc-in, simulator-start, proc-arrive, proc-start-cpu-burst, proc-complete-cpu-burst,
			proc-preempte
			proc-tau, proc-switch-out-cpu, proc-complete-io-burst, proc-terminate, simulator-end,

			file:
			simulator-summary
		:param pid:
		:return:
		'''
		def print_ready_queue():
			if len(self.ready_queue) != 0:
				tmp = [print(str(pid), end=' ') for pid in self.ready_queue[:-1]]
				print(str(self.ready_queue[-1]), end=']\n')
			else:
				print("<empty>]\n", end='')


		if info == "proc-in":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return

			if self.mini_pcb[pid]["cpu_burst_no"] > 1:
				print("Process %s [NEW] (arrival time %s ms) %s CPU bursts\n" %
				  (pid, self.mini_pcb[pid]["arrival_t"], self.mini_pcb[pid]["cpu_burst_no"]), end='')
			else:
				print("Process %s [NEW] (arrival time %s ms) %s CPU burst\n" %
					  (pid, self.mini_pcb[pid]["arrival_t"], self.mini_pcb[pid]["cpu_burst_no"]), end='')
		elif info == "simulator-start":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return

			print("time %sms: Simulator started for %s [Q " %
				  (self.system_clock, self.algo), end='')
			print_ready_queue()
		elif info == "proc-arrive":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			if self.algo in ['SJF', 'SRT']:
				print("time %sms: Process %s (tau %sms) arrived; "
					  "added to ready queue [Q " %
					  (self.system_clock, pid, self.mini_pcb[pid]["current_cpu_burst_t_est"]), end='')
			else:
				print("time %sms: Process %s arrived; "
					  "added to ready queue [Q " %
					  (self.system_clock, pid), end='')
			print_ready_queue()
		elif info == "proc-start-cpu-burst":
			# submitty
			if self.mini_pcb[pid]["current_cpu_burst_t"] == self.mini_pcb[pid]["remain_current_cpu_burst_t"]:
				if (self.system_clock > 999) & (self.submitty == 1):
					self.timeslice_notice_flag = 1
					return
				else:
					print("time %sms: Process %s started using the CPU for %sms burst [Q " %
						  (self.system_clock, pid, self.mini_pcb[pid]["current_cpu_burst_t"]), end='')
					self.timeslice_notice_flag = 1
			# set a variable to control RR "time slice expire; ...continue" output once/per burst
			else:
				if (self.system_clock > 999) & (self.submitty == 1):
					return
				print("time %sms: Process %s started using the CPU with %sms remaining [Q " %
				  (self.system_clock, pid, self.mini_pcb[pid]["remain_current_cpu_burst_t"]), end='')
			print_ready_queue()
		# set a variable to control RR "time slice expire; ...continue" output once/per burst
		elif info == "proc-complete-cpu-burst":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			if self.mini_pcb[pid]["remain_cpu_burst_no"] > 1:
				print("time %sms: Process %s completed a CPU burst; %s bursts to go [Q "%
				  (self.system_clock, pid, self.mini_pcb[pid]["remain_cpu_burst_no"]), end='')
			else:
				print("time %sms: Process %s completed a CPU burst; %s burst to go [Q " %
					  (self.system_clock, pid, self.mini_pcb[pid]["remain_cpu_burst_no"]), end='')

			print_ready_queue()
		elif info == "proc-tau":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			if self.algo in ['SRT', 'SJF']:
				print("time %sms: Recalculated tau = %sms for process %s [Q "%
					  (self.system_clock, self.mini_pcb[pid]["current_cpu_burst_t_est"], pid), end='')
				print_ready_queue()
			else:
				pass
		elif info == "proc-preempte-io": # for SRT, use tau for estimation
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			print("time %sms: Process %s (tau %sms) completed I/O and will preempt %s [Q "%
				  (self.system_clock, pid, self.mini_pcb[pid]["current_cpu_burst_t_est"], self.running_pid), end='')
			print_ready_queue()
		elif info == "proc-preempte-queue": # for SRT, use tau for estimation
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			print("time %sms: Process %s (tau %sms) will preempt %s [Q "%
				  (self.system_clock, pid, self.mini_pcb[pid]["current_cpu_burst_t_est"], self.running_pid), end='')
			print_ready_queue()
		elif info == "proc-preempte-arrival":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			pass # todo
		elif info == "proc-switch-out-cpu":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			print("time %sms: Process %s switching out of CPU; will block on I/O until time %sms [Q " %
				  (self.system_clock, pid, self.system_clock + self.mini_pcb[pid]["current_io_burst_t"] + int(self.tcs/2)), end='')
			print_ready_queue()
		elif info == "proc-timeslice-switch-out":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			print("time %sms: Time slice expired; process %s preempted with %sms to go [Q " %
				  (self.system_clock, pid, self.mini_pcb[pid]["remain_current_cpu_burst_t"]), end='')
			print_ready_queue()
		elif info == "proc-timeslice-continue":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				self.timeslice_notice_flag = 0
				return

			if self.timeslice_notice_flag == 1:
				print("time %sms: Time slice expired; no preemption because ready queue is empty [Q " %
				  (self.system_clock), end='')
				print_ready_queue()
			self.timeslice_notice_flag = 0

		elif info == "proc-complete-io-burst":
			# submitty
			if (self.system_clock > 999) & (self.submitty == 1):
				return
			if self.algo in ['SJF', 'SRT']:
				print("time %sms: Process %s (tau %sms) completed I/O; added to ready queue [Q " %
				  (self.system_clock, pid, self.mini_pcb[pid]["current_cpu_burst_t_est"]), end='')
			else:
				print("time %sms: Process %s completed I/O; added to ready queue [Q " %
					  (self.system_clock, pid), end='')

			print_ready_queue()
		elif info == "proc-terminate":
			print("time %sms: Process %s terminated [Q " %
				  (self.system_clock, pid), end='')
			print_ready_queue()
		elif info == "simulator-end":
			print("time %sms: Simulator ended for %s [Q " %
				  (self.system_clock, self.algo), end='')
			print_ready_queue()
		elif info == "simulator-summary":
			total_burst_no = sum([self.mini_pcb[pid]["cpu_burst_no"] for pid in self.process_queue])

			avg_cpu_burst = sum([self.mini_pcb[pid]["total_cpu_t"] for pid in self.process_queue]) / \
								float(total_burst_no)

			avg_wait_time = sum([self.mini_pcb[pid]["total_wait_t"] for pid in self.process_queue]) / \
							float(total_burst_no)

			total_cxtswitch_no = sum([self.mini_pcb[pid]["total_cxtswitch_no"] for pid in self.process_queue]) / 2 # in + out
			total_preemption_no = sum([self.mini_pcb[pid]["preempt_no"] for pid in self.process_queue])

			if self.algo in ["FCFS", "SJF"]: # non-preemptive
				avg_turnaround_time = avg_cpu_burst + avg_wait_time + self.tcs
			else:
				avg_turnaround_time = ((avg_cpu_burst + avg_wait_time + self.tcs) * total_burst_no +  \
									  (self.tcs * total_preemption_no)) / float(total_burst_no)

			total_preemption_no = sum([self.mini_pcb[pid]["preempt_no"] for pid in self.process_queue])

			if total_preemption_no < 0:
				total_preemption_no = 0
			print('''Algorithm %s\n'''
					'''-- average CPU burst time: %.3f ms\n'''
					'''-- average wait time: %.3f ms\n'''
					'''-- average turnaround time: %.3f ms\n'''
					'''-- total number of context switches: %d\n'''
					'''-- total number of preemptions: %d\n''' %
				  (self.algo, avg_cpu_burst, avg_wait_time, avg_turnaround_time, total_cxtswitch_no, total_preemption_no), end=''
				  ,file=open("./simout.txt", 'a+'))
		else:
			print("ERROR: msg : %s not implemented! \n" % info, file=sys.stderr)


	
	def check(self):
		'''
		for debug

		Very time consuming
		'''

		# check:
		# assert <=1 process is at the cpu, running
		# return 1 if either job at cpu OR job at context switch to cpu
		self.running_pid = -1
		running = 0
		cxtSwitch_2cpu2 = 0
		for pid in self.mini_pcb.keys():
			if self.mini_pcb[pid]["state"] == 0:
				running += 1
				self.running_pid = pid
			elif self.mini_pcb[pid]["state"] in [3, 4, 5, 6]: # 2cpu & any cpu2 context switch
				cxtSwitch_2cpu2 += 1
		if (running + cxtSwitch_2cpu2) > 1:
			print("check: more than 1 process (at cpu + switching into cpu) .\n", file=sys.stderr)
			exit(-1)

		return running + cxtSwitch_2cpu2

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
		self.mini_pcb[pid]["remain_current_cpu_burst_t_est"] -= 1

	def at_context_switch(self, pid):
		'''
		'''
		self.mini_pcb[pid]["remain_current_contextSwitch"] -= 1

	def send_to_cpu(self, pid):
		'''
		from context switch
		'''
		self.preempte_flag = 0
		self.mini_pcb[pid]["state"] = 0
		self.mini_pcb[pid]["total_current_cpu_t"] = 0

	def send_to_io_block(self, pid):
		self.mini_pcb[pid]["state"] = 2

	def send_to_context_switch(self, pid, direction):
		'''
		context switch happens:
			- preempted
			- a cpu burst finished and process enter io

		set state
		3: 2cpu 
		4: cpu2ready
		5: cpu2io
		6: cpu2end
		
		'''
		self.mini_pcb[pid]["total_cxtswitch_no"] += 1

		if direction == "2cpu":
			self.mini_pcb[pid]["state"] = 3
		elif direction == "cpu2ready":
			self.mini_pcb[pid]["state"] = 4
		elif direction == "cpu2io":
			self.mini_pcb[pid]["state"] = 5
		elif direction == "cpu2end":
			self.mini_pcb[pid]["state"] = 6
		else:
			print("send_to_context_switch: not implemented! \n")

		self.mini_pcb[pid]["remain_current_contextSwitch"] = self.tcs / 2  # only to be switched in the cpu

		self.context_switch = 1


	def send_to_ready_queue(self, pid):
		self.mini_pcb[pid]['state'] = 1
		self.ready_queue_add(pid)

	def ready_queue_pop(self):
		pid = self.ready_queue[0]
		self.ready_queue = self.ready_queue[1:]
		return pid

	
	def ready_queue_add(self, pid):
		'''
		Only modify the ready queue, do nothing to the pcb
		For pre-emptive scheduling algos:
			SJF, SRT, RR
		'''

		# add to ready queue
		if self.algo == "RR":
			if self.rr_add == 'BEGINNING':
				self.ready_queue = [pid] + self.ready_queue
			elif self.rr_add == 'END':
				self.ready_queue.append(pid)
			else:
				self.ready_queue.append(pid)  # default FCFS
		elif self.algo == "SJF":
			self.ready_queue.append(pid)
			l = zip(self.ready_queue, [self.mini_pcb[pid]["current_cpu_burst_t_est"] for pid in self.ready_queue])
			l = sorted(l, key = lambda t: t[0]) # first sort by pid 'A', 'B', ... # implicit tie handler
			l = sorted(l, key = lambda t: t[1]) # second sort by current burst time
			self.ready_queue = [pid for (pid, t) in l]
		elif self.algo == "SRT":
			self.ready_queue.append(pid)
			l = zip(self.ready_queue, [self.mini_pcb[pid]["remain_current_cpu_burst_t_est"] for pid in self.ready_queue])
			l = sorted(l, key = lambda t: t[0]) # first sort by pid 'A', 'B', ... # implicit tie handler
			l = sorted(l, key = lambda t: t[1]) # second sort by remain current burst time
			self.ready_queue = [pid for (pid, t) in l]
		elif self.algo == "FCFS":
			self.ready_queue.append(pid)
		else:
			print("ready_queue_add: %s not implemented, check! \n" % self.algo)

	def preempte(self, pid_in, pid_out):
		'''
		head of the ready queue pre-empte the job on cpu

		:param pid_in:
		:param pid_out:
		:return:
		'''
		if self.preempte_flag == 0: #
			self.mini_pcb[pid_out]["preempt_no"] += 1
		#self.mini_pcb[pid_out]["preempted"] = 1
		self.preempte_flag = 1
		self.send_to_context_switch(pid_out, 'cpu2ready')

	def cpu_burst_finish(self, pid):
		self.mini_pcb[pid]["remain_cpu_burst_no"] -= 1

		if self.mini_pcb[pid]["remain_cpu_burst_no"] == 0:
			self.msg(pid, "proc-terminate") # doesn't count the last cpu burst?
			self.send_to_context_switch(pid, 'cpu2end')
		else:
			# send to context switch
			self.send_to_context_switch(pid, 'cpu2io')

			# get new io burst time
			self.mini_pcb[pid]["current_io_burst_t"] = self.process_ioburstTime[pid][
																- self.mini_pcb[pid]["remain_cpu_burst_no"]]
			self.mini_pcb[pid]["remain_current_io_burst_t"] = self.mini_pcb[pid]["current_io_burst_t"]

			# get new estimated cpu burst time
			self.mini_pcb[pid]["current_cpu_burst_t_est"] = math.ceil(
				self.alpha * self.mini_pcb[pid]["current_cpu_burst_t"] +
				(1 - self.alpha) * self.mini_pcb[pid]["current_cpu_burst_t_est"])
			self.mini_pcb[pid]["current_cpu_burst_t"] = self.process_cpuburstTime[pid][
				- self.mini_pcb[pid]["remain_cpu_burst_no"]]

			self.mini_pcb[pid]["remain_current_cpu_burst_t"] = self.mini_pcb[pid]["current_cpu_burst_t"]
			self.mini_pcb[pid]["remain_current_cpu_burst_t_est"] = self.mini_pcb[pid]["current_cpu_burst_t_est"]

			self.msg(pid, "proc-complete-cpu-burst")
			self.msg(pid, "proc-tau")
			self.msg(pid, "proc-switch-out-cpu")

		self.running_pid = -1

	def io_burst_finish(self, pid):

		if self.algo in ['SRT']:  # compare estimated burst time
			if self.running_pid != -1:
				self.send_to_ready_queue(pid)
				success = self.job_preempte_strategy(self.running_pid, pid, "from-io")
				if success == 0:
					self.msg(pid, "proc-complete-io-burst")
			else:
				self.send_to_ready_queue(pid)
				self.msg(pid, "proc-complete-io-burst")
		else:
			self.send_to_ready_queue(pid)
			self.msg(pid, "proc-complete-io-burst")

	def context_switch_finish(self, pid):
		'''
		set state
		3: 2cpu 
		4: cpu2ready
		5: cpu2io
		6: cpu2end
		'''
		if self.mini_pcb[pid]["state"] == 3:
			self.send_to_cpu(pid)
			self.running_pid = pid
			self.msg(pid, "proc-start-cpu-burst")
		elif self.mini_pcb[pid]["state"] == 4:
			self.send_to_ready_queue(pid) # happens when preempted; silent, no message
			# self.msg(pid, "proc-complete-io-burst") #
		elif self.mini_pcb[pid]["state"] == 5:
			self.send_to_io_block(pid)
		elif self.mini_pcb[pid]["state"] == 6:
			self.process_finish(pid)
		else:
			print("context_switch_finish: not implemented! \n")

		self.context_switch = 0 # only one process at the context switch

	def process_finish(self, pid):
		'''
		'''

		self.mini_pcb[pid]["leave_t"] = self.system_clock
		self.mini_pcb[pid]["state"] = 7
		self.mini_pcb[pid]["total_t"] += 1

	def pcb_add_job(self, pid):
		'''
		General for all scheduling algorithms.
		Some entries might be left empty for now 
		For extension, add one job(process) at a time

		:param job []
		:param pid
		'''

		# initialization
		self.mini_pcb[pid]["remain_cpu_burst_no"] = self.mini_pcb[pid]["cpu_burst_no"]

		self.mini_pcb[pid]["current_cpu_burst_t_est"] = math.ceil(1/self.lambda_) #tau, initial cpu burst time guess
		self.mini_pcb[pid]["current_cpu_burst_t"] = self.process_cpuburstTime[pid][0]
		try:
			self.mini_pcb[pid]["current_io_burst_t"] = self.process_ioburstTime[pid][0]
		except IndexError: # cpu burst no == 1; no io burst
			pass
		self.mini_pcb[pid]["remain_current_io_burst_t"] = self.mini_pcb[pid]["current_io_burst_t"]
		self.mini_pcb[pid]["remain_current_cpu_burst_t_est"] = self.mini_pcb[pid]["current_cpu_burst_t_est"]
		self.mini_pcb[pid]["remain_current_cpu_burst_t"] = self.mini_pcb[pid]["current_cpu_burst_t"]


	def exp_rand(self):

		while(1):
			t = - math.log(self.rand.drand()) / self.lambda_
			if t < self.upper_bound_t:
				break

		return t

	# def tie_handler(self, pid1, pid2): # tie is not used for pre-emption, check piazza @345
	# 	'''
	# 	Ties:
	# 	(a) CPU burst completion;
	# 	(b) I/O burst completion (i.e., back to the ready queue); and then
	# 	(c) new process arrival.
	#
	# 	Break-rule:
	# 	process ID order (alphabetically)
	#
	# 	return: pid_1 to preempte, pid_2 send to the cpu
	#
	# 	'''
	#
	# 	if pid1 < pid2:
	# 		pid_1, pid_2 = pid2, pid1
	# 	else:
	# 		pid_1, pid_2 = pid1, pid2
	#
	# 	return pid_1, pid_2

	def compare(self, running, pid2, data):
		''' 
		For job pe-emptive algos
		Compare this burst time (not the total duration time!)
		and Dealing with ties

		return: pid_1 to preempte, pid_2 to be send to cpu
		'''
		if data == "remain_job_time":
			if self.mini_pcb[running]["remain_current_cpu_burst_t_est"] > self.mini_pcb[pid2]["remain_current_cpu_burst_t_est"]:
				pid_out, pid_in = running, pid2
			else: # if <=, nothing changed
				pid_out, pid_in = pid2, running

		else:
			pid_out, pid_in = -1, -1
			print("compare: not implemented! \n")

		return pid_out, pid_in


	def compete(self, running, pid2):
		'''
		For job pre-emptive algos
		'''
		# first give an estimation of the arrive_pid's cpu burst time

		if self.algo == 'SRT':
			pid_out, pid_in = self.compare(running, pid2, 'remain_job_time')
			return (pid_out == self.running_pid)

		else:
			print("compete: not implemented for %s, check.\n" % self.algo)
			exit(-1)


	def job_preempte_strategy(self, running_pid, ready_queue_pid, source_info):
		'''
		SRT
		'''
		success = 0
		if self.algo in ['SRT']: #
			# compare with the current running one
			success = self.compete(running_pid, ready_queue_pid)
			if success:
				if self.preempte_flag == 0: # preempte only happen once for the kicked-out process
					if source_info == 'from-io':
						self.msg(ready_queue_pid, "proc-preempte-io") # from io burst
					else:
						self.msg(ready_queue_pid, "proc-preempte-queue") # from ready queue
					#todo: from arrival?
				else: # don't print preempte msg
					self.msg(ready_queue_pid, "proc-complete-io-burst")

				#
				self.preempte(ready_queue_pid, running_pid)  # in, out
				self.running_pid = -1

		return success

	def time_preempte_strategy(self, running_pid):
		'''
		RR
		:param running_pid:
		:return:
		'''
		if self.algo == 'RR': # time pre-emptive;
			if self.mini_pcb[running_pid]["total_current_cpu_t"] >= self.t_slice:  # pre-empte
				if len(self.ready_queue) > 0:
					# time slice expiration only checks once
					if self.timeslice_notice_flag == 1:
						self.msg(running_pid, "proc-timeslice-switch-out")
						self.preempte(self.ready_queue[0], running_pid)
						self.running_pid = -1 #
					else:
						pass
				else:
					self.msg(running_pid, "proc-timeslice-continue")

	
	def pcb_update(self, kwargs):
		'''
		what happened in this 1 sec:

		:param event: the event to deal with now
				1  a new arrival process
		:param pid: if event = 1, the new arrival's pid

		'''
		# ########Check context switch finish
		for pid, record in self.mini_pcb.items():
			if record["state"] in [3, 4, 5, 6]:  # context switch
				if record["remain_current_contextSwitch"] == 0:
					self.context_switch_finish(pid)
			if record['state'] == 0: # at cpu; cpu burst
				if record["remain_current_cpu_burst_t"] == 0:
					self.cpu_burst_finish(pid)

		#runningORcxtSwitch_cpu = self.check()

		# time slice limit: time pre-empte
		if self.algo in ['RR']:
			if self.running_pid != -1:
				self.time_preempte_strategy(self.running_pid)

		#runningORcxtSwitch_cpu = self.check()

		# then io burst finish
		for pid, record in self.mini_pcb.items():
			if record["state"] == 2:  # at i/o block; i/o burst
				if record["remain_current_io_burst_t"] == 0: #
					self.io_burst_finish(pid) # add to ready-queue/ preempte the running job

		# ########Check running pid

		#runningORcxtSwitch_cpu = self.check()

		# deal with new arrivals
		if kwargs["event"] == 1:  # a new process has arrived, send to ready queue
			self.pcb_add_job(kwargs['pid'])
			self.send_to_ready_queue(kwargs['pid'])
			self.msg(kwargs['pid'], "proc-arrive")

		# new ready queue added: new_arrivals: job pre-empte
		if self.running_pid != -1:
			if self.algo in ['SRT']:
				if len(self.ready_queue) > 0:
					self.job_preempte_strategy(self.running_pid, self.ready_queue[0], 'from-queue')

		#runningORcxtSwitch_cpu = self.check()

		if (self.running_pid == -1) & (self.context_switch == 0):
			# if no job is at cpu & no job is at context switch to cpu
			if len(self.ready_queue) != 0: # 0 if no job in the ready queue
				pid = self.ready_queue_pop()
				self.send_to_context_switch(pid, "2cpu")

		# #########all changes(ready-queue regarding preemptes) made; do general update
		for pid, record in self.mini_pcb.items():
			if record["state"] == 0: # at cpu
				self.at_cpu(pid)
						
			elif record["state"] == 1: # ready
				self.at_ready_queue(pid)

			elif record["state"] == 2: # i/o blocked
				self.at_io_block(pid)

			elif record["state"] in [3, 4, 5, 6]:
				self.at_context_switch(pid)

			else: # state = -1 - not arrived yet
				pass

		# ###########debug
		# for pid in self.process_queue:
		# 	if (self.mini_pcb[pid]["leave_t"] == -1) & \
		# 		((self.mini_pcb[pid]["arrival_t"] >= 0) &
		# 		(self.system_clock > self.mini_pcb[pid]["arrival_t"])):
		#
		# 		self.mini_pcb[pid]["total_t"] += 1
		#
		# 		assert (self.mini_pcb[pid]["total_t"] == (
		# 				self.system_clock -
		# 				self.mini_pcb[pid]["arrival_t"]))
		# 	else:
		# 		if self.mini_pcb[pid]["leave_t"] != -1:
		#
		# 			assert(self.mini_pcb[pid]["total_t"] == (
		# 				self.mini_pcb[pid]["leave_t"] -
		# 				self.mini_pcb[pid]["arrival_t"]
		# 			))


		return 

	def cpu_simulator(self, algo):
		'''

		'''

		self.rand = Rand48(self.seed)
		self.rand.srand(self.seed)
		self.algo = algo


		def local_init(): # clean the state
			self.system_clock = 0 # ms
			self.ready_queue = []
			self.running_pid = -1

		local_init()
		self.init_pcb()

		if self.algo != "SJF": # submitty
			print('\n', end='')  #
		for pid in self.process_queue:
			# Each pid generate its rands in a batch
			# (before hand)
			# arrival time:
			# cpu burst time: [...]
			# io burst time: [...]
			# (not extracting rand numbers on the run)

			# first rand
			self.mini_pcb[pid]["arrival_t"] = math.floor(self.exp_rand())
			############debug
			# if pid == 'A':
			# 	self.mini_pcb[pid]["arrival_t"] = 0
			# else:
			# 	self.mini_pcb[pid]["arrival_t"] = 1
			############

			# second rand
			self.mini_pcb[pid]["cpu_burst_no"] = math.floor(self.rand.drand() * 100) + 1 #[1, 100]
			###########debug
			# if pid == "A":
			# 	self.mini_pcb[pid]["cpu_burst_no"] = 2
			# else:
			# 	self.mini_pcb[pid]["cpu_burst_no"] = 2
			###############


			for i in range(self.mini_pcb[pid][
							   "cpu_burst_no"]-1):
				self.process_cpuburstTime[pid].append(math.ceil(self.exp_rand()))  # cpu burst

				################debug
				#if pid == 'A':
				# 	self.process_cpuburstTime[pid].append(2)  # cpu burst
				# else:
				# 	self.process_cpuburstTime[pid].append(1)
				################
				self.process_ioburstTime[pid].append(math.ceil(self.exp_rand()))  # io burst
				################debug
				#self.process_ioburstTime[pid].append(1)  # io burst
				#####################
			self.process_cpuburstTime[pid].append(math.ceil(self.exp_rand()))  # cpu burst

			self.msg(pid, "proc-in") # "Process A [NEW] (arrival time 9 ms) 16 CPU bursts"


		arrival_l = sorted(zip([self.mini_pcb[pid]["arrival_t"] for pid in self.process_queue],
							   self.process_queue), key=lambda t:t[0])
		i = 0
		job_coming = iter([pid for (arrival_t, pid) in arrival_l])
		self.msg(None, "simulator-start")
		while(1): #
			if i < len(self.process_queue):
				next_job_in_sec = arrival_l[i][0] - self.system_clock
				if next_job_in_sec == 0:
					job_arrive = job_coming.__next__()
					i += 1
					self.pcb_update({'event':1, 'pid':job_arrive}) # what happened at this time
				else:
					self.pcb_update({'event': 0, 'pid': 0})  # general update
			else: # if no next job, not checking for new job arrival
				if -1 in set([self.mini_pcb[pid]["leave_t"] for pid in self.process_queue]):
					# exist some unfinished processes
					self.pcb_update({'event': 0, 'pid': 0})  # general update
				else:
					self.system_clock -= 1
					self.msg(None, "simulator-end")
					self.msg(None, "simulator-summary")
					break

			self.system_clock += 1

class Rand48:
	def __init__(self, seed):
		self.n = seed

	def seed(self, seed):
		self.n = seed

	def srand(self, seed):
		self.n = (seed << 16) + 0x330e

	def next(self):
		self.n = (25214903917 * self.n + 11) & (2 ** 48 - 1)
		return self.n

	def drand(self):
		return self.next() / 2 ** 48

	def lrand(self):
		return self.next() >> 17

	def mrand(self):
		n = self.next() >> 16
		if n & (1 << 31):
			n -= 1 << 32
		return n


if __name__ == "__main__":
	#sys.stdout = open("std.out", 'w')

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


	debugging = 0

	# python test.py arg1 arg2 arg3
	# ['test.py', 'arg1', 'arg2', 'arg3']
	if debugging:
		#sys.argv = ["project_1.py", 2, 0.01, 200, 1, 4, 0.5, 120]
		#sys.argv = ["project_1.py", 2, 0.01, 200, 2, 4, 0.5, 120]
		#sys.argv = ["project_1.py", 2, 0.01, 200, 12, 4, 0.5, 120]
		sys.argv = ["project_1.py", 73, 0.001, 3000, 12, 4, 0.5, 950]  #1min10sec
		algo_list = ["SJF", "SRT", "FCFS", "RR"]
	else:
		algo_list = ["SJF", "SRT", "FCFS", "RR"]

	kwargs = {"seed":int(sys.argv[1]),
				"lambda":float(sys.argv[2]),
				"upper_bound_t":int(sys.argv[3]),
				"n":int(sys.argv[4]),
				"tcs":int(sys.argv[5]),
				"alpha":float(sys.argv[6]),
				"t_slice":int(sys.argv[7]),
			  	"submitty": not(debugging)
				}

	try:
		kwargs["rr_add"] = sys.argv[8]
	except:
		kwargs["rr_add"] = -1

	scheduler = cpu_scheduling(kwargs)

	for algo in algo_list:
		scheduler.cpu_simulator(algo)







