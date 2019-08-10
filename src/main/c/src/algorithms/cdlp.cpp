/*
 * CDLP algorithm implementation in GraphBLAS.
 */

#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>

#include "utils.h"
#include "graphio.h"
#include "computation_timer.hpp"

/*
 * Result serializer function
 */
void WriteOutCDLPResult(
    GrB_Vector result,
    const std::vector<GrB_Index> &mapping,
    const BenchmarkParameters &parameters
) {

    std::ofstream file{parameters.output_file};
    if (!file.is_open()) {
        std::cerr << "File " << parameters.output_file << " does not exists" << std::endl;
        exit(-1);
    }
    file.precision(16);
    file << std::scientific;

    uint64_t value;
    for (GrB_Index res_index = 0; res_index < mapping.size(); res_index++) {
        GrB_Index original_index = mapping[res_index];
        GrB_Index label_in_matrix;
        GrB_Vector_extractElement_UINT64(&value, result, label_in_matrix);
        GrB_Index label_in_graph = mapping[label_in_matrix];

        file << original_index << " " << label_in_graph << std::endl;
    }
}

GrB_Vector LA_CDLP(GrB_Matrix A, bool symmetric, int itermax) {
    GrB_Info info;
    GrB_Vector l;

    {
        ComputationTimer timer{"CDLP"};
        double timing[2];
        OK(LAGraph_cdlp(&l, A, symmetric, itermax, timing))
    }

    return l;
}

int main(int argc, char **argv) {
    BenchmarkParameters parameters = ParseBenchmarkParameters(argc, argv);

    LAGraph_init();
    GxB_Global_Option_set(GxB_GLOBAL_NTHREADS, parameters.thread_num);

    GrB_Matrix A = ReadMatrixMarket(parameters);
    std::vector<GrB_Index> mapping = ReadMapping(parameters);

    std::cout << "Processing starts at: " << GetCurrentMilliseconds() << std::endl;
    GrB_Vector result = LA_CDLP(A, !parameters.directed, parameters.max_iteration);
    std::cout << "Processing ends at: " << GetCurrentMilliseconds() << std::endl;

    WriteOutCDLPResult(result, mapping, parameters);

    GrB_Matrix_free(&A);
    GrB_Vector_free(&result);
}
