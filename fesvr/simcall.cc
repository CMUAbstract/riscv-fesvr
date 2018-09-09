// See LICENSE for license details.

#include "simcall.h"
#include "htif.h"

simcall_t::simcall_t(htif_t *htif)
	: htif(htif), memif(&htif->memif()), table(128) {
	table[12] = &simcall_t::sim_mark_input;
	table[13] = &simcall_t::sim_mark_output;
	table[14] = &simcall_t::sim_clear_mark;
	register_command(0, std::bind(
		&simcall_t::handle_simcall, this, std::placeholders::_1), "simcall");
}

void simcall_t::handle_simcall(command_t cmd) {
	reg_t magicmem[8];
	memif->read(cmd.payload(), sizeof(magicmem), magicmem);
	
	reg_t n = magicmem[0];
	if (n > table.size() || !table[n])
		throw std::runtime_error("bad simcall #" + std::to_string(n));

	magicmem[0] = (this->*table[n])(magicmem[1], magicmem[2], magicmem[3], 
		magicmem[4], magicmem[5], magicmem[6], magicmem[7]);
	memif->write(cmd.payload(), sizeof(magicmem), magicmem);	
	cmd.respond(1);
}

reg_t simcall_t::sim_mark_input(
	reg_t addr, reg_t size, reg_t tag, reg_t a3, reg_t a4, reg_t a5, reg_t a6) {
	htif->mark_input(addr, size, tag);
	return 0;
}

reg_t simcall_t::sim_mark_output(
	reg_t addr, reg_t size, reg_t tag, reg_t a3, reg_t a4, reg_t a5, reg_t a6) {
	htif->mark_output(addr, size, tag);
	return 0;
}

reg_t simcall_t::sim_clear_mark(
	reg_t addr, reg_t size, reg_t tag, reg_t a3, reg_t a4, reg_t a5, reg_t a6) {
	htif->clear_mark(addr, size, tag);
	return 0;
}