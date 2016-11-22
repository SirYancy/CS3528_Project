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
        Genetic(vector<Package*>* packs, vector<vector<unsigned int> >* matrix, unsigned int weight, unsigned int packLimit, unsigned int population, float stops, float drive);
        //! Default destructor */
        virtual ~Genetic();

        void initPopulation();

        float fitness(vector<Client*>);

    protected:

    private:
        vector<Package*>* packages;
        vector<vector<unsigned int> >* adMatrix;

        unsigned int popNum;

        unsigned int weightLimit;
        unsigned int packageLimit;
        float stopTime;
        float driveTime;

        vector<vector<Client*> > genes;
        vector<Client* > bestFit;
};

#endif // GENETIC_H
