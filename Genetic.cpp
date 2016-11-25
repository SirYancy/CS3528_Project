#include "Genetic.h"
//#include "Client.h"
//#include "Package.h"
#include <vector>
#include <iostream>
#include <cmath>
//#include <thread>
#include <future>

Genetic::Genetic(std::vector<Package*> packs,
                 std::vector<vector<unsigned int> > matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive, float shiftTime, unsigned long gens, mutation_enum mut)
                 : packages(packs), adMatrix(matrix), weightLimit(weight), packageLimit(packLimit),
                 stopTime(stops), driveTime(drive), timeLimit(shiftTime), generations(gens), mutation(mut)

{
    // Truncate population to multiples of four for threading.
    popNum = (population / 4) * 4;

    std::cout << "Population: " << popNum << std::endl;

    // Total for roulette selection of mutation.
    mutation.total = mutation.crossOver + mutation.deleteOld + mutation.insertNew + mutation.inversion + mutation.swapOut + mutation.swapWithin;

    // Create the best fitness info vector for later indexing.
    for (unsigned int i = 0; i < 5; ++i) {
        bestFitInfo.push_back(0);
    }

    // How many packages are there in the pool.
    numOfPackages = packages.size();

    // How many good members will be carried over into the next population
    elitist = mutation.elite * popNum;

    for (vector<Package* >::iterator iter = packages.begin(); iter != packages.end(); ++iter) {
        totalPriority += static_cast<unsigned int>((*iter)->getPriority());
    }
}

Genetic::~Genetic()
{
    //dtor
}

void Genetic::initPopulation() {
    std::vector<Package* > individual;

    float totalWeight = 0;
    unsigned int totalPackages = 0;
    unsigned int maxTries = 1000;
    unsigned int tries = 0;


    Package* currentPackage;
    unsigned int randomPackage;
    bool present = false;

    for (unsigned int i = 0; i < popNum; i++) {
        //std::cout << "Individual " << i << std::endl;

        // Clear individual
        individual.clear();
        totalWeight = 0;
        totalPackages = 0;
        tries = 0;

        // Loop while we still have packages to place, and we're under the limits.
        while (totalPackages < numOfPackages && totalPackages < packageLimit && totalWeight < weightLimit && (totalPackages * stopTime) < (timeLimit / 2) && tries < maxTries) {

            tries++;

            // Random index to packages
            randomPackage = rand() % numOfPackages;

            //std::cout << "Random package: " << randomPackage << std::endl;
            // Get the package pointer
            currentPackage = packages[randomPackage];

            //std::cout << "Package ptr: " << currentPackage << std::endl;
            // Assume not present in current individual
            present = false;

            // Loop over the individual looking for duplicate package
            for (std::vector<Package*>::iterator iter = individual.begin(); iter != individual.end(); ++iter) {
                // Check duplicate
                if (*iter == currentPackage) {
                    present = true;
                    //std::cout << "Package already present: " << currentPackage << std::endl;
                    break;
                }
            }

            // If not a duplicate
            if (present == false) {
                // Over truck limit if added?
                if (currentPackage->getWeight() + totalWeight <= weightLimit) {
                    // Add package to individual
                    individual.push_back(currentPackage);
                    //std::cout << "Adding package ptr: " << currentPackage << std::endl;
                    totalPackages++;
                    totalWeight += currentPackage->getWeight();

                }

            }
        }
        //std::cout << "Current weight: " << totalWeight << endl;
        genes.push_back(std::make_pair(individual, 0));
    }

    // Resize the ranking vector to evolution.
    ranking.resize(genes.size());

    // Load exponential ranking array. Always the same number to individuals, so same ranking
    for (unsigned int i = 0; i < ranking.size(); ++i) {
        // Exponential ranking
        ranking[i] = pow(i+1, 3);
        //std::cout << "Ranking[" << i << "]=" << ranking[i] << std::endl;
        // Add rank to total
        rankTotal += ranking[i];
    }
}

vector<float> Genetic::fitness(vector<Package* > individual) {
    unsigned int distance = 0;
    unsigned int shiftTime = 0;
    unsigned int priorities = 0;
    float weight = 0;

    unsigned int previousIndex, currentIndex;

    float indFit = 0;

    previousIndex = 0;

    for (std::vector<Package*>::iterator iter = individual.begin(); iter != individual.end(); ++iter) {
        currentIndex = (*iter)->getReceiver()->getID();
        //std::cout << currentIndex << std::endl;
        distance += adMatrix[previousIndex][currentIndex];

        previousIndex = currentIndex;
        priorities += static_cast<unsigned int>((*iter)->getPriority());
        weight += (*iter)->getWeight();

        //std::cout << "Package priority: " << static_cast<int>((*iter)->getPriority()) << std::endl;
        shiftTime += stopTime;
    }

    distance += adMatrix[previousIndex][currentIndex];

    shiftTime += distance * driveTime;

    //std::cout << "Distance: " << distance << " Time: " << shiftTime << " Priorities: " << priorities << " Weight: " << weight << " Len: " << individual.size() << std::endl;

    indFit = 0;
    if (shiftTime > timeLimit) {
        indFit -= pow(shiftTime - timeLimit, 1.55);
    } else {
        //indFit += pow(static_cast<float>(timeLimit) - static_cast<float>(shiftTime), 1.05);
        indFit += pow(priorities, 1.0 + ((static_cast<float>(timeLimit) - static_cast<float>(shiftTime)) / static_cast<float>(shiftTime)));
    }

    //std::cout << 2.0 + ((static_cast<float>(timeLimit) - static_cast<float>(shiftTime)) / static_cast<float>(shiftTime)) << std::endl;
    //indFit += pow((static_cast<float>(OVERNIGHT) * static_cast<float>(individual.size()))/static_cast<float>(priorities), 1.5);
    indFit += pow(static_cast<float>(priorities), 1.57);


    if (weight > weightLimit) {
        indFit -= pow(weight - weightLimit, 1.4);
    }
    //indFit = pow(timeLimit/static_cast<float>(shiftTime),2) + (static_cast<float>(OVERNIGHT) * static_cast<float>(individual.size()))/static_cast<float>(priorities);// + pow(static_cast<float>(weightLimit - weight), 2);

    //std::cout.precision(17);
    //std::cout << 1/static_cast<float>(shiftTime) << std::endl;
    //std::cout << "Fit: " << indFit << std::endl;
    //std::cout << "OVERNIGHT: " << static_cast<int>(OVERNIGHT) << std::endl;
    vector<float> fit {indFit, priorities, distance, shiftTime, weight};
    return fit;
}

void Genetic::evolve_threads() {
    typedef vector<Package* > route;

    auto f1 = std::async(std::launch::deferred, &Genetic::evolve, this);
    auto f2 = std::async(std::launch::deferred, &Genetic::evolve, this);

    // These do not work. I believe it is due to requiring a reference to evolve. I need
    // a way to create a "copy" of this object, or it's data members to allow simultanous threads accessing the data.
    //auto f3 = std::async(std::launch::async, &Genetic::evolve, this);
    //auto f4 = std::async(std::launch::async, &Genetic::evolve, this);

    auto res1 = f1.get();
    auto res2 = f2.get();
    //auto res3 = f3.get();
    //auto res4 = f4.get();

/*
    auto f1 = std::async(std::launch::async, &Genetic::thread1, this);
    auto f2 = std::async(std::launch::async, &Genetic::thread2, this);

    auto res1 = f1.get();
    auto res2 = f2.get();

    std::cout << res1 << " " << res2 << std::endl;
*/
}


vector<Package * > Genetic::evolve() {

    initPopulation();
    vector<float> currentFitness (5, 0);
    vector<float> currentBest (5, 0);

    std::vector<std::pair<std::vector<Package*>, float> >::iterator row;

    for (unsigned long i = 0; i < generations; ++i) {
            currentBest[0] = 0;
        for (row = genes.begin(); row != genes.end(); ++row) {
                //std::cout << "Current iter ptr: " << (*row) << std::endl;
                currentFitness = fitness((*row).first);
                (*row).second = currentFitness[0];
                if (currentFitness[0] > currentBest[0] || currentBest[0] == 0) {
                    currentBest = currentFitness;
                }

                if (currentFitness[0] > bestFitInfo[0] || bestFitInfo[0] == 0) {
                    bestFit = (*row).first;
                    bestFitInfo = currentFitness;
                }
                //std::cout << "Current fitness: " << currentFitness << " Best: " << bestFitScore << std::endl;
        }

        if (i % 1000 == 0) {
            std::cout << "Generation " << i << " Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << "/" << numOfPackages << " CO: " << avgIndividual << std::endl;//" F: " << genes[popNum - elitist - 2].second << " P: " << currentBest[1] << " D: " << currentBest[2] << " T: " << currentBest[3] << "/" << timeLimit << " W: " << currentBest[4] << "/" << weightLimit << std::endl;
        }
        mate();
    }
    std::cout << "Best route F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;

    return getBest();
}

void Genetic::mate() {
    //vector<unsigned int> ranking (genes.size(), 0);

    vector< pair<vector<Package* >, float> > newPopulation;

    newPopulation.resize(genes.size());

    vector< vector<Package* > > newIndividuals;

    // Sort "in-place" based on fitness value. Least fit routes first in the vector, most fit last.
    mergeSort(0, genes.size() - 1);

    for (unsigned int i = 0; i < (popNum - elitist) / 2; ++i) {


        newIndividuals = mutate();
        newPopulation[2 * i] = make_pair(newIndividuals[0], 0);
        newPopulation[2 * i + 1] = make_pair(newIndividuals[1], 0);

        /*
        thread t1(&Genetic::crossOver, this, 1);
        thread t2(&Genetic::crossOver, this, 2);
        thread t3(&Genetic::crossOver, this, 3);
        thread t4(&Genetic::crossOver, this, 4);
        t1.join();
        t2.join();
        t3.join();
        t4.join();

        newGenes.push_back(make_pair(newIndividual1, 0));
        newGenes.push_back(make_pair(newIndividual2, 0));
        newGenes.push_back(make_pair(newIndividual3, 0));
        newGenes.push_back(make_pair(newIndividual4, 0));
        */
    }

    //std::cout << "Before elite: " << newPopulation.size();
    // Save the elite few, the fitest. Save the Queen!
    for (unsigned int i = popNum - elitist - 1; i < popNum; ++i) {
        newPopulation[i] = genes[i];
    }

    //std::cout << " After elite: " << newPopulation.size() << std::endl;
    genes = newPopulation;
}

vector<vector<Package*> > Genetic::crossOver(vector<Package* > gene1, vector<Package* > gene2) {
    //std::cout << "Doing crossover on " << gene1->getPointer() << " and " << gene2->getPointer() << std::endl;
    unsigned int geneLength1 = gene1.size();
    unsigned int geneLength2 = gene2.size();

    unsigned int smallestLength, longestLength, randomPoint;

    bool present;

    vector<Package* > packageGene1;
    vector<Package* > packageGene2;
    vector<vector<Package* > > newIndividuals;


    if (geneLength1 < geneLength2) {
        smallestLength = geneLength1;
        //whichFirst = 0;

        longestLength = geneLength2;

    } else if (geneLength2 < geneLength1) {
        smallestLength = geneLength2;
        //whichFirst = 1;

        longestLength = geneLength1;


    } else {
        smallestLength = geneLength1;
        longestLength = geneLength1;

    }

    randomPoint = rand() % smallestLength;
/*
    std::cout << "Gene1:" << std::endl;
    for(std::vector<Package*>::iterator iter = gene1.begin(); iter != gene1.end(); ++iter) {
        std::cout << (*iter) << std::endl;
    }

    std::cout << std::endl << "Gene2:" << std::endl;
    for(std::vector<Package*>::iterator iter = gene2.begin(); iter != gene2.end(); ++iter) {
        std::cout << (*iter) << std::endl;
    }
*/


    //std::cout << std::endl << "Random point: " << randomPoint << std::endl;
    //std::cout << "G1 size: " << geneLength1 << " G2 size: " << geneLength2 << std::endl;

    // Genes crosses over from 0 to randomPoint in genes
    for (unsigned int i = 0; i <= randomPoint; ++i) {
        packageGene1.push_back(gene2[i]);
        packageGene2.push_back(gene1[i]);
    }

    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene1_iter = gene1.begin(); gene1_iter != gene1.end(); ++gene1_iter) {
        //std::cout << "Size: " << newIndividual.size() - 1 << std::endl;

        // Assume no duplicates
        present = false;

        // Loop through all genes already places
        for (vector<Package* >::iterator package1_iter = packageGene1.begin(); package1_iter != packageGene1.end(); ++package1_iter) {

            //std::cout << i << " " << j << " Ind: " << newIndividual[j] << " Gene: " << gene2[i] << std::endl;
            // Is potential gene a duplicate?
            if (*package1_iter == *gene1_iter) {
                //std::cout << "Gene already present " << gene2[i] << std::endl;
                present = true;
                break;
            }
        }

        // If not a duplicate, insert.
        if (present == false) {
            packageGene1.push_back(*gene1_iter);
            //std::cout << "Adding G1: " << (*gene1_iter)->getPointer() << std::endl;
        }
    }

    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene2_iter = gene2.begin(); gene2_iter != gene2.end(); ++gene2_iter) {
        //std::cout << "Size: " << newIndividual.size() - 1 << std::endl;

        // Assume no duplicates
        present = false;

        // Loop through all genes already places
        for (vector<Package* >::iterator package2_iter = packageGene2.begin(); package2_iter != packageGene2.end(); ++package2_iter) {

            //std::cout << i << " " << j << " Ind: " << newIndividual[j] << " Gene: " << gene2[i] << std::endl;
            // Is potential gene a duplicate?
            if (*package2_iter == *gene2_iter) {
                //std::cout << "Gene already present " << gene2[i] << std::endl;
                present = true;
                break;
            }
        }

        // If not a duplicate, insert.
        if (present == false) {
            packageGene2.push_back(*gene2_iter);
            //std::cout << "Adding G2: " << (*gene2_iter)->getPointer() << std::endl;
        }
    }

/*
    std::cout << "Baby crossover 1: " << std::endl;
    for(vector<Package* >::iterator iter=packageGene1.begin(); iter != packageGene1.end(); ++iter) {
        std::cout << (*iter)->getPointer() << std::endl;
    }

    std::cout << "Baby crossover 2: " << std::endl;
    for(vector<Package* >::iterator iter=packageGene2.begin(); iter != packageGene2.end(); ++iter) {
        std::cout << (*iter)->getPointer() << std::endl;
    }
*/
    newIndividuals.push_back(packageGene1);
    newIndividuals.push_back(packageGene2);

    //std::cout << "Returning " << &newIndividuals << std::endl;
    return newIndividuals;
}

vector<vector<Package* > > Genetic::mutate() {
    vector<vector<Package* > > newIndividuals;
    newIndividuals.resize(2);

    unsigned int smallestLength;
    unsigned int longestLength;
    unsigned int randomPoint;
    unsigned int whichFirst;
    unsigned int randomNewGene;
    unsigned int tries = 0;


    int randomIndividual1 = 0;
    int randomIndividual2 = 0;

    float randomP;
    unsigned int randomSwap1, randomSwap2;
    bool present = false;


    // Loop until we get different individuals
    while (randomIndividual1 == randomIndividual2) {
        randomIndividual1 = rand() % rankTotal;
        randomIndividual2 = rand() % rankTotal;


        for (unsigned int i = 0; i < ranking.size(); ++i) {
            //std::cout << "In1: " << randomIndividual1 << "/" << ranking[i] << " " << i << std::endl;
            if (randomIndividual1 < ranking[i]) {
                randomIndividual1 = i;
                avgIndividual = avgIndividual * 100 + static_cast<float>(i);
                //std::cout << "Individual 1: " << i << std::endl;
                // Done searching.
                break;
            } else {
                randomIndividual1 -= ranking[i];
            }

        }

        for (unsigned int i = 0; i < ranking.size(); ++i) {
            //std::cout << "In2: " << randomIndividual2 << "/" << ranking[i] << " " << i << std::endl;
            if (randomIndividual2 < ranking[i]) {
                randomIndividual2 = i;
                avgIndividual = (avgIndividual + static_cast<float>(i)) / 102;
                //std::cout << "Individual 2: " << randomIndividual2 << std::endl;

                // Done searching.
                break;
            } else {
                randomIndividual2 -= ranking[i];
            }

        }
    }

    // Grab selected parents genes.
    vector<Package* > gene1 = genes[randomIndividual1].first;
    vector<Package* > gene2 = genes[randomIndividual2].first;

    //std::cout << "New baby of " << gene1[0]->getPointer() << " and " << gene2[0]->getPointer() << std::endl;

    randomP = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * mutation.total;


    if (randomP < mutation.crossOver) {

        newIndividuals = crossOver(gene1, gene2);
        return newIndividuals;
        //std::cout << "Got in return " << newIndividual
    }


    randomP -= mutation.crossOver;

    if (randomP < mutation.deleteOld) {

        newIndividuals[0] = mutateDelete(gene1);
        newIndividuals[1] = mutateDelete(gene2);
        return newIndividuals;
    }

    randomP -= mutation.deleteOld;

    if (randomP < mutation.insertNew) {

        newIndividuals[0] = mutateInsert(gene1);
        newIndividuals[1] = mutateInsert(gene2);
        return newIndividuals;
    }

    randomP -= mutation.insertNew;

    if (randomP < mutation.inversion) {

        newIndividuals[0] = mutateInversion(gene1);
        newIndividuals[1] = mutateInversion(gene2);
        return newIndividuals;
    }

    randomP -= mutation.inversion;

    if (randomP < mutation.swapOut) {
        newIndividuals[0] = mutateSwapNew(gene1);
        newIndividuals[1] = mutateSwapNew(gene2);
        return newIndividuals;
    }

    randomP -= mutation.swapOut;

    // End of the road. Swap genes within each parent.
    newIndividuals[0] = mutateSwapWithin(gene1);
    newIndividuals[1] = mutateSwapWithin(gene2);

    return newIndividuals;

}

vector<Package* > Genetic::mutateInsert(vector<Package *> gene) {
    vector<Package* > newGene;

    // Size of gene
    unsigned int geneSize = gene.size();

    // Where insertion point is
    unsigned int randomPoint = rand() % geneSize;

    unsigned int randomNewGene;

    // Is random package already present in gene
    bool present = true;

    // Limit to number of tries.
    unsigned int tries = 0;

    // Attempt to get non-duplicate package
    while(present == true && tries < 200) {
        // Assume not present in gene
        present = false;

        // Random package
        randomNewGene = rand() % numOfPackages;

        // Check gene for duplicates
        for(unsigned int swapIn = 0; swapIn < geneSize; ++swapIn) {
            if (packages[randomNewGene] == gene[swapIn]) {
                // Duplicate
                present = true;
                break;
            }
        }

        tries++;
    }

    if (tries < 200) {
        // Time to insert, point point
        randomPoint = rand() % geneSize;

        // Jam it in
        gene.insert(gene.begin() + randomPoint, packages[randomNewGene]);
    }

    // Return gene
    newGene = gene;
    return newGene;

}

vector<Package* > Genetic::mutateInversion(vector<Package* > gene) {
    // Size of gene
    unsigned int geneSize = gene.size();
    vector<Package* > newGene;

    unsigned int randomSwap1 = 0;
    unsigned int randomSwap2 = 0;

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        //std::cout << "Swapping genes..." << std::endl;

        // Find differing indices
        while(randomSwap1 >= randomSwap2) {
            randomSwap1 = rand() % (geneSize);
            randomSwap2 = rand() % (geneSize);
        }

        for (unsigned int i = 0; i < (randomSwap2 - randomSwap1) / 2; ++i) {
            // Swap genes
            Package* temp = gene[randomSwap1 + i];
            gene[randomSwap1 + i] = gene[randomSwap2 - i];

            gene[randomSwap2 - i] = temp;
        }

    }
    // Return gene
    newGene = gene;
    return newGene;
}


vector<Package* > Genetic::mutateDelete(vector<Package* > gene) {
    // Size of gene
    unsigned int geneSize = gene.size();
    vector<Package* > newGene;

    unsigned int randomPoint;

    // Delete a gene.
    // Only delete if there is a spare gene to delete.
    if (geneSize >= 2) {
        // Random point
        randomPoint = rand() % geneSize;
        // Remove gene.
        gene.erase(gene.begin() + randomPoint);
    }
    newGene = gene;
    return newGene;
}


vector<Package* > Genetic::mutateSwapWithin(vector<Package *> gene) {
    // Size of gene
    unsigned int geneSize = gene.size();
    vector<Package* > newGene;

    unsigned int randomSwap1 = 0;
    unsigned int randomSwap2 = 0;

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        //std::cout << "Swapping genes..." << std::endl;

        // Find differing indices
        while(randomSwap1 == randomSwap2) {
            randomSwap1 = rand() % (geneSize);
            randomSwap2 = rand() % (geneSize);
        }

        // Swap genes
        Package* temp = gene[randomSwap1];
        gene[randomSwap1] = gene[randomSwap2];

        gene[randomSwap2] = temp;
    }
    // Return gene
    newGene = gene;
    return newGene;
}


vector<Package* > Genetic::mutateSwapNew(vector<Package* > gene) {
    unsigned int geneSize = gene.size();
    vector<Package* > newGene;

    bool present = true;
    unsigned int tries = 0;
    unsigned int randomNewGene;

    // Swap package outside this gene at this index point.
    unsigned int randomPoint = rand() % geneSize;


    while(present == true && tries < 200) {
        present = false;

        randomNewGene = rand() % numOfPackages;

        for(unsigned int swapIn = 0; swapIn < geneSize; ++swapIn) {
            if (packages[randomNewGene] == gene[swapIn]) {
                present = true;
                break;
            }
        }
        ++tries;
    }

    if (tries < 200) {
        gene[randomPoint] = packages[randomNewGene];
    }

    newGene = gene;
    return newGene;
}

void Genetic::mergeLists(unsigned long i, unsigned long m, unsigned long j) {
    typedef pair<vector<Package* >, float> geneFit;
    vector< geneFit > geneBank;
    geneBank.resize(genes.size());

    unsigned long p = i;
    unsigned long q = m + 1;
    unsigned r = i;


    while(p <= m && q <= j) {


        if (genes[p].second <= genes[q].second) {
            geneBank[r] = genes[p];
            p++;
        } else {
            geneBank[r] = genes[q];
            q++;
        }
        r++;
    }

    while(p <= m) {
        geneBank[r] = genes[p];
        p++;
        r++;
    }

    while (q <= j) {
        geneBank[r] = genes[q];
        q++;
        r++;
    }

    for (r = i; r <= j; r++) {
        genes[r] = geneBank[r];
    }

    return;
}

void Genetic::mergeSort(unsigned long i, unsigned long j) {
    if (i == j) {
        return;
    }


    unsigned long m = (i + j) / 2;

    mergeSort(i, m);

    mergeSort(m + 1, j);

    mergeLists(i, m, j);

    return;
}
