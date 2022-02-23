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

#include <iostream>
using namespace std;

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

struct ThreadTABLE
{
  pid_t thread_id;
  char thread_name[50];
  int program_counter;
  char thread_state= 'R';
  mem_word *thread_stack_seg;
  short *thread_stack_seg_h;
  BYTE_TYPE *thread_stack_seg_b;
  mem_addr thread_stack_bot;
  mem_addr stack_addr;

};
struct ThreadTABLE* threads[3]= {NULL};

int glob_counter=0;
int stack_s=1024;





/*You implement your handler here*/
void SPIM_timerHandler()
{ 
  //struct ThreadTABLE *t_thread= new ThreadTABLE();
 
   // Implement your handler..
   try
   {
     printf("Before Interrupt\n %d, %s, %d, %d, %c \n",threads[glob_counter]->thread_id=glob_counter,threads[glob_counter]->thread_name ,threads[glob_counter]->program_counter, threads[glob_counter]->stack_addr,threads[glob_counter]->thread_state);
      threads[glob_counter]->program_counter = PC;
     /* threads[glob_counter]->thread_stack_bot=stack_bot;
      threads[glob_counter]->thread_stack_seg=stack_seg;
      threads[glob_counter]->thread_stack_seg_h=stack_seg_h;
      threads[glob_counter]->thread_stack_seg_b=stack_seg_b;
     */ threads[glob_counter]->stack_addr=R[29];
     glob_counter+=1;
     if (glob_counter ==3)
     {
       glob_counter=0;
     }
     
     int i;
	  for ( i = glob_counter; i < 3; i++)
    {
      if(threads[i]->thread_state != 'D'){
        //t_thread= threads[i];
        break;
      }
      if(i==2){
        i=0;
      }
    }
    //printf("HERE\n");
    glob_counter = i;
    PC=threads[i]->program_counter;
    /*stack_bot= threads[i]->thread_stack_bot;
    stack_seg= threads[i]->thread_stack_seg;
    stack_seg_h=threads[i]->thread_stack_seg_h;
    stack_seg_b=threads[i]->thread_stack_seg_b;
    */R[29]=threads[i]->stack_addr;
    printf("After Interrupt\n %d, %s, %d, %d, %c \n",threads[glob_counter]->thread_id=glob_counter,threads[glob_counter]->thread_name ,threads[glob_counter]->program_counter, threads[glob_counter]->stack_addr,threads[glob_counter]->thread_state);
    return;
    
   }
   catch ( exception &e )
   {
      cerr <<  endl << "Caught: " << e.what( ) << endl;

   };
   
}
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
    case THREAD_CREATE:
    {
      char temp[3][20]={{"main"},{"t1"},{"t2"}};
      int i;
      for ( i = 0; i < 3; i++)
      {
        if(threads[i]==NULL){
          threads[i]= new ThreadTABLE();
          break;
        }
      }
      
      threads[i]->thread_id=i;
      strcpy(threads[i]->thread_name,temp[i]);
      threads[i]->program_counter= R[REG_A0];threads[i]->thread_stack_bot=stack_bot;
      /*threads[i]->thread_stack_seg=stack_seg;
      threads[i]->thread_stack_seg_h=stack_seg_h;
      threads[i]->thread_stack_seg_b=stack_seg_b;
      threads[i]->thread_state='R';
      */threads[i]->stack_addr= R[REG_A1];
      printf("created %d, %s, %d, %d, %c \n",threads[i]->thread_id=i,threads[i]->thread_name ,threads[i]->program_counter, threads[i]->stack_addr,threads[i]->thread_state);



      break;
    }
    case THREAD_JOIN:
    {
      int flag =0;
      int i=0;
      for(int j=0; j<3; j++)
        for ( i = 0; i < 3; i++)
        {
          if (threads[i]->thread_state !='D')
          {
            flag=1;
            break;
          }
          
        }
        if(flag == 1){
          while(PC != threads[i]->program_counter){
            PC=threads[i]->program_counter;
            stack_bot= threads[i]->thread_stack_bot;
            stack_seg= threads[i]->thread_stack_seg;
            stack_seg_h=threads[i]->thread_stack_seg_h;
            stack_seg_b=threads[i]->thread_stack_seg_b;
          }
          threads[i]->thread_state ='D';
          flag =0;
        }
      



      break;
    }
    case THREAD_EXIT:
    {
      int t_id_exit=R[REG_A0];
      int i;
      for ( i = 0; i < 3; i++)
      {
        if (t_id_exit == threads[i]->thread_id)
        {
          break;
        }
      }

      if(threads[i]->thread_state != 'D'){
        threads[i]->thread_state = 'D';
      }
      
      

      break;
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
