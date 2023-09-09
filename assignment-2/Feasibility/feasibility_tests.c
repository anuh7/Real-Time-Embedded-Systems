/******************************************************************************
* @file: feasibility_tests.c
* @brief: To learn the methods of worst case analysis and compare exact and 
            estimated feasibility decision testing.
* @author: Modified by Anuhya, code provided in class-feasibility_tests.c by Prof. Sam Siewart
* @date:  28-June-2023
*******************************************************************************/

#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// U=
U32_T ex0_period[] = {2, 4, 16};
U32_T ex0_wcet[] = {1, 1, 4};

// U=
U32_T ex1_period[] = {3, 5, 15};
U32_T ex1_wcet[] = {1, 2, 5};

// U=
U32_T ex2_period[] = {2, 5, 7, 14};
U32_T ex2_wcet[] = {1, 1, 1, 2};

// U=
U32_T ex3_period[] = {5, 9, 16};
U32_T ex3_wcet[] = {2, 3, 4};

// U=
U32_T ex4_period[] = {2, 4, 7};
U32_T ex4_wcet[] =  {1, 1, 1};

// U=
U32_T ex5_period[] = {5, 9, 15};
U32_T ex5_wcet[] = {3, 3, 3};

int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int scheduling_point_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int rate_monotonic_least_upper_bound(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);


int main(void)
{ 
    int i;
	U32_T numServices;
    
    printf("******** Completion Test Feasibility Example\n");
   
    printf("Ex-5 U=%4.2f\% (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/4.0)*100.0 + (4.0/16.0)*100.0));
	numServices = 3;
    printf("\n");
    if(completion_time_feasibility(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("CT test FEASIBLE\n");
    else
        printf("CT test INFEASIBLE\n");

    if(scheduling_point_feasibility(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf(" SP test FEASIBLE\n");
    else
        printf("SP test INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

    printf("Ex-6 U=%4.2f\% (C1=1, C2=2, C3=5; T1=3, T2=5, T3=15; T=D): ", 
		   ((1.0/3.0)*100.0 + (2.0/5.0)*100.0 + (5.0/15.0)*100.0));
	numServices = 3;
        printf("\n");
    if(completion_time_feasibility(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("CT test FEASIBLE\n");
    else
        printf("CT test INFEASIBLE\n");

    if(scheduling_point_feasibility(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf(" SP test FEASIBLE\n");
    else
        printf("SP test INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

	
    printf("Ex-7 U=%4.2f\% (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=14; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/5.0)*100.0 + (1.0/7.0)*100.0 + (2.0/14.0)*100.0));
	numServices = 4;
        printf("\n");
    if(completion_time_feasibility(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
        printf("CT test FEASIBLE\n");
    else
        printf("CT test INFEASIBLE\n");
    
    if(scheduling_point_feasibility(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
        printf("SP test FEASIBLE\n");
    else
        printf("SP test INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");


    printf("Ex-8 U=%4.2f\% (C1=2, C2=3, C3=4; T1=5, T2=9, T3=16; T=D): ",
		   ((2.0/5.0)*100.0 + (3.0/9.0)*100.0 + (4.0/16.0)*100.0));
	numServices = 3;
        printf("\n");
    if(completion_time_feasibility(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        printf("CT test FEASIBLE\n");
    else
        printf("CT test INFEASIBLE\n");

    if(scheduling_point_feasibility(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        printf("SP test FEASIBLE\n");
    else
        printf("SP test INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

	
    printf("Ex-9 U=%4.2f\% (C1=1, C2=1, C3=1; T1=2, T2=4, T3=7; T=D): ",
		   ((1.0/2.0)*100.0 + (1.0/4.0)*100.0 + (1.0/7.0)*100.0));
	numServices = 3;
        printf("\n");
    if(completion_time_feasibility(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        printf("CT test FEASIBLE\n");
    else
        printf("CT test INFEASIBLE\n");
    
    if(scheduling_point_feasibility(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        printf("SP test FEASIBLE\n");
    else
        printf("SP test INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");

    printf("Ex-10 U=%4.2f\% (C1=3, C2=3, C3=3; T1=5, T2=9, T3=15; T=D): ",
		   ((3.0/5.0)*100.0 + (3.0/9.0)*100.0 + (3.0/15.0)*100.0));
	numServices = 3;
        printf("\n");
    if(completion_time_feasibility(numServices, ex5_period, ex5_wcet, ex5_period) == TRUE)
        printf("CT test FEASIBLE\n");
    else
        printf("CT test INFEASIBLE\n");
    
    if(scheduling_point_feasibility(numServices, ex5_period, ex5_wcet, ex5_period) == TRUE)
        printf("SP test FEASIBLE\n");
    else
        printf("SP test INFEASIBLE\n");

    if(rate_monotonic_least_upper_bound(numServices, ex5_period, ex5_wcet, ex5_period) == TRUE)
        printf("RM LUB FEASIBLE\n");
    else
        printf("RM LUB INFEASIBLE\n");
    printf("\n");
}


int rate_monotonic_least_upper_bound(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  double utility_sum=0.0, lub=0.0;
  int idx;

  printf("for %d, utility_sum = %lf\n", numServices, utility_sum);

  // Sum the C(i) over the T(i)
  for(idx=0; idx < numServices; idx++)
  {
    utility_sum += ((double)wcet[idx] / (double)period[idx]);
    printf("for %d, wcet=%lf, period=%lf, utility_sum = %lf\n", idx, (double)wcet[idx], (double)period[idx], utility_sum);
  }
  printf("utility_sum = %lf\n", utility_sum);

  // Compute LUB for number of services
  lub = (double)numServices * (pow(2.0, (1.0/((double)numServices))) - 1.0);
  printf("LUB = %lf\n", lub);

  // Compare the utilty to the bound and return feasibility
  if(utility_sum <= lub)
	  return TRUE;
  else
	  return FALSE;
}


int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  int i, j;
  U32_T an, anext;
  
  // assume feasible until we find otherwise
  int set_feasible=TRUE;
   
  //printf("numServices=%d\n", numServices);
 
  // For all services in the analysis 
  for (i=0; i < numServices; i++)
  {
       an=0; anext=0;
       
       for (j=0; j <= i; j++)
       {
           an+=wcet[j];
       }
       
	   //printf("i=%d, an=%d\n", i, an);

       while(1)
       {
             anext=wcet[i];
	     
             for (j=0; j < i; j++)
                 anext += ceil(((double)an)/((double)period[j]))*wcet[j];
		 
             if (anext == an)
                break;
             else
                an=anext;

			 //printf("an=%d, anext=%d\n", an, anext);
       }
       
	   //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);

       if (an > deadline[i])
       {
          set_feasible=FALSE;
       }
  }
  
  return set_feasible;
}


int scheduling_point_feasibility(U32_T numServices, U32_T period[], 
				 U32_T wcet[], U32_T deadline[])
{
   int rc = TRUE, i, j, k, l, status, temp;

   // For all services in the analysis
   for (i=0; i < numServices; i++) // iterate from highest to lowest priority
   {
      status=0;

      // Look for all available CPU minus what has been used by higher priority services
      for (k=0; k<=i; k++) 
      {
	  // find available CPU windows and take them
          for (l=1; l <= (floor((double)period[i]/(double)period[k])); l++)
          {
               temp=0;

               for (j=0; j<=i; j++) temp += wcet[j] * ceil((double)l*(double)period[k]/(double)period[j]);

	       // Can we get the CPU we need or not?
               if (temp <= (l*period[k]))
			   {
				   // insufficient CPU during our period, therefore infeasible
				   status=1;
				   break;
			   }
           }
           if (status) break;
      }

      if (!status) rc=FALSE;
   }
   return rc;
}
