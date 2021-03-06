#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "prog.h"
//
// the next two inclusions are
// to be used in the software version
//  
#ifdef SW
#include <pipeHandler.h>
#include <Pipes.h>
#include <pthreadUtils.h>
#include "prog.h"
#else
#include "vhdlCStubs.h"
#endif
#include <math.h>

#define EPSILON 1.0e-6
//
//
float a_matrix[ORDER][ORDER], b_matrix[ORDER][ORDER], expected_c_matrix[ORDER][ORDER], c_matrix[ORDER][ORDER];

void Exit(int sig)
{
	fprintf(stderr, "## Break! ##\n");
	exit(0);
}

#ifdef SW
DEFINE_THREAD(mmultiply);
#endif

void write_matrices()
{
	uint32_t i,j;
#ifndef PERFONLY
        for(i = 0; i < ORDER; i++) 
	{
        	for(j = 0; j < ORDER; j++) 
		{
			write_float32("in_data_pipe",a_matrix[i][j]);
		}
	}

        for(i = 0; i < ORDER; i++) 
	{
        	for(j = 0; j < ORDER; j++) 
		{
			write_float32("in_data_pipe",b_matrix[i][j]);
		}
	}
#else
	write_float32("in_data_pipe",0.0);
	write_float32("in_data_pipe",0.0);
#endif
}


void read_result_matrix()
{
	uint32_t i,j;
#ifndef PERFONLY
        for(i = 0; i < ORDER; i++) 
	{
        	for(j = 0; j < ORDER; j++) 
		{
			c_matrix[i][j]= read_float32("result_pipe");
		}
	}
#else
	float v = read_float32("result_pipe");
#endif
}


int main(int argc, char* argv[])
{
	signal(SIGINT,  Exit);
  	signal(SIGTERM, Exit);
  	signal(SIGSEGV, Exit);

        int i,j,k;

        srand48(100);

        for(i = 0; i < ORDER; i++)
	{
        	for(j = 0; j < ORDER; j++)
		{
			a_matrix[i][j] = drand48();
			b_matrix[i][j] = drand48();
		}
	}

        for(i = 0; i < ORDER; i++)
	{
        	for(j = 0; j < ORDER; j++)
		{
			expected_c_matrix[i][j] = 0;
        		for(k = 0; k < ORDER; k++)
				expected_c_matrix[i][j] += a_matrix[i][k] * b_matrix[k][j];
		}
	}
	
#ifdef SW
	init_pipe_handler();
	PTHREAD_DECL(mmultiply);
	PTHREAD_CREATE(mmultiply);
#endif

	write_matrices();
	read_result_matrix();



#ifndef PERFONLY
	fprintf(stdout,"results: \n ");
	for(i = 0; i < ORDER; i++)
	{
		for(j = 0; j < ORDER; j++)
		{
			float tmp_err = fabs(expected_c_matrix[i][j] - c_matrix[i][j]);
			if(tmp_err < EPSILON)
				fprintf(stdout,"result[%d][%d] = %f\n", i, j, c_matrix[i][j]);
			else
				fprintf(stdout,"Error: result[%d][%d] = %f, expected = %f\n", 
						i, j, c_matrix[i][j], expected_c_matrix[i][j]);

		}
	}
#endif
	fprintf(stdout,"done\n");

#ifdef SW
	PTHREAD_CANCEL(mmultiply);
	close_pipe_handler();
#endif
}
