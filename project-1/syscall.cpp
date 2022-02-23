/* SPIM S20 MIPS simulator.
   Execute SPIM syscalls, both in simulator and bare mode.
   Execute MIPS syscalls in bare mode, when running on MIPS systems.
   Copyright (c) 1990-2010, James R. Larus.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of the James R. Larus nor the names of its contributors may be
   used to endorse or promote products derived from this software without specific
   prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#endif

#include "spim.h"
#include "string-stream.h"
#include "inst.h"
#include "reg.h"
#include "mem.h"
#include "sym-tbl.h"
#include "syscall.h"
#include <string.h>
#include "run.h"
#include "spim-utils.h"



#ifdef _WIN32
/* Windows has an handler that is invoked when an invalid argument is passed to a system
   call. https://msdn.microsoft.com/en-us/library/a9yf33zb(v=vs.110).aspx

   All good, except that the handler tries to invoke Watson and then kill spim with an exception.

   Override the handler to just report an error.
*/

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

void myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{
  if (function != NULL)
    {
      run_error ("Bad parameter to system call: %s\n", function);
    }
  else
    {
      run_error ("Bad parameter to system call\n");
    }
}

static _invalid_parameter_handler oldHandler;

void windowsParameterHandlingControl(int flag )
{
  static _invalid_parameter_handler oldHandler;
  static _invalid_parameter_handler newHandler = myInvalidParameterHandler;

  if (flag == 0)
    {
      oldHandler = _set_invalid_parameter_handler(newHandler);
      _CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.
    }
  else
    {
      newHandler = _set_invalid_parameter_handler(oldHandler);
      _CrtSetReportMode(_CRT_ASSERT, 1);  // Enable the message box for assertions.
    }
}
#endif



/************************************************************************************/
	int runAsm(int file){
  bool runnable = false;
  
  initialize_world(exception_file_name,false);
  if (file==1)
     read_assembly_file("ShowPrimes.asm");
  else if (file==2)
    read_assembly_file("Factorize.asm");
  else if(file==3)
    read_assembly_file("BubbleSort.asm");
  else{
    printf("INVALID FILE\n");
    R[REG_RES]=-1;
    return -1;
    }
    
  
    run_program (starting_address(), DEFAULT_RUN_STEPS, false, false, &runnable);
	return 0;
  }
/**************************************************************************************/



/* Decides which syscall to execute or simulate.  Returns zero upon
   exit syscall and non-zero to continue execution. */

int
do_syscall ()
{
#ifdef _WIN32
    windowsParameterHandlingControl(0);
#endif

  /* Syscalls for the source-language version of SPIM.  These are easier to
     use than the real syscall and are portable to non-MIPS operating
     systems. */

  switch (R[REG_V0])
    {
    case PRINT_INT_SYSCALL:
      write_output (console_out, "%d", R[REG_A0]);
      break;

    case PRINT_FLOAT_SYSCALL:
      {
	float val = FPR_S (REG_FA0);

	write_output (console_out, "%.8f", val);
	break;
      }

    case PRINT_DOUBLE_SYSCALL:
      write_output (console_out, "%.18g", FPR[REG_FA0 / 2]);
      break;

    case PRINT_STRING_SYSCALL:
      write_output (console_out, "%s", mem_reference (R[REG_A0]));
      break;

    case READ_INT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	R[REG_RES] = atol (str);
	break;
      }

    case READ_FLOAT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR_S (REG_FRES) = (float) atof (str);
	break;
      }

    case READ_DOUBLE_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR [REG_FRES] = atof (str);
	break;
      }

    case READ_STRING_SYSCALL:
      {
	read_input ( (char *) mem_reference (R[REG_A0]), R[REG_A1]);
	data_modified = true;
	break;
      }

    case SBRK_SYSCALL:
      {
	mem_addr x = data_top;
	expand_data (R[REG_A0]);
	R[REG_RES] = x;
	data_modified = true;
	break;
      }

    case PRINT_CHARACTER_SYSCALL:
      write_output (console_out, "%c", R[REG_A0]);
      break;

    case READ_CHARACTER_SYSCALL:
      {
	static char str [2];

	read_input (str, 2);
	if (*str == '\0') *str = '\n';      /* makes xspim = spim */
	R[REG_RES] = (long) str[0];
	break;
      }

    case EXIT_SYSCALL:
      spim_return_value = 0;
      return (0);

    case EXIT2_SYSCALL:
      spim_return_value = R[REG_A0];	/* value passed to spim's exit() call */
		initialize_world(exception_file_name, false);
      return (0);

    case OPEN_SYSCALL:
      {
#ifdef _WIN32
        R[REG_RES] = _open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#else
	R[REG_RES] = open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#endif
	break;
      }

    case READ_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	data_modified = true;
	break;
      }

    case WRITE_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	break;
      }

    case CLOSE_SYSCALL:
      {
#ifdef _WIN32
	R[REG_RES] = _close(R[REG_A0]);
#else
	R[REG_RES] = close(R[REG_A0]);
#endif
	break;
      }
	case CREATE_PROCESS_SYSCALL:
  {
      /********************************************************************/
  int x = R[REG_A0];//torun determined files with sys callas according to contex of a0

  
  
    ///taking backup of global variables in mem.h so it will be easy to switch old context
	instruction **text_bUP=text_seg;
	bool textAccess_bUP=text_modified;
	mem_addr textT_bUP=text_top;

	mem_word *data_bUP=data_seg;
	bool dataAccess_bUP=data_modified;
	short *dataH_bUP=data_seg_h;
	BYTE_TYPE *dataB_bUP= data_seg_b;
	mem_addr dataT_bUP= data_top;
	
	mem_addr gp_bUP= gp_midpoint;
	mem_word *stack_bUP= stack_seg;
	short *stackH_bUP= stack_seg_h;
	BYTE_TYPE *stackB_bUP= stack_seg_b;
	mem_addr stackBot_bUP= stack_bot;
	
	instruction **k_text_bUP= k_text_seg;
	mem_addr k_textT_bUP= k_text_top;
	
	mem_word *k_data_bUP= k_data_seg;
	short *k_dataH_bUP= k_data_seg_h;
	BYTE_TYPE *k_dataB_bUP= k_data_seg_b;
	mem_addr k_dataT_bUP= k_data_top;
	
	reg_word Rc[R_LENGTH];
	reg_word HI_bUP= HI;
	reg_word LO_bUP= LO;
	mem_addr PC_bUP= PC;
	mem_addr nPC_bUP= nPC;
	double *FPR_bUP= FPR;
	float *FGR_bUP= FGR;
	int *FWR_bUP= FWR;
	char *int_reg_names_bUP[32];
		memcpy(&int_reg_names_bUP,&int_reg_names,sizeof(int_reg_names_bUP));
	reg_word CCR_bUP[4][32];
		memcpy(&CCR_bUP,&CCR,sizeof(CCR));
	reg_word CPR_bUP[4][32];
		memcpy(&CPR_bUP,&CPR,sizeof(CPR));


        
    ///to clean variables and get ready for new process
	text_seg = NULL;
	data_seg = NULL;
	data_seg_h = NULL;
	data_seg_b = NULL;
	stack_seg = NULL;
	stack_seg_h = NULL;
	stack_seg_b = NULL;
	k_text_seg = NULL;
	k_data_seg = NULL;
	k_data_seg_h = NULL;
	k_data_seg_b = NULL;
	FPR = NULL;
	FGR = NULL;
	FWR = NULL;
	
    //run the assembly file i do that by checking a0 value to keep simple of c file
  runAsm(x);


  /////take backup and place them into related places so we come back to run old process which elements are stroed in backup  variables
	data_seg = data_bUP;
	data_modified = dataAccess_bUP;
	data_seg_h = dataH_bUP;
	data_seg_b = dataB_bUP;
	data_top = dataT_bUP;

	gp_midpoint = gp_bUP;
	stack_seg = stack_bUP;
	stack_seg_h = stackH_bUP;
	stack_seg_b = stackB_bUP;
	stack_bot = stackBot_bUP;

	text_seg = text_bUP; 
	text_modified = textAccess_bUP;
	text_top = textT_bUP;

	k_text_seg = k_text_bUP;
	k_text_top = k_textT_bUP;
	k_data_seg = k_data_bUP;
	k_data_seg_h = k_dataH_bUP;
	k_data_seg_b = k_dataB_bUP;
	k_data_top = k_dataT_bUP;
	memcpy ( &R, &Rc, sizeof(Rc) );
	HI = HI_bUP;
	LO = LO_bUP;
	PC = PC_bUP;
	nPC = nPC_bUP;
	FPR = FPR_bUP;
	FGR = FGR_bUP;
	FWR = FWR_bUP;
	memcpy ( &int_reg_names, &int_reg_names_bUP, sizeof(int_reg_names_bUP) );
	memcpy ( &CCR, &CCR_bUP, sizeof(CCR_bUP) );
	memcpy ( &CPR, &CPR_bUP, sizeof(CPR_bUP) );
	
	break;
    /*********************************************************************************************/
  }
    default:
      run_error ("Unknown system call: %d\n", R[REG_V0]);
      break;
    }

#ifdef _WIN32
    windowsParameterHandlingControl(1);
#endif
  return (1);
}


void
handle_exception ()
{
  if (!quiet && CP0_ExCode != ExcCode_Int)
    error ("Exception occurred at PC=0x%08x\n", CP0_EPC);

  exception_occurred = 0;
  PC = EXCEPTION_ADDR;

  switch (CP0_ExCode)
    {
    case ExcCode_Int:
      break;

    case ExcCode_AdEL:
      if (!quiet)
	error ("  Unaligned address in inst/data fetch: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_AdES:
      if (!quiet)
	error ("  Unaligned address in store: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_IBE:
      if (!quiet)
	error ("  Bad address in text read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_DBE:
      if (!quiet)
	error ("  Bad address in data/stack read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_Sys:
      if (!quiet)
	error ("  Error in syscall\n");
      break;

    case ExcCode_Bp:
      exception_occurred = 0;
      return;

    case ExcCode_RI:
      if (!quiet)
	error ("  Reserved instruction execution\n");
      break;

    case ExcCode_CpU:
      if (!quiet)
	error ("  Coprocessor unuable\n");
      break;

    case ExcCode_Ov:
      if (!quiet)
	error ("  Arithmetic overflow\n");
      break;

    case ExcCode_Tr:
      if (!quiet)
	error ("  Trap\n");
      break;

    case ExcCode_FPE:
      if (!quiet)
	error ("  Floating point\n");
      break;

    default:
      if (!quiet)
	error ("Unknown exception: %d\n", CP0_ExCode);
      break;
    }
}
