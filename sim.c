/*
 * Genetic Algorithm Example
 * Written by Marshall Beddoe <mbeddoe@gmail.com>
 * April 2003
 *
 * The purpose of this algorithm is to find the value of x which maximizes
 * the function f(x) = sin(pi * x / 256) over the domain 0 <= x <= 255, where
 * values of x are restricted to integers.
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define M_RATE	.001		/* Rate of mutation */
#define C_RATE	.8		/* Rate of crossover */
#define ROUNDS	512		/* Number of reproduction rounds */
#define	N_POP	32		/* Number of members in population */

/* Function prototypes */
double do_func(int);
void init_pop(u_int *);
void calc_function(int *, double *);
void find_slice(double *, double *);
void find_fitness(double *, double *);
void reproduce(double *, u_int *);
void init_random(double *);

/* Global variables */
int m_total, m_count;
int c_total, c_count;

int
main(int argc, char **argv)
{
	int i, j, verbose = 0;
	u_int population[N_POP];
	double results[N_POP], fitness[N_POP], slice[N_POP];
	double m_percent, c_percent, init_average, evol_average, total = 0;

	m_total = m_count = c_total = c_count = 0;

	if (argv[1] && argv[1][0] == 'v')
		verbose = 1;
	else
		printf("Use 'v' switch for verbose\n");

	/* Initialize population */
	init_pop((int *) &population);

	/* Calculate results & find average */
	calc_function((u_int *) &population, (double *) &results);

	for (i = 0; i < N_POP; i++) {
		total += results[i];
	}

	init_average = total / N_POP;

	if (verbose == 1) {
		puts("Initial Population:");

		for (i = 0; i < N_POP; i++)
			printf("%d\t\t\t\t%f\n", population[i], results[i]);
	}
	for (j = 0; j < ROUNDS; j++) {
		/* Calculate fitness of each member */
		find_fitness((double *) &results, (double *) &fitness);

		/* Create cumulative total of averages (roulette wheel) */
		find_slice((double *) &fitness, (double *) &slice);

		/* Select the fittest and reproduce */
		reproduce((double *) &slice, (u_int *) & population);

		/* Calculate results of evolved population */
		calc_function((u_int *) & population, (double *) &results);
	}

	for (i = 0, total = 0; i < N_POP; i++) {
		total += results[i];
	}

	/* Calculate average fitness for evolved population */
	evol_average = total / N_POP;

	if (verbose == 1) {
		puts("\nEvolved Population:");

		for (i = 0; i < N_POP; i++)
			printf("%d\t\t\t\t%f\n", population[i], results[i]);
	}
	m_percent = 1.0 * (100 * m_count) / m_total;
	c_percent = 1.0 * (100 * c_count) / c_total;

	printf("\nStatistics\n"
	       "\tPopulation Size\t= %d\n"
	       "\tReproduction\t= %d\n"
	       "\tInitial Average\t= %f\n"
	       "\tEvolved Average\t= %f\n"
	       "\tMutations\t= %d (%.02f%%)\n"
	       "\tCrossovers\t= %d (%.02f%%)\n", N_POP, ROUNDS, init_average,
	       evol_average, m_count, m_percent, c_count, c_percent);
}

/*
 * Function: reproduce()
 * Purpose: Find the most fit in population and procreate
 */
void
reproduce(double *slice, u_int * pop)
{
	int i, j;
	double random[N_POP], prob;
	u_int new_pop[N_POP], mask, tmp1, tmp2;
	u_int mutation[8] = {128, 64, 32, 16, 8, 4, 2, 1};

	init_random((double *) random);

	/*
	 * Find the fittest individuals of the population by generating
         * N_POP random numbers between 0 and 1.  Spin the roulette wheel.
         * Essentially, the greater the fitness, the greater the chance the
	 * randomly generated number will "select" that member of the population
         */

	for (i = 0; i < N_POP; i++) {
		for (j = 0; j < N_POP; j++) {
			if (j == 0) {
				if (random[i] > 0 && random[i] < slice[j])
					new_pop[i] = pop[j];
				continue;
			}
			if (random[i] > slice[j - 1] && random[i] < slice[j])
				new_pop[i] = pop[j];
		}
	}


	/*
	 * Perform allele-like crossover by generating a random 8 bit mask
	 * and using it to swap bits from one parent to the other. Thanks
	 * eugene for help!
	 */

	for (i = 0; i < N_POP; i += 2) {
		j = rand() % 1000;

		prob = 1.0 * j / 1000;

		c_total++;

		if (prob >= C_RATE)
			continue;

		c_count++;

		mask = arc4random() % 100;

		tmp1 = (new_pop[i + 1] & mask) | (new_pop[i] & ~mask);
		tmp2 = (new_pop[i] & mask) | (new_pop[i + 1] & ~mask);

		new_pop[i] = tmp1;
		new_pop[i + 1] = tmp2;
	}

	/*
         * Randomly mutate members of the population based on a pre-defined
         * mutation rate.  This allows more variation to occur when the
         * population is generally uniform.
         */

	for (i = 0; i < N_POP; i++) {
		j = arc4random() % 1000;

		prob = 1.0 * j / 1000;

		m_total++;

		if (prob <= M_RATE) {
			m_count++;
			j = arc4random() % 8;
			new_pop[i] = new_pop[i] ^ mutation[j];
		}
	}

	memcpy(pop, new_pop, sizeof(u_int) * N_POP);
}

/*
 * Function: init_random()
 * Purpose: Initialize random numbers for use in roulette wheel
 */
void
init_random(double *random)
{
	int i;

	for (i = 0; i < N_POP; i++) {
		srand48((time(NULL) ^ getpid()) ^ i);
		random[i] = drand48();
	}
}

/*
 * Function: find_slice()
 * Purpose: Find slice for member in the roulette wheel
 */
void
find_slice(double *fitness, double *slice)
{
	int i;

	slice[0] = 0 + fitness[0];

	for (i = 1; i < N_POP; i++)
		slice[i] = slice[i - 1] + fitness[i];
}

/*
 * Function: find_fitness()
 * Purpose: Calculate fitness for given member (take average)
 */
void
find_fitness(double *results, double *fitness)
{
	int i;
	double total;

	for (i = 0; i < N_POP; i++)
		total += results[i];

	for (i = 0; i < N_POP; i++)
		fitness[i] = results[i] / total;
}

/*
 * Function: calc_function()
 * Purpose: Perform math function against entire population
 */
void
calc_function(int *pop, double *results)
{
	int i;

	for (i = 0; i < N_POP; i++)
		results[i] = do_func(pop[i]);
}

/*
 * Function: init_pop()
 * Purpose: Initialize population by generating random values
 */
void
init_pop(u_int * pop)
{
	int i;

	for (i = 0; i < N_POP; i++)
		pop[i] = (u_int) arc4random() % 255;
}

/*
 * Function: do_func()
 * Purpose: f(x) = sin(pi * x / 256)
 */
double
do_func(int x)
{
	return (sin(M_PI * x / 256));
}
