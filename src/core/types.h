#pragma once

#include <complex>
#include <fftw3.h>


#ifdef USE_DOUBLE
    using Real = double;
    using Complex = fftw_complex; // double[2]
    
    #define FFTW_PLAN                fftw_plan
    #define FFTW_MALLOC              fftw_malloc
    #define FFTW_ALLOC_R             fftw_alloc_real
    #define FFTW_ALLOC_C             fftw_alloc_complex
    #define FFTW_FREE                fftw_free

    #define FFTW_PLAN_DFT_2D         fftw_plan_dft_2d
    #define FFTW_PLAN_DFT_R2C_2D     fftw_plan_dft_r2c_2d
    #define FFTW_PLAN_DFT_C2R_2D     fftw_plan_dft_c2r_2d

    #define FFTW_PLAN_DFT_3D         fftw_plan_dft_3d
    #define FFTW_PLAN_DFT_R2C_3D     fftw_plan_dft_r2c_3d
    #define FFTW_PLAN_DFT_C2R_3D     fftw_plan_dft_c2r_3d

    #define FFTW_EXECUTE_DFT         fftw_execute_dft
    #define FFTW_EXECUTE_DFT_R2C     fftw_execute_dft_r2c
    #define FFTW_EXECUTE_DFT_C2R     fftw_execute_dft_c2r

    #define FFTW_INIT_THREADS        fftw_init_threads
    #define FFTW_PLAN_WITH_NTHREADS  fftw_plan_with_nthreads
    #define FFTW_CLEANUP_THREADS     fftw_cleanup_threads
    #define FFTW_DESTROY_PLAN        fftw_destroy_plan

    #define H5T_REAL                 H5T_NATIVE_DOUBLE

#else
    using Real = float;
    using Complex = fftwf_complex; // float[2]
    
    #define FFTW_PLAN                fftwf_plan
    #define FFTW_MALLOC              fftwf_malloc
    #define FFTW_ALLOC_R             fftwf_alloc_real
    #define FFTW_ALLOC_C             fftwf_alloc_complex
    #define FFTW_FREE                fftwf_free

    #define FFTW_PLAN_DFT_2D         fftwf_plan_dft_2d
    #define FFTW_PLAN_DFT_R2C_2D     fftwf_plan_dft_r2c_2d
    #define FFTW_PLAN_DFT_C2R_2D     fftwf_plan_dft_c2r_2d

    #define FFTW_PLAN_DFT_3D         fftwf_plan_dft_3d
    #define FFTW_PLAN_DFT_R2C_3D     fftwf_plan_dft_r2c_3d
    #define FFTW_PLAN_DFT_C2R_3D     fftwf_plan_dft_c2r_3d

    #define FFTW_EXECUTE_DFT         fftwf_execute_dft
    #define FFTW_EXECUTE_DFT_R2C     fftwf_execute_dft_r2c
    #define FFTW_EXECUTE_DFT_C2R     fftwf_execute_dft_c2r

    #define FFTW_INIT_THREADS        fftwf_init_threads
    #define FFTW_PLAN_WITH_NTHREADS  fftwf_plan_with_nthreads
    #define FFTW_CLEANUP_THREADS     fftwf_cleanup_threads
    #define FFTW_DESTROY_PLAN        fftwf_destroy_plan

    #define H5T_REAL                 H5T_NATIVE_FLOAT

#endif


#ifdef USE_GPU

#include <cufft.h>

#ifdef USE_DOUBLE
    using CuComplex = cufftDoubleComplex;    // double2
    using CuReal    = cufftDoubleReal;       // double
                                             
    #define CUFFT_C2C_TYPE  CUFFT_Z2Z
    #define CUFFT_R2C_TYPE  CUFFT_D2Z
    #define CUFFT_C2R_TYPE  CUFFT_Z2D

    #define CUFFT_EXEC_C2C  cufftExecZ2Z
    #define CUFFT_EXEC_R2C  cufftExecD2Z
    #define CUFFT_EXEC_C2R  cufftExecZ2D

    #define FABS  fabs
    #define FMAX  fmax
#else
    using CuComplex = cufftComplex;          // float2
    using CuReal    = cufftReal;             // float

    #define CUFFT_C2C_TYPE  CUFFT_C2C
    #define CUFFT_R2C_TYPE  CUFFT_R2C
    #define CUFFT_C2R_TYPE  CUFFT_C2R

    #define CUFFT_EXEC_C2C  cufftExecC2C
    #define CUFFT_EXEC_R2C  cufftExecR2C
    #define CUFFT_EXEC_C2R  cufftExecC2R
    
    #define FABS  fabsf
    #define FMAX  fmaxf
#endif

#endif
