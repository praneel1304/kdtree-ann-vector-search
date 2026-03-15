#include <iostream>
#include <vector>
#include<ctime>
#include<chrono>
#include "vector_db.hpp"
using namespace std;

vector<float> generateRandomVector(int dimen){
    vector<float>coords;
    for(int i= 0 ; i< dimen;i++){
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        coords.push_back(r);
    }
    return coords;
}
int main(){//pore algorithmic speedup;
    srand(42);
    VectorStore store;
    
    int DIMENSIONS = 128; 
    int NUM_POINTS = 100000; 

    cout << "Generating " << NUM_POINTS << " vectors (" << DIMENSIONS << "d)..." << endl;
    for(int i = 0; i < NUM_POINTS; i++) {
        store.save(VectorPoint(i, generateRandomVector(DIMENSIONS)));
    }

    auto startBuild = chrono::high_resolution_clock::now();
    store.rebuildIndex();
    auto endBuild = chrono::high_resolution_clock::now();
    cout << "Tree Build Time: " << chrono::duration_cast<chrono::milliseconds>(endBuild - startBuild).count() << "ms" << endl;

    VectorPoint target(999999, generateRandomVector(DIMENSIONS));

    
    cout << "Running Linear Search..." << endl;
    auto startLin = chrono::high_resolution_clock::now();
    VectorPoint resultLin = store.FindNearestLinear(target); 
    auto endLin = chrono::high_resolution_clock::now();

    cout << "Running KD-Tree Search..." << endl;
    auto startKD = chrono::high_resolution_clock::now();
    VectorPoint resultKD = store.FindNearestKD(target,1000);
    auto endKD = chrono::high_resolution_clock::now();

  
    cout << "\n---------------- RESULTS ----------------" << endl;
    long timeLin = chrono::duration_cast<chrono::microseconds>(endLin - startLin).count();
    long timeKD = chrono::duration_cast<chrono::microseconds>(endKD - startKD).count();

    cout << "Linear Search Time: " << timeLin << " microseconds" << endl;
    cout << "KD-Tree Search Time: " << timeKD << " microseconds" << endl;
    
    
    if (timeKD > 0) {
        cout << "Speedup: " << (float)timeLin / timeKD << "x FASTER" << endl;
    }

    cout << "-----------------------------------------" << endl;
    cout << "Linear Distance: " << target.distanceto(resultLin) << endl;
    cout << "KD-Tree Distance with max nodes = 1000: " << target.distanceto(resultKD) << endl;

    return 0;
}