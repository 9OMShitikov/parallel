#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <functional>
#include <thread>

struct Matrix {
    int width;
    int height;
    double * elements;
    Matrix():
            width(0),
            height(0),
            elements(nullptr){
    }
    Matrix(Matrix&& other)  noexcept {
        elements = other.elements;
        width = other.width;
        height = other.height;

        other.elements = nullptr;
        other.width = 0;
        other.height = 0;
    }

    Matrix& operator=( Matrix&& other) noexcept {
        // Guard self assignment
        if (this == &other)
            return *this;

        elements = other.elements;
        width = other.width;
        height = other.height;
        other.elements = nullptr;
        other.width = 0;
        other.height = 0;
        return *this;
    }

    Matrix(int height_, int width_):
            width(width_),
            height(height_),
            elements(new double [width * height]){
    }
    ~Matrix() {
        delete[] elements;
        elements = nullptr;
    }

    inline double & operator() (int y, int x) const {
       return elements[x + y * width];
    }

    Matrix T() const;
    void fill();
    void print() const;
};

void Matrix::fill() {
    for (int index = 0; index < width * height; ++index) {
        elements[index] = double (rand())/RAND_MAX;
    }
}

void Matrix::print() const {
    std::cout<<std::setprecision(4);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            std::cout<<elements[j + i * width]<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

Matrix Matrix::T() const {
    Matrix T (width, height);
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            T(i, j) = (*this)(j, i);
        }
    }
    return T;
}

void MatMulThreadRoutine(Matrix* A, Matrix* B_t, Matrix* C, int start_a_row, int finish_a_row) {
    for (int j = start_a_row; j < finish_a_row; ++j) {
        for  (int i = 0; i < B_t->height; ++i) {
            double result = 0;
            for (int k = 0; k < A->width; ++k) {
                result += (*A)(j, k) * (*B_t)(i, k);
            }
            (*C)(j, i) = result;
        }
    }
}

Matrix MatMulParallel(Matrix &A, Matrix &B, int n_threads) {
    if (n_threads <= 0) {
        throw std::invalid_argument("number of threads should be more than zero");
    }
    if (A.width != B.height) {
        throw std::invalid_argument("matrices sizes don't match");
    }

    // Transpose B matrix, so values which are need to compute
    // result sell would be stored sequentially
    Matrix B_t = B.T();

    Matrix *A_pt = &A, *B_pt = &B_t;

    if (A.height < B.height) {
        std::swap(A_pt, B_pt);
    }

    Matrix C(A_pt->height, B_pt->height);
    Matrix *C_pt = &C;
    int height = A_pt->height;
    auto* threads = new std::thread[n_threads];

    for (int i = 0; i < n_threads; ++i) {
        int start_row = i * (height/n_threads) + (i < height%n_threads ? i : height%n_threads);
        int finish_row = (i + 1) * (height/n_threads) + (i + 1 < height%n_threads ? i + 1: height%n_threads);
        threads[i] = std::thread(MatMulThreadRoutine, A_pt, B_pt, C_pt, start_row, finish_row);
    }

    for (int i = 0; i < n_threads; ++i) {
        threads[i].join();
    }
    delete[] threads;

    if (A.height < B.height) {
        return C.T();
    }
    return C;
}

Matrix MatMulNaive(Matrix &A, Matrix &B) {
    if (A.width != B.height) {
        throw std::invalid_argument("matrices sizes don't match");
    }
    Matrix C(A.height, B.width);
    for (int i = 0; i < A.height; ++i) {
        for (int j = 0; j < B.width; ++j) {
            double result = 0;
            for (int k = 0; k < A.width; ++k) {
                result += A(i, k) * B(k, j);
            }
            C(i, j) = result;
        }
    }
    return C;
}
