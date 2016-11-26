#ifndef GENETIC_H
#define GENETIC_H

#include <string>
#include <vector>
#include <utility>
#include "Client.h"
#include "Package.h"

//! Mutation structure to ease passing multiple values to genetic algorithm.
typedef struct {
    float swapWithin;
    float swapOut;
    float crossOver;
    float insertNew;
    float deleteOld;
    float inversion;
    float elite;
    float total;

} mutation_enum;

class Genetic {
    public:
        //! Creates and initializes a genetic routing object.
        Genetic(std::vector<Package*> packs,
                 std::vector<vector<unsigned int> > matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive, float shiftTime, unsigned long gens, mutation_enum mut);
        virtual ~Genetic();

        //! Runs the simulation and "evolves" the best route.
        /*! \return A package vector of the best fit route found given the constraints and random chance.
         */
        vector<Package* > evolve();

        vector<Package* > getBest() {return bestFit;};

        vector< pair<vector<Package* >, float> > evolve_threads();

        //! Calculates fitness of individual based on route.
        /*! \param individual The route to evaluate the fitness of.
         *  \return A vector of fitness values. Individual fitness score, priority package points, distance traveled, shift time, weight.
         */
        vector<float> fitness(vector<Package* > individual);

        //! Create initial route population.
        void initPopulation();

        void loadPopulation(vector< pair<vector<Package* >, float> > newPopulation);

        void printGene(vector<Package* > gene);

    protected:

    private:

        void initRanking(float exponent);

        //! Select and create new individuals for the next population.
        void mate();

        //! Handles mutation of current population based on probabilities passed during creation.
        vector<vector<Package* > > mutate();

        //! Crosses over genes between two parents.
        /*! \param gene1 Gene sequence of parent 1.
         *  \param gene2 Gene sequence of parent 2.
         * \return Two new individuals crossed over from parent inputs in a random crossover point. Genes may be shorter due to removal of duplicate packages.
         */
        vector<vector<Package*> > crossOver(vector<Package* > gene1, vector<Package* > gene2);

        //! Mutates parent gene by randomly inserting a new package somewhere within the parent.
        /*! \param gene Gene to be mutated.
         *  \return Gene that has been mutated.
         */
        vector<Package* > mutateInsert(vector<Package *> gene);

        //! Mutates parent gene by randomly deleting a gene within the parent.
        /*! \param gene Gene to be mutated.
         *  \return Gene that has been mutated.
         */
        vector<Package* > mutateDelete(vector<Package* > gene);

        //! Mutates parent gene by randomly swapping two genes somewhere within the parent.
        /*! \param gene Gene to be mutated.
         *  \return Gene that has been mutated.
         */
        vector<Package* > mutateSwapWithin(vector<Package *> gene);

        //! Mutates parent gene by randomly swapping an existing gene within the parent with a new non-duplicate package.
        /*! \param gene Gene to be mutated.
         *  \return Gene that has been mutated.
         */
        vector<Package* > mutateSwapNew(vector<Package* > gene);

        //! Mutates parent gene by randomly selecting a section of the gene, \f$i\dots\f$, and reversing the genes sequence.
        /*! \param gene Gene to be mutated.
         *  \return Gene that has been mutated.
         */
        vector<Package* > mutateInversion(vector<Package* > gene);

        //! Merge lists for merge sort. For packages based on fitness value in route vector.
        void mergeLists(unsigned long i, unsigned long m, unsigned long j);
        //! Merge sort for packages based on fitness value in route vector.
        void mergeSort(unsigned long i, unsigned long j);



        //! Vector of pointers to all potential packages to consider routing.
        vector<Package* > packages;
        //! Size of vector of potential packages. Saves from expensive .size() calls.
        unsigned int numOfPackages;

        //! Adjacency matrix of clients in package list.
        /*! Holds the matrix for all packages being considered. Currently indexed
         *  via client ID numbers, with smallest client ID as lower indices.
         *  Matrix holds the Manhattan distance from each receiving client to all
         *  other clients. A completely connected, undirected graph.
         */
        vector<vector<unsigned int> > adMatrix;

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

        //! The current population of "routes."
        /*! A list (vector) of individuals in the population.
         *  Each entry in the vector is a pair.
         *  The first item in the pair is the gene of the individual.
         *  This is a vector of the packages in that route.
         *  The second item in the pair is that individual's fitness value.
         */
        vector< pair<vector<Package* >, float> > genes;

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
        vector<float> bestFitInfo;

        //! Mutation probabilities
        mutation_enum mutation;

        //! Number of high ranking, or elite, members of the population that survive into the next population instead of being replaced.
        unsigned int elitist;

        //! Average ranking for parents in roulette. Debug.
        float avgIndividual = 0;

};

#endif // GENETIC_H
