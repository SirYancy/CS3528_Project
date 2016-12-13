#ifndef GENETIC_H
#define GENETIC_H

#include <string>
#include <vector>
#include <utility>
#include "Client.h"
#include "Package.h"
#include <iostream>
#include <cmath>
//#include <thread>
#include <future>
#include <random>
#include <algorithm>
#include <unordered_map>

#define OVERNIGHT_WEIGHT  8
#define TWODAY_WEIGHT     4
#define REGULAR_WEIGHT    1

//! Mutation structure to ease passing multiple values to genetic algorithm.
/*! This structure holds multiple mutation probabilities. All mutation probabilities
 *  (not including elite or crossover) are totaled. These mutation probabilities should add
 *  up to less than 1 to allow gene to escape unaltered. Future additions will include
 *  an "other" probability like the package_generator program.
 */
typedef struct {
    float swapWithin;   //!< Probability to swap two genes within genome. Normally given as decimal percentage.
    float swapOut;      //!< Probability to swap one gene with another "outside" gene not already in the genome.
    float crossOver;    //!< Decimal percentage of total population that is randomly selected for crossover.
    float insertNew;    //!< Probability of inserting a brand new, non-duplicate gene.
    float deleteOld;    //!< Probability of deleting an existing gene.
    float inversion;    //!< Probability of inverting between two random points within the genome.
    float elite;        //!< Percentage of the fittest individuals that are saved to next generation.
    float total;        //!< Total add up of certain probabilities.

} mutation_struct;

class Genetic {
    public:

        //! Structure to hold genome information used by GA.
        typedef struct {
            vector<double> fitnessValue;    //!< Fitness of this genome.
            size_t hashValue;               //!< Hash of this genome.
            unsigned int sizeValue;         //!< Size of this genome in number of packages. Speeds up by not consistently calling size() method.
        } geneInfo;

        //! Creates and initializes a genetic routing object.
        /*! \param packs The package vector list of all eligible packages for a route.
         *  \param matrix Manhattan distance adjacency matrix for packages passed into GA object. Indexed by client ID.
         *  \param packLimit A max limit on the number of packages a truck, or the GA, can consider a genome to hold.
         *  \param population The number of individuals within the GA to evolve. Greater individuals yield more diverse searching and better results.
         *  \param stops The time a package delivery stop takes.
         *  \param drive The time it takes the truck to travel one block.
         *  \param shiftTime The maximum shift time allowable that the GA will attempt to be under.
         *  \param gens The number of generations to run the GA before returning the most fit individual (or population.)
         *  \param mut The desired mutation parameters. See mutation_struct.
         *  \param weight The maximum allowable route weight.
         */
        Genetic(std::vector<Package*> packs,
                 std::vector<vector<unsigned int> > matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive, float shiftTime, unsigned long gens, mutation_struct mut);
        virtual ~Genetic();

        //! Runs the simulation and "evolves" the best route.
        /*! \return A package vector of the best fit route found given the constraints and random chance.
         */
        vector<Package* > evolve();

        //! Returns best fit individual route found from previous evolution.
        /*! \return Returns the vector of best fit route found from previous evolution.
        */
        vector<Package* > getBest() {return bestFit;};

        //! Similar to evolve. Evolves population to be used with a threading environment.
        /*! This evolves a population for multiple threading implementations. In multi-threading
         *  this method is called to evolve isolated (and therefore more diverse) population.
         *  The returned population from this method can then be mixed with other populations
         *  and fed back into loadPopulation and invoke evolve to better explore the search space.
         *  \return Returns a complete population vector to be later fed back into loadPopulation.
         */
        vector< pair<vector<Package* >, geneInfo> > evolve_threads();

        //! Calculates fitness of individual based on route.
        /*! \param individual The route to evaluate the fitness of.
         *  \return A vector of fitness values. Individual fitness score, priority package points, distance traveled, shift time, weight.
         */
        vector<double> fitness(vector<Package* >* individual);

        //! Create random initial route population.
        void initPopulation();

        //! Load a population of individuals to evolve.
        /*! This method loads a population of individual routes from previous invocations
         *  of evolve_threads. The method evolve or evolve_threads would then be called.
         *  \param newPopulation A population to load into the genome vector for further evolving.
         */
        void loadPopulation(vector< pair<vector<Package* >, geneInfo> > newPopulation);

        //! Printing a genome for debugging or fun and profit.
        /*! \param gene The genome of packages one wishes to print out.
        */
        void printGene(vector<Package* >* gene) const;

        //! Creates a hash of a genome.
        /*! Hashes a genome of packages for internal use, or external use.
         *  \param gene The genome to hash.
         *  \return A size_t integer representing the hashed value.
         */
        size_t hash(vector<Package* >* gene) const;

        //! Perform 2-opt (2-optimal inversion) on a gene, looping until no improvement is seen.
        /*! \param gene A gene vector of packages \b not including the depot end-points.
         *  \return A vector of two-optimized packages.
         */
        vector<Package* > twoOpt(vector<Package* >* gene);

    protected:

    private:

        //! 2-opt optimization of the entire population.
        /*! Performs 2-opt optimization of the population in-place to reduce route lengths.
         *  This is used to better the population more quickly to enable a higher
         *  package count, reducing the GA's need to optimize the route will searching
         *  for "good" packages.
         */
        void twoOptPopulation();

        //! Initiates the exponential ranking for individual selection.
        /*! Initiates the ranking used in crossover selection. Ranking vector
         *  needs to be sized to the population size.
         *  \param exponent The exponential value to use in the exponential ranking.
         */
        void initRanking(float exponent);

        //! Select and create new individuals for the next population.
        void mate();

        //! Handles mutation of current population based on probabilities passed during creation.
        /*! /param choosen The genome to mutate.
         */
        void mutate(vector<Package* >* choosen);

        //! Crosses over genes between two parents.
        /*! \param gene1 Gene sequence of parent 1.
         *  \param gene2 Gene sequence of parent 2.
         *  \return Two new individuals crossed over from parent inputs in a random crossover point. Genes may be shorter due to removal of duplicate packages.
         */
        vector<vector<Package*> > crossOver(vector<Package* >* gene1, vector<Package* >* gene2);

        //! Mutates parent gene by randomly inserting a new package somewhere within the parent.
        /*! \param gene Gene to be mutated.
         *  \param location Index to start or apply mutation.
         *  \return Gene that has been mutated.
         */
        void mutateInsert(vector<Package *>* gene, unsigned int location);

        //! Mutates parent gene by randomly deleting a gene within the parent.
        /*! \param gene Gene to be mutated.
         *  \param location Index to start or apply mutation.
         *  \return Gene that has been mutated.
         */
        void mutateDelete(vector<Package* >* gene, unsigned int location);

        //! Mutates parent gene by randomly swapping two genes somewhere within the parent.
        /*! \param gene Gene to be mutated.
        *  \param location Index to start or apply mutation.
         *  \return Gene that has been mutated.
         */
        void mutateSwapWithin(vector<Package *>* gene, unsigned int location);

        //! Mutates parent gene by randomly swapping an existing gene within the parent with a new non-duplicate package.
        /*! \param gene Gene to be mutated.
         *  \param location Index to start or apply mutation.
         *  \return Gene that has been mutated.
         */
        void mutateSwapNew(vector<Package* >* gene, unsigned int location);

        //! Mutates parent gene by randomly selecting a section of the gene, \f$i\dots\f$, and reversing the genes sequence.
        /*! \param gene Gene to be mutated.
         *  \param location Index to start or apply mutation.
         *  \return Gene that has been mutated.
         */
        void mutateInversion(vector<Package* >* gene, unsigned int location);

        //! Merge lists for merge sort. For packages based on fitness value in route vector.
        void mergeLists(unsigned long i, unsigned long m, unsigned long j);

        //! Merge sort for packages based on fitness value in route vector.
        void mergeSort(unsigned long i, unsigned long j);


        //! Adjacency matrix of clients in package list.
        /*! Holds the matrix for all packages being considered. Currently indexed
         *  via client ID numbers, with smallest client ID as lower indices.
         *  Matrix holds the Manhattan distance from each receiving client to all
         *  other clients. A completely connected, undirected graph.
         */
        vector<vector<unsigned int> > adMatrix;

        //! Choose crossover parents.
        /*! Chooses unique parents for using in crossOver.
         *  \return Returns an integer pair indices to which individuals to pass to crossOver.
         */
        pair<int, int> chooseParents();

        //! Population size for evolution. Population size will be rounded down to multiples of 4 (threading.)
        unsigned int popNum;

        //! Maximum possible priority score.
        unsigned int totalPriority = 0;

        //! Weight limit for route. Truck can hold this much. Ounces.
        unsigned int weightLimit;
        //! Maximum number of packages for route. Not currently implemented.
        unsigned int packageLimit;
        //! Time limit for driver shift.
        float timeLimit;

        //! Stop time in minutes for package delivery
        float stopTime;

        //! Drive time in minutes for one block.
        /*! Used with the adjacency matrix to determine drive time based on output value of matrix.
         */
        float driveTime;

        //! Number of desired generations.
        unsigned long generations;

        //! Current generation number
        unsigned long currentGeneration = 0;

        //! The current population of "routes."
        /*! A list (vector) of individuals in the population.
         *  Each entry in the vector is a pair.
         *  The first item in the pair is the gene of the individual.
         *  This is a vector of the packages in that route.
         *  The second item in the pair is that individual's geneInfo (fitness, hash, etc.)
         */
        vector< pair<vector<Package* >, geneInfo> > genes;


        //! Vector of pointers to all potential packages to consider routing.
        vector<Package* > packages;
        //! Size of vector of potential packages. Saves from expensive .size() calls.
        unsigned int numOfPackages;

        //! Ranking vector for population used in parent selection for mutation.
        vector<double> ranking;

        //! Ranking total for random selection in roulette selection.
        double rankTotal = 0;

        //! Length of ranking array. Same as population.
        unsigned int rankingSize;

        //! New population individual for threads. Two offspring.
        vector<vector<Package* > > newIndividual1;
        //! New population individual for threads. Two offspring.
        vector<vector<Package* > > newIndividual2;
        //! New population individual for threads. Two offspring.
        vector<vector<Package* > > newIndividual3;
        //! New population individual for threads. Two offspring.
        vector<vector<Package* > > newIndividual4;

        //! Best route found so far.
        vector<Package* > bestFit;

        //! Best route information for console output.
        vector<double> bestFitInfo;

        //! Gene info of best genome found so far. Should be used instead of bestFit info.
        geneInfo bestGeneInfo;

        //! Mutation probabilities
        mutation_struct mutation;

        //! Number of high ranking, or elite, members of the population that survive into the next population instead of being replaced.
        unsigned int elitist;

        //! Average ranking for parents in roulette. Debug.
        float avgIndividual = 0;

};

#endif // GENETIC_H
