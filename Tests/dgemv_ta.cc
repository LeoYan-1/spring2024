#include <blas/dgemv_ta.hh>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <complex>

using namespace CDC8600;

extern "C" int32_t dgemv_(char *trans, int32_t *m, int32_t *n, double *alpha, double *A, int32_t *LDA, double *x, int32_t *incx, double *beta, double *y, int32_t *incy);

const int N = 20;

void test_dgemv_ta(int count, int traceon, i32 m, i32 n, i32 LDA, f64 alpha, f64 beta, i32 incx, i32 incy)
{
    
    reset();

    tracing = traceon;
    
    i32 nx = m*abs(incx); if (0 == nx) nx = 1;
    i32 ny = n*abs(incy); if (0 == ny) ny = 1;

    f64 *A = (f64*)CDC8600::memalloc(LDA*n);
    f64 *x = (f64*)CDC8600::memalloc(nx);
    f64 *y = (f64*)CDC8600::memalloc(ny);
    f64 *Y = new f64[ny];

    for (int i = 0; i < LDA*n; i++) { A[i] = f64(drand48()); }
    for (int i = 0; i < nx; i++) { x[i] = f64(drand48()); }
    for (int i = 0; i < ny; i++) { y[i] = f64(drand48()); }
    for (int i = 0; i < ny; i++) { Y[i] = y[i]; }

    char trans = 'T';

    dgemv_(&trans, &m, &n, &alpha, A, &LDA, x, &incx, &beta, Y, &incy);         // Reference implementation of DAXPY
    CDC8600::BLAS::dgemv_ta(m, n, alpha, A, LDA, x, incx, beta, y, incy);       // Implementation of DAXPY for the CDC8600
    
    const f64 epsilon = 1e-9;
    bool pass = true;
    for (int i = 0; i < ny; i++)
    {
        
        if (abs(Y[i] - y[i]) > 
        ((abs(Y[i]) < abs(y[i]) ? abs(Y[i]) : abs(y[i])) + epsilon) * epsilon)
        {
            pass = false;
        }
    }

    delete [] Y;

    cout << "dgemv_ta [" << setw(2) << count << "] ";
    cout << "(m = " << setw(3) << m;
    cout << ", n = " << setw(3) << n;
    //cout << ", alpha = " << setw(2) << alpha;
    //cout << ", LDA = " << setw(2) << LDA;
    //cout << ", incx = " << setw(2) << incx;
    //cout << ", beta = " << setw(2) << beta;
    //cout << ", incy = " << setw(2) << incy;
    cout << ", # of instr = ";
    for (u32 p = 0; p < params::Proc::N; p++) cout << setw(9) << PROC[p].instr_count;
    cout << ", # of cycles = ";
    for (u32 p = 0; p < params::Proc::N; p++) cout << setw(9) << PROC[p].op_maxcycle;
    cout << ") : ";

    if (pass)
        cout << "PASS" << std::endl;
    else
        cout << "FAIL" << std::endl;

    if (tracing) {
      dump(PROC[0].trace, "dgemv_ta.tr.0");
      dump(PROC[1].trace, "dgemv_ta.tr.1");
      dump(PROC[2].trace, "dgemv_ta.tr.2");
      dump(PROC[3].trace, "dgemv_ta.tr.3");
    }
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
	for (u32 i = 0; i < N; i++)
	{
            i32 m = rand() % 256;
            i32 n = rand() % 256;
            i32 LDA = m + rand() % 256; if (0 == LDA) LDA = 1;
            f64 alpha = f64(drand48());            
            f64 beta = f64(drand48());           
            i32 incx = (rand() % 16) - 8; if (incx == 0) incx = 1;
            i32 incy = (rand() % 16) - 8; if (incy == 0) incy = 1;
                        
	    test_dgemv_ta(i, false, m, n, LDA, alpha, beta, incx, incy);
	}
    }
    else if (argc == 8)
    {
        i32 m = atoi(argv[1]);
	i32 n = atoi(argv[2]);
        i32 LDA = atoi(argv[3]);
	f64 alpha = atof(argv[4]);
	f64 beta = atof(argv[5]);
	i32 incx = atoi(argv[6]);
	i32 incy = atoi(argv[7]);
        if (LDA < m) {
            cerr << "LDA must be larger than m" << endl;
            exit(1);
        }
        test_dgemv_ta(0, true, m, n, LDA, alpha, beta, incx, incy);
    }
    else
    {
	cerr << "Usage : " << argv[0] << " [m n LDA alpha beta incx incy]" << endl;
	exit(1);
    }
}
