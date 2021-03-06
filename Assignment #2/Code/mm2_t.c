/* External definitions for double-server queueing system. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */

#define Q_LIMIT 10000  /* Limit on queue length. */
#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0  /* and idle. */

int   next_event_type, num_custs_delayed1, num_custs_delayed2, time_limit, num_events,
      num_in_q1, num_in_q2, server1_status, server2_status, max_in_transit, num_in_transit, total_in_transit;
      
float area_num_in_q1, area_num_in_q2, area_server_status1, area_server_status2, mean_interarrival, service_time1, service_time2,
      sim_time, queue1[Q_LIMIT + 1], queue2[Q_LIMIT + 1], time_last_event, time_next_event[6],
      total_of_delays1, total_of_delays2;
      
FILE  *infile, *outfile;

void  initialize(void);
void  timing(void);
void  arrive1(void);
void  arrive2(void);
void  depart1(void);
void  depart2(void);
void  finish(void);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);
float uniform(float a, float b);

int main()  /* Main function. */
{
    /* Open input and output files. */

    infile  = fopen("mm2_t.in",  "r");
    outfile = fopen("mm2_t.out", "w");

    /* Specify the number of events for the timing function. */

    num_events = 5;

    /* Read input parameters. */

    fscanf(infile, "%f %f %f %d", &mean_interarrival, &service_time1, &service_time2, &time_limit);
           

    /* Write report heading and input parameters. */

    fprintf(outfile, "Double-server queueing system with transit time\n\n");
    fprintf(outfile, "Mean interarrival time%11.3f minutes\n\n",
            mean_interarrival);
    fprintf(outfile, "Mean service time for server 1%16.3f minutes\n\n", service_time1);
    fprintf(outfile, "Mean service time for server 2%16.3f minutes\n\n", service_time2);
    fprintf(outfile, "Time limit%14d\n\n", time_limit);

    /* Initialize the simulation. */

for(int i = 0; i < 10; i++) {    
	initialize();

		int running = 1;
		while(running) {
			/* Determine the next event. */

			timing();

			/* Update time-average statistical accumulators. */

			update_time_avg_stats();

			/* Invoke the appropriate event function. */

			switch (next_event_type) {
				case 1:
					arrive1();
					break;
				case 2:
					depart1();
					break;
				case 3:
					arrive2();
					break;
				case 4:
					depart2();
					break;
				case 5:
					finish();
					running = 0;
					break;
			}
		}

    
}

    fclose(infile);
    fclose(outfile);

    return 0;
}

void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */

    sim_time = 0.0;

    /* Initialize the state variables. */

    server1_status  = IDLE;
    server2_status  = IDLE;
    num_in_q1       = 0;
    num_in_q2       = 0;
    
    time_last_event = 0.0;

    /* Initialize the statistical counters. */

    num_custs_delayed1  = 0;
    num_custs_delayed2  = 0;
    total_of_delays1    = 0.0;
    total_of_delays2    = 0.0;
    area_num_in_q1      = 0.0;
    area_num_in_q2      = 0.0;
    area_server_status1 = 0.0;
    area_server_status2 = 0.0;
    num_in_transit = 0;
    max_in_transit = 0;
    total_in_transit = 0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration, as is the
       queue change operation. */

    time_next_event[1] = sim_time + expon(mean_interarrival);
    time_next_event[2] = 1.0e+30;
    time_next_event[3] = 1.0e+30;
    time_next_event[4] = 1.0e+30;
    time_next_event[5] = 1000;
    
}

void timing(void)  /* Timing function. */
{
    int   i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0;

    /* Determine the event type of the next event to occur. */

    for (i = 1; i <= num_events; ++i)
        if (time_next_event[i] < min_time_next_event) {
            min_time_next_event = time_next_event[i];
            next_event_type     = i;
        }

    /* Check to see whether the event list is empty. */

    if (next_event_type == 0) {

        /* The event list is empty, so stop the simulation. */

        fprintf(outfile, "\nEvent list empty at time %f", sim_time);
        exit(1);
    }

    /* The event list is not empty, so advance the simulation clock. */

    sim_time = min_time_next_event;
}

void arrive1(void)  /* Arrival event function. */
{
    float delay;

    /* Schedule next arrival. */

    time_next_event[1] = sim_time + expon(mean_interarrival);

    /* Check to see whether server 1 is busy. */

    if (server1_status == BUSY) {

        /* Server 1 is busy, so increment number of customers in first queue. */

        ++num_in_q1;

        /* Check to see whether an overflow condition exists. */

        if (num_in_q1 > Q_LIMIT) {

            /* The queue has overflowed, so stop the simulation. */
			
            fprintf(outfile, "\nOverflow of the array time_arrival at");
            fprintf(outfile, " time %f", sim_time);
            exit(2);
        }

        /* There is still room in the queue, so store the time of arrival of the
           arriving customer at the (new) end of time_arrival. */

        queue1[num_in_q1] = sim_time;
    }

    else {

        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay            = 0.0;
        total_of_delays1 += delay;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed1;
        server1_status = BUSY;

        /* Schedule a a queue change event. */

        time_next_event[2] = sim_time + expon(service_time1);
    }
    
    //printf("ARRIVE1: %d in queue 1 and %d in queue 2, SERVER 1 STATUS: %d and SERVER 2 STATUS: %d, %d in transit\n", num_in_q1, num_in_q2, server1_status, server2_status, num_in_transit);
}

void depart1(void)  /* Queue change event function. */
{
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q1 == 0) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server1_status      = IDLE;
        time_next_event[2] = 1.0e+30;
    }

    else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q1;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay = sim_time - queue1[1];
        total_of_delays1 += delay;

        /* Increment the number of customers delayed, and schedule next change and arrival into second queue. */

        ++num_custs_delayed1;
        time_next_event[2] = sim_time + expon(service_time1);
        time_next_event[3] = sim_time + uniform(0.0,2.0);

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q1; ++i)
            queue1[i] = queue1[i + 1];
    }
    
    /* Increment the current number of customers in transit. */
    
    num_in_transit += 1;
    //printf("DEPART1: %d in queue 1 and %d in queue 2, SERVER 1 STATUS: %d and SERVER 2 STATUS: %d, %d in transit\n", num_in_q1, num_in_q2, server1_status, server2_status, num_in_transit);
    
}
    
void arrive2(void) {

	float delay;
	
	
	/* Schedule next arrival from transit. */
	time_next_event[3] = sim_time + uniform(0.0,2.0);

    /* Check to see whether server 2 is busy. */

    if (server2_status == BUSY) {

        /* Server 2 is busy, so increment number of customers in second queue. */

        ++num_in_q2;

        /* Check to see whether an overflow condition exists. */

        if (num_in_q2 > Q_LIMIT) {

            /* The queue has overflowed, so stop the simulation. */

            fprintf(outfile, "\nOverflow of the array time_arrival at");
            fprintf(outfile, " time %f", sim_time);
            exit(2);
        }

        /* There is still room in the queue, so store the time of arrival of the
           arriving customer at the (new) end of time_arrival. */

        queue2[num_in_q2] = sim_time;
    }

    else {

        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay            = 0.0;
        total_of_delays2 += delay;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed2;
        server2_status = BUSY;

        /* Schedule a queue departure event. */

        time_next_event[4] = sim_time + expon(service_time2);
    }
    
    num_in_transit -= 1;
     
    
    //printf("ARRIVE2: %d in queue 1 and %d in queue 2, SERVER 1 STATUS: %d and SERVER 2 STATUS: %d, %d in transit\n", num_in_q1, num_in_q2, server1_status, server2_status, num_in_transit);

    
    
}


void depart2(void)  /* Departure event function. */
{
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q2 == 0) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server2_status      = IDLE;
        time_next_event[4] = 1.0e+30;
    }

    else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q2;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - queue2[1];
        total_of_delays2 += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed2;
        time_next_event[4] = sim_time + expon(service_time2);

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q2; ++i)
            queue2[i] = queue2[i + 1];
    }
    
    //printf("DEPART2: %d in queue 1 and %d in queue 2, SERVER 1 STATUS: %d and SERVER 2 STATUS: %d, %d in transit\n", num_in_q1, num_in_q2, server1_status, server2_status, num_in_transit);

}


void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */

    fprintf(outfile, "\n\nAverage delay in queue 1%11.3f minutes\n\n",
            total_of_delays1 / num_custs_delayed1);
    fprintf(outfile, "Average delay in queue 2%11.3f minutes\n\n",
            total_of_delays2 / num_custs_delayed2);
    fprintf(outfile, "Average number in queue 1%10.3f\n\n",
            area_num_in_q1 / sim_time);
    fprintf(outfile, "Average number in queue 2%10.3f\n\n",
            area_num_in_q2 / sim_time);
    fprintf(outfile, "Server 1 utilization%15.3f\n\n",
            area_server_status1 / sim_time);
    fprintf(outfile, "Server 2 utilization%15.3f\n\n",
            area_server_status2 / sim_time);    
    fprintf(outfile, "Maximum number in transit%14d\n\n",
            max_in_transit);     
    fprintf(outfile, "Average number in transit%15.3f\n\n",
            total_in_transit / sim_time);
            
    fprintf(outfile, "Time simulation ended%12.3f minutes\n\n\n", sim_time);
}


void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;

    /* Update area under number-in-queue function. */

    area_num_in_q1      += num_in_q1 * time_since_last_event;
    area_num_in_q2      += num_in_q2 * time_since_last_event;

    /* Update area under server-busy indicator function. */

    area_server_status1 += server1_status * time_since_last_event;
    area_server_status2 += server2_status * time_since_last_event;
    
    
    /* Update transit stats. */
    
    if(num_in_transit > max_in_transit) {
    	max_in_transit = num_in_transit;
    }
    
    total_in_transit += num_in_transit * time_since_last_event;

}

void finish(void) 
{
	/* Invoke the report generator and end the simulation. */
    report();
}


float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */

    return -mean * log(lcgrand(1));
}

float uniform(float a, float b)  /* Uniform variate generation function. */
{
    /* Return a U(a,b) random variate. */

    return a + lcgrand(1) * (b - a);
}






