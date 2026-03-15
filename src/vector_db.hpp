#ifndef VECTOR_DB_HPP
#define VECTOR_DB_HPP
#include <iostream>
#include <vector>
#include<cmath>
#include <cstdlib>
#include<ctime>
#include<chrono>
#include<fstream>
#include<algorithm>
#include <future>
#include <thread>

using namespace std;

class VectorPoint{
    public:
    long id;
    int dimensions;
    vector<float> coords;
    VectorPoint() : id(-1), dimensions(0) {};
VectorPoint(long inputId, vector<float> inputCoords){
    id = inputId;
    dimensions = inputCoords.size(); 
    coords = inputCoords;
   
}

void printVector(){
    for(int i=0;i<coords.size();i++){
        cout<<'x'<<i<<": "<<coords[i]<<" ";
    }
}

float distanceto( VectorPoint& v2) {
    if( coords.size()!=v2.coords.size()){cout<<"Dimension Mismatch"<<endl; return -1;}
    float dist = 0 ;
    for(int i=0;i<coords.size();i++){
        dist += (coords[i]-v2.coords[i])*(coords[i]-v2.coords[i]);
    }
    return sqrt(dist);
}

};
struct KDNode{
VectorPoint val;
KDNode* left;   
KDNode* right;
KDNode(VectorPoint p) : val(p), left(nullptr), right(nullptr) {};
};
class VectorStore{
private:
    vector<VectorPoint>db;
    KDNode* root = nullptr;
void freeTree(KDNode* node){
    if(node==nullptr){return;}
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}
public:
~VectorStore() { freeTree(root); }
void save(const VectorPoint& vec) {
        db.push_back(vec);
}
void logALL(){
    for(auto v:db){
        v.printVector();
    }

}
void saveDB(string filename){
   
        if(db.empty()){return;}
        ofstream file(filename, ios::binary);
        int dim = db[0].dimensions;
        file.write((char*)&dim, sizeof(int));
        for(auto& v:db){
            file.write((char*)&v.id, sizeof(long));//ID
            file.write((char*)&v.coords[0], dim * sizeof(float));

        }file.close();
        cout<<"db saved to "<<filename<<endl;
    
}
void loadDB(string filename) {
        ifstream infile(filename, ios::binary);
        if (!infile.is_open()) {
            cout << "Error opening file!" << endl;
            return;
        }
        int dim;
        infile.read((char*)&dim, sizeof(int)); 
        if (!infile) return;//immediately check if the read fails
        db.clear(); 
     while (true) {
    long id;
    vector<float> tempCoords(dim); 
    infile.read((char*)&id, sizeof(long));
    if (!infile) break; 
    infile.read((char*)&tempCoords[0], dim * sizeof(float));
    if (!infile) break;
    db.push_back(VectorPoint(id, tempCoords));
}
        
        infile.close();
        cout << "Database loaded: " << db.size() << " vectors." << endl;
    }

VectorPoint FindNearestLinear(VectorPoint& v1){
    float minDist = INFINITY;
    VectorPoint* nearest = nullptr;
    for(auto& v:db){
        float dist = v1.distanceto(v);
        if(dist < minDist){
            minDist = dist;
            nearest = &v;
        }
    }
    if (nearest == nullptr) {
            cout << "Error: Database is empty!" << endl;
         
            return v1; 
        }
    return *nearest;}
KDNode* buildKdTree(vector<VectorPoint>& nodes, int start, int end, int depth) {
    if (start >= end) return nullptr;

    int axis = depth % nodes[0].dimensions;
    int mid = (start + end) / 2;

        nth_element( nodes.begin() + start, 
        nodes.begin() + mid, 
        nodes.begin() + end,           
    [axis](const VectorPoint& a, const VectorPoint& b) {
        return a.coords[axis] < b.coords[axis];
    });
    KDNode* node = new KDNode(nodes[mid]);
    node->left = buildKdTree(nodes, start, mid, depth + 1);
    node->right = buildKdTree(nodes, mid + 1, end, depth + 1);
    return node;
}
void rebuildIndex() { 
if (db.empty()) return;
vector<VectorPoint> points = db; 
    root = buildKdTree(points, 0,points.size(),0);
    cout << "KD-Tree Index Rebuilt with " << db.size() << " nodes." << endl;
}
// adding multithreading
struct SearchState{
    VectorPoint* best = nullptr; float minDist = INFINITY;
    int vis = 0 ;
};


void searchTree(KDNode* node, VectorPoint& query, int depth,
    int max_nodes,SearchState &state){
        if(state.vis>=max_nodes&&max_nodes>-1){return;}
   if(node == nullptr){return;}
   float dist = query.distanceto(node->val);
   if(state.minDist>dist){
    state.minDist = dist;state.best = &node->val;
   }
   int axis = depth%query.dimensions;
   float diff = query.coords[axis] - node->val.coords[axis];
   KDNode* near= diff>0?node->right:node->left;
   KDNode* far = diff>0?node->left:node->right;
   state.vis+=1;
   searchTree(near,query,depth+1,max_nodes,state);
   if(abs(diff)<state.minDist){
    searchTree(far,query,depth+1,max_nodes,state);
   }
}
VectorPoint FindNearestKD(VectorPoint& query,int max_nodes =-1) {
        if (root == nullptr) {
            cout << "Error: KD-Tree is empty. Did you run rebuildIndex()?" << endl;
            return query; 
        }
        SearchState local ;
        searchTree(root, query, 0,max_nodes,local);
        if(!local.best){cout<<"Max nodes too low" ;
            return query;}
        return *(local.best);
    }
    vector<VectorPoint> FindNearestParellal(vector<VectorPoint>& queries, int max_nodes = -1){
        int n = queries.size();
        vector<VectorPoint> result(n);
        auto numThreads = thread::hardware_concurrency();
        int chunk = (n + numThreads - 1) / numThreads;  
        vector<future<void>> futures;
        for(unsigned int i = 0 ; i < numThreads;i++){
            int start  = i*chunk;
            int end  = min(start+chunk,n);
            if (start >= n) break;
            auto searchthread = [this,&queries,&result,start,end,max_nodes](){
                for(int j =start;j<end;j++ ){
                    result[j] = this->FindNearestKD(const_cast<VectorPoint&>(queries[j]), max_nodes);
            }
                };
                futures.push_back(async(launch::async,searchthread));
            }
            for (auto& f : futures) f.get();

    return result;
        
    }
};
#endif