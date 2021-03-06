/*
  emain.c

  Copyright (C) 2012 Adapteva, Inc.
  Contributed by Xin Mao <maoxin99@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program, see the file COPYING.  If not, see
  <http://www.gnu.org/licenses/>.
*/

// This is the device side of the Hardware Barrier example project.
// The host may load this program to any eCore. When launched, the 
// core will do the counting. The hardware barrier is inserted in 
// each counting to syncronize the counting. A success/error message 
// is sent to the host according to the result.
//
// Aug-2013, XM.


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <e-lib.h>

#define E_WAND_INT (0x8)


void __attribute__((interrupt)) wand_isr();

unsigned state;
volatile unsigned *result;

/* Sync point between first core in group and host */
volatile uint32_t *pause;


int main(void) {
	//initialize
	int i;
	unsigned row, col, delay, num;
	unsigned *ivt;

	e_irq_global_mask(E_FALSE);
	e_irq_attach(E_WAND_INT, wand_isr);
	e_irq_mask(E_WAND_INT, E_FALSE);

	row     = e_group_config.core_row;
	col     = e_group_config.core_col;
	num     = row * e_group_config.group_cols + col;
	pause   = (volatile uint32_t *) (0x7000);
	result  = (volatile unsigned *) (0x8f000000 + 0x4*num);
	delay   = 0x2000 * num + 0x2000;

	if (num == 0)
		*pause = 0;

	*result = 0xdeadbeef;

	for(i=0; i<0x10000; i++)
	{
		*result = i;
		e_wait(E_CTIMER_0, delay);

		if (num == 0)
			while ((*pause));


		__asm__ __volatile__("wand");
		__asm__ __volatile__("idle");

		// clear wand bit
		state = e_reg_read(E_REG_STATUS);
		state = state & (~0x8);
		e_reg_write(E_REG_FSTATUS, state);
	}

	return EXIT_SUCCESS;
}


void __attribute__((interrupt)) wand_isr()
{
//	e_wait(E_CTIMER_1, 0x100);

	return;
}


