#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 
#include "vector_db.hpp"

namespace py = pybind11;

PYBIND11_MODULE(vectordb, m) {
    m.doc() = "Fast Vector Database Plugin"; 

    py::class_<VectorPoint>(m, "VectorPoint")
        .def(py::init<long, vector<float>>())
        .def_readwrite("id", &VectorPoint::id)
        .def_readwrite("coords", &VectorPoint::coords)
        .def("printVector", &VectorPoint::printVector)
        .def("distanceto", &VectorPoint::distanceto);

   
    py::class_<VectorStore>(m, "VectorStore")
        .def(py::init<>())
        .def("save", &VectorStore::save)
        .def("rebuildIndex", &VectorStore::rebuildIndex)
        .def("loadDB", &VectorStore::loadDB)
        .def("findNearestLinear", &VectorStore::FindNearestLinear)
        
     
        .def("findNearestkd", &VectorStore::FindNearestKD, 
             py::arg("query"), 
             py::arg("max_nodes") = -1)
             .def("findNearestParallel", &VectorStore::FindNearestParellal, 
             py::call_guard<py::gil_scoped_release>(),
             py::arg("queries"), 
             py::arg("max_nodes") = -1);
}