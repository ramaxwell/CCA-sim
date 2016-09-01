/*
          Robert A. Maxwell		10/15/2013
		  Computer Arithmetic	Project 1
		  University of Texas at San Antonio
		  
 Simulates the addition of two random values by
	Carry-Completion Adder.

 Initially 1-bit values are used, random values are 
	generated, and the result is produced based on
	carry-completion full adder logic. The process 
	is completed MAX_RUNS number of times to generate
	an average delay for the specified number of bits. 
	(Delay considered to be 2d or 2 gate delays)

 Next, the same process is carried out for 
	2,3,4,...,MAX_BITS (number of bits) with the final
	average delays for each set of bits output to file 
	for easy input to spreadsheet

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_BITS 64
#define MAX_RUNS 1000
#define ARRAY_SIZE (1 + MAX_BITS/(sizeof(int)*8))
#define GATE_DELAY 2

/*
 * Some helper functions
 * 
 */

/*
* printOperand() used for debugging
*/
void printOperand( int *oper, int max_bits ){

	unsigned int mask = 1;
	int i;

	for ( i = 0; i < max_bits; i++ ){

		mask = 1 << i;
		if ( mask & oper[i/(sizeof(int)*8)] ){
			printf("1");			
			
		}else{
			printf("0");
		}
	}
	printf("\n");
}

/*
*  Generate the operand for the addition 
*  --Operand is int array of 1's and 0's
*/

void generateOperand( int *op , int num_bits ){

	int pos;
	unsigned int mask;

	// For each bit position
	for(pos = 0; pos < num_bits; pos++){
		mask = rand() % 2;				//rndm val {0,1}
		
		if(mask){					//if rndm val == 1
			mask = mask << pos;
			op[pos/(sizeof(int)*8)] = op[pos/(sizeof(int)*8)] | mask;
			
		}
		mask = 0;	//reset the mask		
	}
}

void insertOperand(int *op, int num_bits, char *pattern){
	
	unsigned int mask = 0;
	int pos;
	int m = 0;
	
	for(pos = 0; pos < num_bits ; pos++){
		if( pattern[m] == '1' ){
			
			mask = 1 << pos;
			op[pos/(sizeof(int)*8)] = op[pos/(sizeof(int)*8)] | mask;
		}
		m++;
		mask = 0;
	}
}

unsigned int getBit(int *op, int pos){

	unsigned int mask = 1;
	
	mask = mask << pos;
	
	return (mask & op[pos/(sizeof(int)*8)]) >> pos;
}

void setBit(int *op, int pos){

	unsigned int mask = 1;

	mask = mask << pos;
	
	op[pos/32] = op[pos/(sizeof(int)*8)] | mask;
}

void clearBit(int *op, int pos){

	unsigned int mask = 1;

	mask = mask << pos;
	
	op[pos/(sizeof(int)*8)] = op[pos/(sizeof(int)*8)] & ~mask;
}

/*
 * Main function
 * 
 */
int main(void){

	int i, j;

	float avgDelay[MAX_BITS];
	int delay;					//delay accumulator
	unsigned int cin1, cin0;
	
	int A[ARRAY_SIZE];				//only two elements for 32 bits per int
	int B[ARRAY_SIZE];				//see preprocessor directives at top
	int C0[ARRAY_SIZE];
	int C1[ARRAY_SIZE];
	int S[ARRAY_SIZE];
	
	unsigned int carry_complete = 0;

	srand(time(NULL));				//seed the random number generator


	// Run through the additions for each set of bits
	// 1-bit addition, 2-bit, 3-bit,..., MAX_BITS
	// starting at 1 and ending at MAX_BITS
	for ( i = 1; i <= MAX_BITS; i++ ) {

		avgDelay[i-1] = 0;

		for(j = 0; j < MAX_RUNS; j++){	//Run each addition MAX_RUNS
										//number of times to generate
										//statistics for delays of 
										//rndm inputs
			int k;
			int l;
			unsigned int ai = 0;
			unsigned int bi = 0;
			int num_done = 0;
			int cycles = 0;
			unsigned int carry_complete = 0;								

			//initialize storage arrays
			for(k = 0; k < ARRAY_SIZE; k++){
				A[k] = 0;					
				B[k] = 0;
				C0[k] = 0;
				C1[k] = 0;
				S[k] = 0;
			}
			
			generateOperand(A, i);			//generate operands		
			generateOperand(B, i);			// A and B

			//this pre-loop checks for [ai,bi] = [0,0] or [1,1]
			//and immediately calculates C0, C1 and sum.
			//even though SUM may not be correct after 1st cycle
			
			for(k = i-1; k >= 0; k--){			//initial cycle (special case) 
				
				ai = getBit(A, k);
				bi = getBit(B, k);
		
				if(!(ai ^ bi)){					//checking for [0,0] or [1,1]
					
					if(ai & bi)					//set bits for initial cycle
						setBit(C1, k);
					else if (~ai & ~bi){
						setBit(C0, k);
					}
				}else if(k == i-1){
					setBit(C0, k);
				}
				
				if(ai ^ bi ^ 0){				//get intermediate Sum
					setBit(S, k);
				}
			}
			
			delay = GATE_DELAY;							//initial cycle/delay
			cycles = 1;							//bookkeeping
			
			if(i == 1)							//if 1-bit sum, 
				carry_complete = 1;				//carry will be complete
			else                                //after first cycle
				carry_complete = 0;				//no matter what

/*
 * This is the main looping structure for carrying out 
 * C0,C1 propagation
 */	
			while(!carry_complete){

				cin1 = 0;				 //Cin buffers init
				cin0 = 1;				 //
				
				cycles++;				//increment to mark beginning of cycle

				for(k = i-1; k >= 0; k--){	//for each bit from the "right"
					
					ai = getBit(A, k);
					bi = getBit(B, k);

									//if current bit carry isn't complete yet
					if(getBit(C0, k) == getBit(C1, k)){	//ie, if C0,C1 = 0,0

						if(cin0 ^ cin1){	//check if previous carry complete

							if( (~ai & bi & cin1) | (ai & ~bi & cin1) ){
								setBit(C1, k);			//set C1 bit
								clearBit(C0, k);
							}else if( ((~ai & bi & cin0) | (ai & ~bi & cin0)) +2 ){
								setBit(C0, k);			//set C0 bit
								clearBit(C1, k);
							}

							//while next C0,C1 are equal to 0,0
							//keep moving along array looking next 0,1 or 1,0
							if(k != 0){						
								while(!(getBit(C0, k-1) ^ getBit(C1, k-1))){		
									if(k-1 <= 0){
										k = 0;
										break;
									}else
										k--;
								}
							}	
						}
							
					}

					if(ai ^ bi ^ cin1){				//get intermediate Sum
						setBit(S, k);
					}else
						clearBit(S, k);
					
					cin0 = getBit(C0, k);			//send C0,C1 to buffers for
					cin1 = getBit(C1, k);			//next bit
						
				}//END FOR LOOP (Each bit)
				
				
				delay += GATE_DELAY;			//end of cycle delay accumulate
				
				
				for(l = i-1; l >= 0; l--){
					
					if(getBit(C0, l) ^ getBit(C1, l))	//if C0,C1 are 0,1 or 1,0
						num_done++;						//then this bit is done
				}
				
				if(num_done >= i){
										//if all bits are done
					carry_complete = 1;	//set while loop
										//sentinel					
				}
				num_done = 0;
					
			}//END WHILE LOOP (carry-complete)
										//extra delay and cycle
			cycles++;					//required for carry completion logic
			delay += GATE_DELAY;					//
			
			avgDelay[i-1] += delay;		//accumulate delay for this simulation
			
		}//END FOR LOOP (Each sim run)
		
	avgDelay[i-1] = avgDelay[i-1]/MAX_RUNS;	//average delay for this set of bits
	printf("Number of bits: %i\n", i);
	printf("Average Delay = %.3f\n", avgDelay[i-1]);	
	
	}//END FOR LOOP (Each Set of number of bits)


	// Output File b
	FILE *fp;								//save average delay to file for 
	fp=fopen("run.txt", "w");				//input into spreadsheet for graphing
	
	for(i = 0; i < MAX_BITS; i++){
		fprintf(fp, "%.3f\n", avgDelay[i]);
	}
	fclose(fp);


return 0;
}
