#ifndef GENETIC_H
#define GENETIC_H

#include <string>
#include <vector>
#include <utility>
#include "Client.h"
#include "Package.h"


class Genetic {
    public:
        //! Default constructor */
        Genetic(vector<Package*> packs, vector<vector<unsigned int> > matrix, unsigned int weight, unsigned int packLimit, unsigned int population, float stops, float drive, float shiftTime, float mutateIn, float mutateSw, float mutateLen);
        //! Default destructor */
        virtual ~Genetic();

        vector<Package* > evolve();


    protected:

    private:
        void initPopulation();
        void mate();

        vector<Package*> crossOver(vector<Package* > gene1, vector<Package* > gene2);
        vector<float> fitness(vector<Package* >);

        void mergeLists(unsigned long i, unsigned long m, unsigned long j);
        void mergeSort(unsigned long i, unsigned long j);

        vector<Package* > packages;
        vector<vector<unsigned int> > adMatrix;

        unsigned int popNum;

        unsigned int weightLimit;
        unsigned int packageLimit;
        float timeLimit;

        float stopTime;
        float driveTime;

        //! The current population of "routes."
        /*! A list (vector) of individuals in the population.
         *  Each entry in the vector is a pair.
         *  The first item in the pair is the gene of the individual.
         *  This is a vector of the packages in that route.
         *  The second item in the pair is that individual's fitness value.
         */
        vector< pair<vector<Package* >, float> > genes;
        //! Best route found so far.
        vector<Package* > bestFit;
        vector<float> bestFitInfo;

        float mutation[4] = {0, 0, 0};
        float mutationTotal = 0;

};

#endif // GENETIC_H
