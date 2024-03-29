/*
 *  idlestat.h
 *
 *  Copyright (C) 2014, Linaro Limited.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Contributors:
 *     Daniel Lezcano <daniel.lezcano@linaro.org>
 *     Zoran Markovic <zoran.markovic@linaro.org>
 *     Tuukka Tikkanen <tuukka.tikkanen@linaro.org>
 *
 */
#ifndef __IDLESTAT_H
#define __IDLESTAT_H

#define BUFSIZE 256
#define NAMELEN 16
#define MAXCSTATE 16
#define MAXPSTATE 16
#define MAX(A, B) (A > B ? A : B)
#define MIN(A, B) (A < B ? A : B)
#define AVG(A, B, I) ((A) + ((B - A) / (I)))

#define IRQ_WAKEUP_UNIT_NAME "cpu"

#define CPUIDLE_STATE_TARGETRESIDENCY_PATH_FORMAT \
	"/sys/devices/system/cpu/cpu%d/cpuidle/state%d/residency"
#define CPUFREQ_AVFREQ_PATH_FORMAT \
	"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies"
#define CPUIDLE_STATENAME_PATH_FORMAT \
	"/sys/devices/system/cpu/cpu%d/cpuidle/state%d/name"
#define CPUFREQ_CURFREQ_PATH_FORMAT \
	"/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_cur_freq"

double total_dur_cluster_pstates;
double *state_dur_cluster_freq_array;

double cluster_opp;
double cluster_mhz;
double total_mhz;

struct cpuidle_data {
	double begin;
	double end;
	double duration;
};

struct cpuidle_cstate {
	char *name;
	struct cpuidle_data *data;
	int nrdata;
	int early_wakings;
	int late_wakings;
	double avg_time;
	double max_time;
	double min_time;
	double duration;
	int target_residency; /* -1 if not available */
};

struct wakeup_irq {
	int id;
	char name[NAMELEN+1];
	int count;
	int early_triggers;
	int late_triggers;
};

struct wakeup_info {
	struct wakeup_irq *irqinfo;
	int nrdata;
};

struct cpuidle_cstates {
	struct cpuidle_cstate cstate[MAXCSTATE];
	struct wakeup_info wakeinfo;
	int current_cstate;
	int cstate_max;
	struct wakeup_irq *wakeirq;
	enum {as_expected, too_long, too_short} actual_residency;
};

extern void release_cstate_info(struct cpuidle_cstates *cstates, int nrcpus);

struct cpufreq_pstate {
	int id;
	unsigned int freq;
	int count;
	double min_time;
	double max_time;
	double avg_time;
	double duration;
};

struct cpufreq_pstates {
	struct cpufreq_pstate *pstate;
	int current;
	int idle;
	double time_enter;
	double time_exit;
	int max;
};

struct cpu_topology;

struct cpuidle_datas {
	struct cpuidle_cstates *cstates;
	struct cpufreq_pstates *pstates;
	struct cpu_topology *topo;
	struct cpuidle_datas *baseline;
	int nrcpus;
};

enum modes {
	TRACE = 0,
	IMPORT
};

struct trace_buffer_settings {
	unsigned int percpu_buffer_size;
	unsigned int poll_interval;
};

struct program_options {
	int mode;
	int display;
	int duration;
	struct trace_buffer_settings tbs;
	char *filename;
	char *baseline_filename;
	char *outfilename;
	int verbose;
	char *energy_model_filename;
	char *report_type_name;
};

#define IDLE_DISPLAY      0x1
#define FREQUENCY_DISPLAY 0x2
#define WAKEUP_DISPLAY    0x4
#define CPU_STATE_PER 0x08

struct cpuidle_datas *idlestat_load(const char *);

struct pstate_energy_info {
	unsigned int speed;
	unsigned int cluster_power;
	unsigned int core_power;
	double max_core_duration;
};

struct cstate_energy_info {
	char cstate_name[NAMELEN];
	unsigned int cluster_idle_power;
	unsigned int core_idle_power;
	double cluster_duration;
};

struct wakeup_energy_info {
	unsigned int cluster_wakeup_energy;
	unsigned int core_wakeup_energy;
};

enum energy_file_parse_state {
	uninitialized = 0,
	parsed_cluster_info,
	parsing_cap_states,
	parsing_c_states
};

struct cluster_energy_info {
	unsigned int number_cap_states;
	unsigned int number_c_states;
	struct pstate_energy_info *p_energy;
	struct cstate_energy_info *c_energy;
	struct wakeup_energy_info wakeup_energy;
	enum energy_file_parse_state state;
};

struct init_pstates {
	int nrcpus;
	unsigned int *freqs;
};

extern int store_data(double time, int state, int cpu, struct cpuidle_datas *datas);
extern struct cpuidle_cstates *build_cstate_info(int nrcpus);
extern struct cpufreq_pstates *build_pstate_info(int nrcpus);
extern int cpu_change_pstate(struct cpuidle_datas *datas, int cpu, unsigned int freq, double time);
extern int get_wakeup_irq(struct cpuidle_datas *datas, char *buffer);

#endif
