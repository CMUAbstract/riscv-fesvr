// See LICENSE for license details.

#ifndef __SIMCALL_H
#define __SIMCALL_H

#include "device.h"
#include "memif.h"

class simcall_t;
typedef reg_t (simcall_t::*simcall_func_t)(reg_t, reg_t, reg_t, reg_t, reg_t, reg_t, reg_t);

class htif_t;
class memif_t;

class simcall_t : public device_t {
	public:
		simcall_t(htif_t *);
		~simcall_t() {}
		const char *identity() { return "simcall"; }
	private:
		void handle_simcall(command_t cmd);
	private:
		reg_t sim_mark(reg_t, reg_t, reg_t, reg_t, reg_t, reg_t, reg_t);
		reg_t sim_unmark(reg_t, reg_t, reg_t, reg_t, reg_t, reg_t, reg_t);
		reg_t sim_trace(reg_t, reg_t, reg_t, reg_t, reg_t, reg_t, reg_t);
		reg_t sim_stop_trace(reg_t, reg_t, reg_t, reg_t, reg_t, reg_t, reg_t);
	private:
		htif_t *htif;
		memif_t *memif;
		std::vector<simcall_func_t> table;
};
#endif