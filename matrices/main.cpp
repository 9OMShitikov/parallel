#include <iostream>
#include <cassert>
#include "matrix.h"

//checks proximity of A and B matrices
void check_proximity(Matrix& A, Matrix& B) {
    if (A.width != B.width || A.height != B.height) {
        throw std::invalid_argument("matrices sizes don't match");
    }
    double eps = 0.01;
    for (int i = 0; i < A.height; ++i) {
        for (int j = 0; j < A.width; ++j) {
            // check if ratio |A[i, j] - B[i, j]|/(|A[i, j]| + |B[i, j]|) is small (A[i, j] ~ B[i, j])
            if (A(i, j) != 0 || B(i, j) != 0) {
                assert(
                        std::abs(A(i, j) - B(i, j)) / (std::abs(A(i, j)) + std::abs(B(i, j))) < eps
                );
            }
        }
    }
}

// finds ratio of parallel computation time and naive computation time for different numbers of threads
void acceleration_test (Matrix& A, Matrix& B, int n_threads_max) {
    std::ofstream results;
    results.open ("results.csv");
    results << R"("Number of threads","time ratio")"<<std::endl;

    auto start = std::chrono::steady_clock::now();
    auto C_naive = MatMulNaive(A, B);
    auto end = std::chrono::steady_clock::now();
    double t_naive = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double t_parallel;
    for (int i = 1; i <= n_threads_max; ++i) {
        start = std::chrono::steady_clock::now();
        auto C_par = MatMulParallel(A, B, i);
        end = std::chrono::steady_clock::now();
        t_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        check_proximity(C_naive, C_par);
        results << i <<","<<t_naive/t_parallel<<std::endl;
    }
    results.close();
}

int main() {
    int matrix_size = 256;
    int n_threads_max = 14;
    Matrix A(matrix_size, matrix_size);
    Matrix B(matrix_size, matrix_size);
    A.fill();
    B.fill();

    // testing algorithm for correctness
    auto C_par = MatMulParallel(A, B, n_threads_max);
    auto C_naive = MatMulNaive(A, B);
    check_proximity(C_par, C_naive);
    std::cout<<"Results are almost same"<<std::endl;
    acceleration_test(A, B, n_threads_max);
    return 0;
}
