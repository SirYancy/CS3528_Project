#include "Genetic.h"

// Seed nice random number generator.
std::mt19937 rngGenetic(std::random_device{}());

Genetic::Genetic(std::vector<Package*> packs,
                 std::vector<vector<unsigned int> > matrix,
                 unsigned int weight,
                 unsigned int packLimit,
                 unsigned int population,
                 float stops, float drive, float shiftTime, unsigned long gens, mutation_enum mut)
                 : packages(packs), adMatrix(matrix), weightLimit(weight), packageLimit(packLimit),
                 stopTime(stops), driveTime(drive), timeLimit(shiftTime), generations(gens), mutation(mut)

{
    Priority currentPriority;

    // Truncate population to multiples of four for threading.
    popNum = population;

    std::cout << "Population: " << popNum << " Generations: " << gens << std::endl;

    // Total for roulette selection of mutation.
    mutation.total = mutation.deleteOld + mutation.insertNew + mutation.inversion + mutation.swapOut + mutation.swapWithin;

    // Create the best fitness info vector for later indexing.
    for (unsigned int i = 0; i < 5; ++i) {
        bestFitInfo.push_back(0);
    }

    // How many packages are there in the pool.
    numOfPackages = packages.size();

    // How many good members will be carried over into the next population
    elitist = mutation.elite * popNum;

    for (vector<Package* >::iterator iter = packages.begin(); iter != packages.end(); ++iter) {
        currentPriority = (*iter)->getPriority();

        // Assign weight.
        if (currentPriority == Priority::OVERNIGHT) {
            totalPriority += OVERNIGHT_WEIGHT;
        } else if (currentPriority == Priority::TWO_DAY) {
            totalPriority += TWODAY_WEIGHT;
        } else {
            totalPriority += REGULAR_WEIGHT;
        }
    }
}

Genetic::~Genetic()
{
    //dtor
}

size_t Genetic::hash(vector<Package* >* gene) const {
    size_t seed = gene->size();
    for (auto iter = gene->begin(); iter != gene->end(); ++iter) {
        seed ^= (*iter)->getID() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

void Genetic::initPopulation() {
    // Package distribution
    std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

    // Individual we're creating randomly
    std::vector<Package* > individual;

    // Resize population to avoid push_backs
    genes.resize(popNum);

    // Gene info for current genome
    geneInfo currentGeneInfo;

    // Totals to keep track of for route, don't overload the truck.
    float totalWeight = 0;
    unsigned int totalPackages = 0;

    // Currently selected package
    Package* currentPackage;

    // Random package index
    unsigned int randomPackage;

    // Is package already present flag
    bool present = false;

    // Create population of individuals.
    for (unsigned int i = 0; i < popNum; i++) {

        // Clear individual
        individual.clear();

        // Don't overload the truck. Reset.
        totalWeight = 0;
        totalPackages = 0;

        // Loop while we still have packages to place, and we're under the limits.
        while (totalPackages < numOfPackages && totalPackages < packageLimit && totalWeight < weightLimit && (totalPackages * stopTime) < timeLimit / 2) {

            // Random index to packages
            randomPackage = packageUniform(rngGenetic);

            // Get the package pointer
            currentPackage = packages[randomPackage];

            // Assume not present in current individual
            present = false;

            // Loop over the individual looking for duplicate package
            for (std::vector<Package*>::iterator iter = individual.begin(); iter != individual.end(); ++iter) {
                // Check duplicate
                if (*iter == currentPackage) {
                    present = true;
                    break;
                }
            }

            // If not a duplicate
            if (present == false) {

                // Add package to individual
                individual.push_back(currentPackage);
                totalPackages++;
                totalWeight += currentPackage->getWeight();
            }
        }

        // Optimize individual.
        // Incredibly slow!
        //individual = twoOpt(&individual);

        // Get gene info
        currentGeneInfo.fitnessValue = fitness(&individual);
        currentGeneInfo.hashValue = hash(&individual);
        currentGeneInfo.sizeValue = individual.size();

        // Add to gene pool
        genes[i] = std::make_pair(individual, currentGeneInfo);

    }

    // Create ranking array for selection
    initRanking(2);

    twoOptPopulation();

}

void Genetic::initRanking(float exponent) {
    // Resize the ranking vector used in selection to population.
    ranking.resize(genes.size());
    rankingSize = ranking.size();

    // Load exponential ranking array. Always the same number of indices to individuals, so same ranking
    for (unsigned int i = 0; i < rankingSize; ++i) {
        // Exponential ranking
        ranking[i] = pow(i+1, exponent);
        // Add rank to total
        rankTotal += ranking[i];
    }
}

vector<double> Genetic::fitness(vector<Package* >* individual) {
    unsigned int distance = 0;
    unsigned int shiftTime = 0;
    unsigned int priorities = 0;
    Priority currentPriority;

    double weight = 0;

    unsigned int previousIndex = 0, currentIndex = 0;

    double indFit = 0;

    previousIndex = 0;

    for (std::vector<Package*>::iterator iter = (*individual).begin(); iter != (*individual).end(); ++iter) {
        currentIndex = (*iter)->getReceiver()->getID();

        distance += adMatrix[previousIndex][currentIndex];


        previousIndex = currentIndex;
        currentPriority = (*iter)->getPriority();

        // Assign weight.
        if (currentPriority == Priority::OVERNIGHT) {
            priorities += OVERNIGHT_WEIGHT;
        } else if (currentPriority == Priority::TWO_DAY) {
            priorities += TWODAY_WEIGHT;
        } else {
            priorities += REGULAR_WEIGHT;
        }

        weight += (*iter)->getWeight();

        shiftTime += stopTime;
    }

    distance += adMatrix[previousIndex][0];

    shiftTime += distance * driveTime;

    double generationRatio;

    if (currentGeneration <  (generations + 1) / 2) {
        generationRatio = static_cast<double>(currentGeneration + (generations + 1.0)/2.0)/static_cast<double>(generations);
    } else {
        generationRatio = 1.0;
    }

    indFit = pow(2.0, static_cast<double>(priorities) / static_cast<double>(totalPriority)) * pow(1.05, (static_cast<double>(individual->size() * stopTime) / static_cast<double>(shiftTime)));
    //std::cout << "Initial Indfit: " << indFit << " P: " << priorities << "/" << totalPriority << " T: " << shiftTime << " Generation ratio: " << generationRatio << std::endl;
    if (shiftTime > timeLimit) {
        indFit -= (pow(2.0, 1.5 + (static_cast<double>(shiftTime) - static_cast<double>(timeLimit)) / static_cast<double>(timeLimit)) * generationRatio);
        //std::cout << " Over time limit: " << indFit << " " << std::endl;
    } //else {
        //std::cout << "Under time limit" << std::endl;
        //if (indFit < 0) {
            //std::cout << "Negative fitness" << std::endl;
            //indFit /= pow(2.0, (static_cast<double>(timeLimit) - static_cast<double>(shiftTime)) / static_cast<double>(timeLimit));
       // } //else {
            //std::cout << "Positive fitness" << std::endl;
            //indFit *= ;
        //}
    //}

    if (weight > weightLimit) {
        indFit -= pow(2.0, 1.5 + (static_cast<double>(weight) - static_cast<double>(weightLimit)) / static_cast<double>(weightLimit)) * generationRatio;
    }

    if (individual->size() > packageLimit) {
        indFit -= pow(2.0, 1.5 + (static_cast<double>(individual->size()) - static_cast<double>(packageLimit)) / static_cast<double>(packageLimit)) * generationRatio;
    }


    indFit = pow(indFit, 3.0);
    //std::cout << "Indfit: " << indFit << std::endl;

    vector<double> fit {indFit, static_cast<double>(priorities), static_cast<double>(distance), static_cast<double>(shiftTime), static_cast<double>(weight)};
    return fit;
}

void Genetic::loadPopulation(vector< pair<vector<Package* >, geneInfo> > newPopulation) {

    genes = newPopulation;
    popNum = newPopulation.size();

    // Initialize current fitness.
    vector<float> currentFitness (5, 0);

    initRanking(1.5);

    // Recompute the fitness values
    currentGeneration = 1;

    for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
        //(*iter).first = twoOpt(&(*iter).first);
        (*iter).second.fitnessValue = fitness(&(*iter).first);
    }

    std::sort(genes.begin(), genes.end(), [] (pair<vector<Package* >, Genetic::geneInfo> const& left, pair<vector<Package* >, Genetic::geneInfo> const& right) {return left.second.fitnessValue < right.second.fitnessValue;});

    return;

}

vector< pair<vector<Package* >, Genetic::geneInfo> > Genetic::evolve_threads() {
    evolve();

    twoOptPopulation();

    return genes;

}

void Genetic::twoOptPopulation() {
    std::cout << "*** Optimizing population Generation " << currentGeneration << " ***" << std::endl;

    // Iterate over population and invert optimize the genomes.
    for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
        (*iter).first = twoOpt(&(*iter).first);
        (*iter).second.fitnessValue = fitness(&(*iter).first);
    }
}

void Genetic::printGene(vector<Package* >* gene) const {
       for (auto iter = gene->begin(); iter != gene->end(); ++iter) {
            std::cout << (*iter)->getID() << " ";
       }
}

vector<Package* > Genetic::evolve() {

    vector<double> currentFitness (5, 0);
    vector<double> currentBest (5, 0);

    vector<Package* > tempGene;
    vector<double> tempInfo;

    geneInfo currentInfo;

    // Sadly, the use of NaN and -Inf are not guaranteed, so we'll use a dumb flag.
    // Have we loaded a best fitness individual yet?
    bool bestFoundYet = false;

    std::vector<std::pair<std::vector<Package* >, geneInfo> >::iterator row;

    for (unsigned long i = 0; i < generations; ++i) {

        currentGeneration = i + 1;

        currentBest[0] = 0;

        // Optimize the population occasionally, but not the last generation, we do that on the way out.
        if (currentGeneration % (generations / 4) == 0 && currentGeneration != generations) {
            twoOptPopulation();
        }

        for (row = genes.begin(); row != genes.end(); ++row) {

            // Check if this is our best gene. Update fitness if so.
            if (row->second.hashValue == bestGeneInfo.hashValue) {
                //std::cout << "Found old best " << row->second.hashValue << " as: " << row->second.fitnessValue[0];
                row->second.fitnessValue = fitness(&(row->first));
                bestGeneInfo = row->second;
                //std::cout << " Updated new best " << bestGeneInfo.hashValue << " as: " << bestGeneInfo.fitnessValue[0] << std::endl;

            }

            // SLOW!
            //row->first = twoOpt(&(row->first));
            //row->second.fitnessValue = fitness(&(row->first));
            //row->second.hashValue = hash(&(row->first));

            if (row->second.fitnessValue[0] > currentBest[0] || currentBest[0] == 0) {
                currentBest = row->second.fitnessValue;
            }

            // Found a better individual, or is this the first individual?
            if (bestFoundYet == false || row->second.fitnessValue[0] > bestGeneInfo.fitnessValue[0]) {
                bestFit = (*row).first;
                bestFitInfo = row->second.fitnessValue;
                bestGeneInfo = row->second;
                bestFoundYet = true;

            }
        }

        if (i % 500 == 0) {
            std::cout << "Generation " << i << " Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << "/" << numOfPackages << " CO: " << avgIndividual << std::endl;//" F: " << genes[popNum - elitist - 2].second << " P: " << currentBest[1] << " D: " << currentBest[2] << " T: " << currentBest[3] << "/" << timeLimit << " W: " << currentBest[4] << "/" << weightLimit << std::endl;

            //tempGene = twoOpt(&bestFit);
            //tempInfo = fitness(&tempGene);
            //std::cout << "         2-opt Best F: " << tempInfo[0] << " P: " << tempInfo[1] << "/" << totalPriority << " D: " << tempInfo[2] << " T: " << tempInfo[3] << "/" << timeLimit << " W: " << tempInfo[4] << "/" << weightLimit << " L: " << tempInfo.size() << std::endl;
        }
        mate();
    }
    std::cout << "Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;

    bestFit = twoOpt(&bestFit);
    bestFitInfo = fitness(&bestFit);

    //std::cout << "Generation: " << currentGeneration << std::endl;
    std::cout << "2-opt Best F: " << bestFitInfo[0] << " P: " << bestFitInfo[1] << "/" << totalPriority << " D: " << bestFitInfo[2] << " T: " << bestFitInfo[3] << "/" << timeLimit << " W: " << bestFitInfo[4] << "/" << weightLimit << " L: " << bestFit.size() << std::endl;

    return getBest();
}

pair<int, int> Genetic::chooseParents() {

    vector<vector<Package* > > newIndividuals;
    newIndividuals.resize(2);

    std::uniform_real_distribution<double> rankUniform(0, rankTotal);

    int randomIndividual1 = 0;
    int randomIndividual2 = 0;
    double rank1 = 0;
    double rank2 = 0;

    bool different = false;


    // Loop until we get different individuals
    while (different == false) {
        randomIndividual1 = 0;
        randomIndividual2 = 0;

        rank1 = rankUniform(rngGenetic);
        rank2 = rankUniform(rngGenetic);


        for (unsigned int i = 0; i < rankingSize; ++i) {
            if (rank1 < ranking[i]) {
                randomIndividual1 = i;
                avgIndividual = avgIndividual * 100 + static_cast<float>(i);
                // Done searching.
                break;
            } else {
                rank1 -= ranking[i];
            }

        }

        for (unsigned int i = 0; i < rankingSize; ++i) {
            if (rank2 < ranking[i]) {
                randomIndividual2 = i;
                avgIndividual = (avgIndividual + static_cast<float>(i)) / 102;

                // Done searching.
                break;
            } else {
                rank2 -= ranking[i];
            }

        }

        different = false;

        if (genes[randomIndividual1].second.hashValue != genes[randomIndividual2].second.hashValue) {

            different = true;

        }

    }

    return make_pair(randomIndividual1, randomIndividual2);

}

vector<Package* > Genetic::twoOpt(vector<Package* >* gene) {
    // Size of gene
    unsigned int geneSize = gene->size();
    // Copies of gene for working on.
    vector<Package* > previousBestGene = *gene, workingGene = *gene;

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        vector<double> currentFitnessGene, previousFitness = fitness(&workingGene);

        bool improvement = true;
        int inversionSize;

        //std::cout << "Incoming distance: " << previousFitness[2] << std::endl;

        while (improvement == true) {

            // Assume we will have no improvement
            improvement = false;

            // Loop through gene looking for improvement. Front index of inversion
            for (unsigned int frontIndex = 0; frontIndex < geneSize - 2; ++frontIndex) {

                // Inner loop for end-point/back of inversion
                for (unsigned int backIndex = frontIndex + 1; backIndex < geneSize - 1; ++backIndex) {

                    inversionSize = backIndex - frontIndex;

                    // Perform the inversion
                    for (int index = 0; index < inversionSize / 2; ++index) {
                        // Swap genes
                        std::swap(workingGene.at(frontIndex + index), workingGene.at(backIndex - index));
                    }

                    // Check for improvement
                    currentFitnessGene = fitness(&workingGene);

                    // Check if distance improved
                    if (currentFitnessGene[2] < previousFitness[2]) {
                        //std::cout << "New distance: " << currentFitnessGene[2] << std::endl;

                        // Had an improvement, flag it
                        improvement = true;

                        // Save best score.
                        previousFitness = currentFitnessGene;

                        // Save best gene.
                        previousBestGene = workingGene;

                        // Break out of inside loop.
                        break;
                    } else {
                        // No improvement, restore previous best gene.
                        workingGene = previousBestGene;
                    }
                }


                if (improvement == true) {
                    // Break out of outer loop.
                    break;
                }
            }
        }
    }

    return previousBestGene;
}


void Genetic::mate() {

    vector< pair<vector<Package* >, geneInfo> > newPopulation;
    vector< vector<Package* > > newIndividuals;
    pair<int, int> choosenParents;
    vector<Package* >* choosenGene;
    geneInfo currentGeneInfo1, currentGeneInfo2;

    // Resize to correct size to increase speed.
    newPopulation.resize(popNum);

    // Sort "in-place" based on fitness value. Least fit routes first in the vector, most fit last.
    std::sort(genes.begin(), genes.end(), [] (pair<vector<Package* >, Genetic::geneInfo> const& left, pair<vector<Package* >, Genetic::geneInfo> const& right) {return left.second.fitnessValue < right.second.fitnessValue;});

    bool different = false;

    for (int i = 0; i < static_cast<int>(mutation.crossOver * popNum) / 2; ++i) {
        different = false;

        while (different == false) {
            choosenParents.first = 0;
            choosenParents.second = 0;

            while (choosenParents.first == choosenParents.second) {
                choosenParents = chooseParents();
            }

            newIndividuals = crossOver(&genes.at(choosenParents.first).first, &genes.at(choosenParents.second).first);

            currentGeneInfo1.sizeValue = newIndividuals[0].size();
            currentGeneInfo2.sizeValue = newIndividuals[1].size();

            // Incredibly slow!
            //newIndividuals[0] = twoOpt(&newIndividuals[0]);
            //newIndividuals[1] = twoOpt(&newIndividuals[1]);
            currentGeneInfo1.hashValue = hash(&newIndividuals[0]);
            currentGeneInfo2.hashValue = hash(&newIndividuals[1]);

            if (currentGeneInfo1.hashValue != currentGeneInfo2.hashValue) {
                different = true;
                currentGeneInfo1.fitnessValue = fitness(&newIndividuals[0]);
                currentGeneInfo2.fitnessValue = fitness(&newIndividuals[1]);
            }

            /*
            if (different == false) {
                std::cout << "Crossover identical" << std::endl;
                printGene(&newIndividuals[0]);
                std::cout << std::endl;
                printGene(&newIndividuals[1]);
                std::cout << std::endl;
            }
            */
        }

        newPopulation[2 * i] = make_pair(newIndividuals[0], currentGeneInfo1);
        newPopulation[2 * i + 1] = make_pair(newIndividuals[1], currentGeneInfo2);
    }

    for (int i = static_cast<int>(mutation.crossOver * popNum) / 2; i < static_cast<int>(popNum - elitist); ++i) {
        newPopulation[i] = genes[i];
    }

    // Create a new population not including save elite individuals.
    for (int i = 0; i < static_cast<int>(popNum - elitist); ++i) {

        choosenGene = &newPopulation[i].first;

        different = false;

        while (different == false) {
            mutate(choosenGene);

            currentGeneInfo1.hashValue = hash(choosenGene);

            different = true;

            for (auto iter = newPopulation.begin(); iter != newPopulation.end(); ++iter) {
                if (currentGeneInfo1.hashValue == (*iter).second.hashValue) {
                    different = false;
                    break;
                }
            }

            if (different == true) {
                for (auto iter = genes.begin(); iter != genes.end(); ++iter) {
                    if (currentGeneInfo1.hashValue == (*iter).second.hashValue) {
                        different = false;
                        break;
                    }
                }
            }
        }

        currentGeneInfo1.sizeValue = choosenGene->size();
        currentGeneInfo1.fitnessValue = fitness(choosenGene);

        newPopulation[i] = make_pair(*choosenGene, currentGeneInfo1);
    }

    // Save the elite few, the fitest. Save the Queen!
    for (unsigned int i = (popNum - elitist - 1); i < popNum; ++i) {
            newPopulation[i] = genes[i];
    }


    genes = newPopulation;
}

vector<vector<Package*> > Genetic::crossOver(vector<Package* >* gene1, vector<Package* >* gene2) {

    // Do we have genes of size 1?
    if (gene1->size() <= 1 || gene2->size() <= 1) {
        return vector<vector<Package* > > {*gene1, *gene2};
    }

    // Size of genes.
    unsigned int geneLength1 = gene1->size();
    unsigned int geneLength2 = gene2->size();

    // Maps to hold what genes are within each vector for excluding duplicate genes.
    std::unordered_map<Package*,bool> gene1Map;
    std::unordered_map<Package*,bool> gene2Map;


    std::unordered_map<Package*,bool>::const_iterator gene1MapIter;
    std::unordered_map<Package*,bool>::const_iterator gene2MapIter;

    std::vector<Package* >::iterator gene1End = gene1->end();
    std::vector<Package* >::iterator gene2End = gene2->end();

    // Lengths of genes
    unsigned int smallestLength, longestLength;

    // Random crossover point.
    unsigned int randomPoint;

    // New genes for the individuals
    vector<Package* > packageGene1;
    vector<Package* > packageGene2;

    // Holders return individuals.
    vector<vector<Package* > > newIndividuals;

    // Resize to save on reallocation
    newIndividuals.resize(2);


    // Which gene is longer? Could be simplified.
    if (geneLength1 < geneLength2) {
        smallestLength = geneLength1;

        longestLength = geneLength2;

    } else if (geneLength2 < geneLength1) {
        smallestLength = geneLength2;

        longestLength = geneLength1;

    } else {
        smallestLength = geneLength1;
        longestLength = geneLength1;

    }

    // Size the hash table appropriately for the max number of elements.
    gene1Map.reserve(longestLength);
    gene2Map.reserve(longestLength);

    std::uniform_int_distribution<int> pointUniform(1, smallestLength - 1);

    // Random point to apply crossover. Don't want to cross over at 0
    randomPoint = pointUniform(rngGenetic); //(rand() % (smallestLength - 1)) + 1;

    // Resize to eliminate push-backs during initial crossover
    packageGene1.resize(randomPoint + 1);
    packageGene2.resize(randomPoint + 1);

    // Genes crosses over from 0 to randomPoint in genes
    for (unsigned int i = 0; i <= randomPoint; ++i) {
        packageGene1[i] = (*gene2)[i];
        gene1Map.insert({packageGene1[i], true});
        packageGene2[i] = (*gene1)[i];
        gene2Map.insert({packageGene2[i], true});
    }


    // Fill map
    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene1_iter = gene1->begin(); gene1_iter != gene1End; ++gene1_iter) {

        gene1MapIter = gene1Map.find(*gene1_iter);

        if (gene1MapIter == gene1Map.end()) {
            packageGene1.push_back(*gene1_iter);
            gene1Map.insert({*gene1_iter, true});
        }
    }

    // Fill individual[0] genes based on the size of gene1 in the order they appear
    for (vector<Package* >::iterator gene2_iter = gene2->begin(); gene2_iter != gene2End; ++gene2_iter) {

        gene2MapIter = gene2Map.find(*gene2_iter);

        if (gene2MapIter == gene2Map.end()) {
            packageGene2.push_back(*gene2_iter);
            gene2Map.insert({*gene2_iter, true});
        }

    }

    newIndividuals[0] = packageGene1;
    newIndividuals[1] = packageGene2;

    return newIndividuals;
}

void Genetic::mutate(vector<Package* >* choosen) {

    std::uniform_real_distribution<double> rankUniform(0, rankTotal);
    std::uniform_real_distribution<double> mutationUniform(0, 1);


    float randomP;
    bool maxCap = false;

    if (choosen->size() == numOfPackages) {
        maxCap = true;
    }

    for (unsigned int i = 0; i < choosen->size(); ++i) {
        randomP = mutationUniform(rngGenetic);

        if (randomP < mutation.deleteOld) {

            mutateDelete(choosen, i);
            return;
        }

        randomP -= mutation.deleteOld;

        if (randomP < mutation.insertNew) {

            mutateInsert(choosen, i);
            return;
        }

        randomP -= mutation.insertNew;

        if (randomP < mutation.inversion) {

            mutateInversion(choosen, i);
            return;
        }

        randomP -= mutation.inversion;

        if (randomP < mutation.swapOut) {
            mutateSwapNew(choosen, i);
            return;
        }

        randomP -= mutation.swapOut;

        // Swap genes within each parent.
        if (randomP < mutation.swapWithin) {
            mutateSwapWithin(choosen, i);
        }

        // Escaped with no mutation.

    }
    return;

}

void Genetic::mutateInsert(vector<Package *>* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Do we have any packages to insert?
    if (geneSize < numOfPackages) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);
        std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

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
            randomNewGene = packageUniform(rngGenetic); //rand() % numOfPackages;

            // Check gene for duplicates
            for(unsigned int swapIn = 0; swapIn < geneSize; ++swapIn) {
                if (packages[randomNewGene] == (*gene)[swapIn]) {
                    // Duplicate
                    present = true;
                    break;
                }
            }

            tries++;
        }

        if (tries < 200) {
            // Jam it in
            gene->insert(gene->begin() + location, packages[randomNewGene]);
        }

    }
    return;


}

void Genetic::mutateInversion(vector<Package* >* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);

        unsigned int randomSwap = location;

        // Find differing indices
        while(randomSwap == location) {
            randomSwap = geneUniform(rngGenetic);
        }

        int inversionSize = randomSwap - static_cast<int>(location);

        if (inversionSize < 0) {
            inversionSize *= -1;

            for (int i = 0; i < inversionSize / 2; ++i) {
                // Swap genes
                std::swap(gene->at(randomSwap + i), gene->at(location - i));
            }
        } else {
             for (int i = 0; i < inversionSize / 2; ++i) {
                // Swap genes
                std::swap(gene->at(location + i), gene->at(randomSwap - i));
            }
        }
    }
    return;
}


void Genetic::mutateDelete(vector<Package* >* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Delete a gene.
    // Only delete if there is a spare gene to delete.
    if (geneSize >= 2) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);

        // Remove gene.
        gene->erase(gene->begin() + location);
    }

    return;
}


void Genetic::mutateSwapWithin(vector<Package *>* gene, unsigned int location) {
    // Size of gene
    unsigned int geneSize = gene->size();

    // Swap if we have genes to swap
    if (geneSize >= 2) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);

        //unsigned int randomSwap1 = 0;
        //unsigned int randomSwap2 = 0;

        unsigned int randomSwap = location;

        // Find differing indices
        while(randomSwap == location) {
            randomSwap = geneUniform(rngGenetic);
        }

        // Swap genes
        std::swap(gene->at(randomSwap), gene->at(location));

    }
    // Return gene
    return;
}


void Genetic::mutateSwapNew(vector<Package* >* gene, unsigned int location) {
    unsigned int geneSize = gene->size();

    if (geneSize < numOfPackages) {

        std::uniform_int_distribution<int> geneUniform(0, geneSize - 1);
        std::uniform_int_distribution<int> packageUniform(0, numOfPackages - 1);

        bool present = true;
        unsigned int tries = 0;
        unsigned int randomNewGene;

        while(present == true && tries < 200) {
            present = false;

            randomNewGene = packageUniform(rngGenetic);

            for(unsigned int swapIn = 0; swapIn < geneSize; ++swapIn) {
                if (packages[randomNewGene] == gene->at(swapIn)) {
                    present = true;
                    break;
                }
            }
            ++tries;
        }

        if (tries < 200) {
            gene->at(location) = packages[randomNewGene];
        }
    }

    return;

}

void Genetic::mergeLists(unsigned long i, unsigned long m, unsigned long j) {
    typedef pair<vector<Package* >, geneInfo> geneFit;
    vector< geneFit > geneBank;
    geneBank.resize(genes.size());

    unsigned long p = i;
    unsigned long q = m + 1;
    unsigned r = i;


    while(p <= m && q <= j) {


        if (genes[p].second.fitnessValue <= genes[q].second.fitnessValue) {
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
