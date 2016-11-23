#include "Genetic.h"
//#include "Client.h"
//#include "Package.h"
#include <vector>
#include <iostream>
#include <cmath>

Genetic::Genetic(std::vector<Package*> packs,
                 std::vector<vector<unsigned int> > matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive, float shiftTime, float mutateIn, float mutateSw, float mutateLen)
                 : packages(packs), adMatrix(matrix), weightLimit(weight), packageLimit(packLimit),
                 popNum(population), stopTime(stops), driveTime(drive), timeLimit(shiftTime)

{
    mutation[0] = mutateIn;
    mutation[1] = mutateSw;
    mutation[2] = mutateLen;
    mutationTotal = 1;
    mutation[3] = 1 - (mutation[0] + mutation[1] + mutation[2]);

    for (unsigned int i = 0; i < 5; ++i) {
        bestFitInfo.push_back(0);
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

    unsigned int numOfPackages = packages.size();
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
        while (totalPackages < numOfPackages && totalPackages < packageLimit && totalWeight < weightLimit && tries < maxTries) {

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
}

vector<float> Genetic::fitness(vector<Package*> individual) {
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
        indFit -= pow(shiftTime - timeLimit, 3);
    } else {
        indFit += pow(static_cast<float>(shiftTime) / static_cast<float>(timeLimit), 3);
    }

    //indFit += pow((static_cast<float>(OVERNIGHT) * static_cast<float>(individual.size()))/static_cast<float>(priorities), 1.5);
    indFit += pow(static_cast<float>(priorities), 2);
    if (weight > weightLimit) {
        indFit -= pow(weight - weightLimit, 1.5);
    }
    //indFit = pow(timeLimit/static_cast<float>(shiftTime),2) + (static_cast<float>(OVERNIGHT) * static_cast<float>(individual.size()))/static_cast<float>(priorities);// + pow(static_cast<float>(weightLimit - weight), 2);

    //std::cout.precision(17);
    //std::cout << 1/static_cast<float>(shiftTime) << std::endl;
    //std::cout << "Fit: " << indFit << std::endl;
    //std::cout << "OVERNIGHT: " << static_cast<int>(OVERNIGHT) << std::endl;
    vector<float> fit {indFit, priorities, distance, shiftTime, weight};
    return fit;
}

vector<Package*> Genetic::evolve() {

    initPopulation();
    vector<float> currentFitness (5, 0);
    vector<float> currentBest (5, 0);

    std::vector<std::pair<std::vector<Package*>, float> >::iterator row;

    for (unsigned int i = 0; i < 10000; ++i) {
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

        if (i % 100 == 0) {
            std::cout << "Generation " << i << " F: " << currentBest[0] << " P: " << currentBest[1] << " D: " << currentBest[2] << " T: " << currentBest[3] << "/" << timeLimit << " W: " << currentBest[4] << "/" << weightLimit << " Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;
        }
        mate();
    }
    std::cout << "Best route F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;
    /*
    std::cout << "Unsorted fitness: ";
    unsigned int i = 0;
    for (row = genes.begin(); row != genes.end(); ++row) {
        std::cout << (*row).second << " ";
        i++;
    }

    std::cout << std::endl << "Unsorted items " << i << std::endl << "Sorted fitness: ";
    mergeSort(0, genes.size() - 1);

    i = 0;
    for (row = genes.begin(); row != genes.end(); ++row) {
        std::cout << (*row).second << " ";
        i++;
    }

    std::cout << "Sorted items " << i << std::endl;
    */

    return bestFit;
}

void Genetic::mate() {
    vector<unsigned int> ranking (genes.size(), 0);
    unsigned int rankTotal = 0;
    vector< pair<vector<Package* >, float> > newGenes;
    vector<Package* > newIndividual;

    int randomIndividual1 = 0;
    int randomIndividual2 = 0;
    unsigned int selectionTotal = 0;

    // Sort based on fitness value. Least fit routes first, most fit last.
    mergeSort(0, genes.size() - 1);

    // Load exponential ranking array.
    for (unsigned int i = 0; i < ranking.size(); ++i) {
        // Exponential ranking
        ranking[i] = pow(i+1, 2);

        // Add rank to total
        rankTotal += ranking[i];
    }

    for (unsigned int i = 0; i < popNum; ++i) {
            randomIndividual1 = 0;
            randomIndividual2 = 0;

        // Loop until we get different individuals
        while (randomIndividual1 == randomIndividual2) {
            randomIndividual1 = rand() % rankTotal;
            randomIndividual2 = rand() % rankTotal;
        }

        for (unsigned int i = 0; i < ranking.size(); ++i) {
            //std::cout << "In1: " << randomIndividual1 << "/" << ranking[i] << " " << i << std::endl;
            if (randomIndividual1 < ranking[i]) {
                randomIndividual1 = i;
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
                //std::cout << "Individual 2: " << randomIndividual2 << std::endl;

                // Done searching.
                break;
            } else {
                randomIndividual2 -= ranking[i];
            }

        }

        //std::cout << genes.size() << std::endl;
        //std::cout << genes[0].second;

        newIndividual = crossOver(genes[randomIndividual1].first, genes[randomIndividual2].first);
        newGenes.push_back(make_pair(newIndividual, 0));
    }

    genes = newGenes;
}

vector<Package* > Genetic::crossOver(vector<Package* > gene1, vector<Package* > gene2) {
    vector<Package* > newIndividual;

    unsigned int geneLength1 = gene1.size();
    unsigned int geneLength2 = gene2.size();
    unsigned int smallestLength;
    unsigned int longestLength;
    unsigned int randomPoint;
    unsigned int whichFirst;
    unsigned int randomNewGene;
    unsigned int tries = 0;

    float randomP;
    unsigned int randomSwap1, randomSwap2;
    bool present = false;


    if (geneLength1 < geneLength2) {
        smallestLength = geneLength1;
        longestLength = geneLength2;
        whichFirst = 0;
    } else if (geneLength2 < geneLength1) {
        smallestLength = geneLength2;
        longestLength = geneLength1;
        whichFirst = 1;
    } else {
        smallestLength = geneLength1;
        longestLength = geneLength1;
        whichFirst = rand() % 2;
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

    std::cout << std::endl << "Random point: " << randomPoint << std::endl;
    */
    // Gene1 crosses over from 0 to randomPoint it's genes
    if (whichFirst == 0) {
        //std::cout << "Gene1 first" << std::endl;
        for (unsigned int i = 0; i < randomPoint; ++i) {
            newIndividual.push_back(gene1[i]);
        }

        for (unsigned int i = 0; i < longestLength; ++i) {
            present = false;
            for (unsigned int j = 0; j < i; ++j) {
                if (newIndividual[j] == gene2[i]) {
                    //std::cout << "Gene already present" << std::endl;
                    present = true;
                    break;
                }
            }

            if (present == false) {
                newIndividual.push_back(gene2[i]);
            }
        }
    } else {
        //std::cout << "Gene2 first" << std::endl;
        // Cross over gene2 first, then gene1
        for (unsigned int i = 0; i < randomPoint; ++i) {
            newIndividual.push_back(gene2[i]);
        }

        for (unsigned int i = 0; i < longestLength; ++i) {
            present = false;
            for (unsigned int j = 0; j < i; ++j) {
                if (newIndividual[j] == gene1[i]) {
                    //std::cout << "Gene already present" << std::endl;
                    present = true;
                    break;
                }
            }

            if (present == false) {
                newIndividual.push_back(gene1[i]);
            }
        }
    }

    unsigned int newSize = newIndividual.size();
    for(std::vector<Package*>::iterator iter = newIndividual.begin(); iter != newIndividual.end(); ++iter) {
        randomP = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * mutationTotal;
        //std::cout << randomP << std::endl;
        // Check to swap a package with one not in this individual.
        if (randomP < mutation[0]) {
            //std::cout << "Swapping with packages..." << std::endl;
            present = true;
            tries = 0;
            while(present == true && tries < 200) {
                present = false;

                randomNewGene = rand() % packages.size();

                for(unsigned int swapIn = 0; swapIn < newIndividual.size(); ++swapIn) {
                    if (packages[randomNewGene] == newIndividual[swapIn]) {
                        present = true;
                        break;
                    }
                }
                tries++;
            }

            if (present == false) {
                (*iter) = packages[randomNewGene];
            }

            break;
        }

        randomP -= mutation[0];

        //std::cout << "Size: " << newSize << std::endl;
        // Check to swap genes around
        if (randomP < mutation[1]) {

            //std::cout << "Swapping genes..." << std::endl;
            randomSwap1 = 0;
            randomSwap2 = 0;
            while(randomSwap1 == randomSwap2) {
                randomSwap1 = rand() % (newSize);
                randomSwap2 = rand() % (newSize);
            }

            Package* temp = newIndividual[randomSwap1];
            newIndividual[randomSwap1] = newIndividual[randomSwap2];

            newIndividual[randomSwap2] = temp;
            break;
        }

        randomP -= mutation[1];

        // Time to randomly insert a gene.
        if (randomP < mutation[2]) {
            //std::cout << "Inserting gene..." << std::endl;

            if (rand() % 2 == 0) {
                present = true;
                tries = 0;
                while(present == true && tries < 200) {
                    present = false;

                    randomNewGene = rand() % packages.size();

                    for(unsigned int swapIn = 0; swapIn < newIndividual.size(); ++swapIn) {
                        if (packages[randomNewGene] == newIndividual[swapIn]) {
                            present = true;
                            break;
                        }
                    }
                    tries++;
                }

                randomPoint = rand() % newSize;
                if (present == false) {
                     newIndividual.insert(newIndividual.begin() + randomPoint, packages[randomNewGene]);
                }

                break;
            } else {
                // Delete a gene.
                //std::cout << "Deleting gene" << std::endl;
                // Only delete if there is a gene to delete.
                if (newIndividual.size() > 1) {
                    // Random point
                    randomPoint = rand() % newIndividual.size();
                    // Remove gene.
                    newIndividual.erase(newIndividual.begin() + randomPoint);
                }
                break;
            }
        }
    }

    /*
    std::cout << std::endl << "New:" << std::endl;
    for(std::vector<Package*>::iterator iter = newIndividual.begin(); iter != newIndividual.end(); ++iter) {
        std::cout << (*iter) << std::endl;
    }
*/
    return newIndividual;
}

void Genetic::mergeLists(unsigned long i, unsigned long m, unsigned long j) {
    typedef pair<vector<Package* >, float> geneFit;
    vector< geneFit > geneBank = genes;

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
