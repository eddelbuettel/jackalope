
/*
 ********************************************************

 Functions to create new sequences.

 ********************************************************
 */



#include <RcppArmadillo.h>
#include <vector>
#include <string>
#include <random> // C++11 sampling distributions
#include <pcg/pcg_random.hpp> // pcg prng
#ifdef _OPENMP
#include <omp.h>  // omp
#endif

#include "gemino_types.h"  // integer types
#include "sequence_classes.h" // RefGenome, RefSequence classes
#include "table_sampler.h" // table sampling
#include "pcg.h" // pcg::max, mc_seeds, seeded_pcg

using namespace Rcpp;









/*
 ========================================================================================
 ========================================================================================

 Random sequences

 ========================================================================================
 ========================================================================================
 */



/*
 Template that does most of the work for creating new sequences for the following two
 functions when `len_sd > 0`.
 Classes `OuterClass` and `InnerClass` can be `std::vector<std::string>` and
 `std::string` or `RefGenome` and `RefSequence`.
 No other combinations are guaranteed to work.
 */

template <typename OuterClass, typename InnerClass>
OuterClass create_sequences_(const uint32& n_seqs,
                             const double& len_mean,
                             const double& len_sd,
                             NumericVector equil_freqs,
                             const uint32& n_cores) {

    if (equil_freqs.size() == 0) equil_freqs = NumericVector(4, 0.25);

    // Converting to STL format
    std::vector<double> pi_tcag = as<std::vector<double>>(equil_freqs);

    // Generate seeds for random number generators (1 RNG per core)
    const std::vector<std::vector<uint64>> seeds = mc_seeds(n_cores);

    // Table-sampling object
    const TableSampler sampler(pi_tcag);

    // Creating output object
    OuterClass seqs_out(n_seqs);

    // parameters for creating the gamma distribution
    const double gamma_shape = (len_mean * len_mean) / (len_sd * len_sd);
    const double gamma_scale = (len_sd * len_sd) / len_mean;


    #ifdef _OPENMP
    #pragma omp parallel default(shared) num_threads(n_cores) if (n_cores > 1)
    {
    #endif

    std::vector<uint64> active_seeds;

    // Write the active seed per core or just write one of the seeds.
    #ifdef _OPENMP
    uint32 active_thread = omp_get_thread_num();
    active_seeds = seeds[active_thread];
    #else
    active_seeds = seeds[0];
    #endif

    pcg32 engine = seeded_pcg(active_seeds);
    // Gamma distribution to be used for size selection (doi: 10.1093/molbev/msr011):
    std::gamma_distribution<double> distr;
    if (len_sd > 0) {
        distr = std::gamma_distribution<double>(gamma_shape, gamma_scale);
    }

    // Parallelize the Loop
    #ifdef _OPENMP
    #pragma omp for schedule(static)
    #endif
    for (uint32 i = 0; i < n_seqs; i++) {
        InnerClass& seq(seqs_out[i]);

        // Get length of output sequence:
        uint32 len;
        if (len_sd > 0) {
            len = static_cast<uint32>(distr(engine));
            if (len < 1) len = 1;
        } else len = len_mean;
        // Sample sequence:
        seq.resize(len, 'x');
        for (uint32 j = 0; j < len; j++) {
            uint32 k = sampler.sample(engine);
            seq[j] = table_sampler::bases[k];
        }
    }

    #ifdef _OPENMP
    }
    #endif

    return seqs_out;
}





//' Create `RefGenome` pointer based on nucleotide equilibrium frequencies.
//'
//' Function to create random sequences for a new reference genome object.
//'
//' Note that this function will never return empty sequences.
//'
//' @param n_seqs Number of sequences.
//' @param len_mean Mean for the gamma distribution for sequence sizes.
//' @param len_sd Standard deviation for the gamma distribution for sequence sizes.
//'     If set to `<= 0`, all sequences will be the same length. Defaults to `0`.
//' @param equil_freqs Vector of nucleotide equilibrium frequencies for
//'     "T", "C", "A", and "G", respectively. Defaults to `rep(0.25, 4)`.
//' @param n_cores Number of cores to use via OpenMP.
//'
//'
//' @return External pointer to a `RefGenome` C++ object.
//'
//' @export
//'
//' @examples
//'
//' genome <- create_genome(10, 100e6, 10e6, equil_freqs = c(0.1, 0.2, 0.3, 0.4))
//'
//[[Rcpp::export]]
SEXP create_genome(const uint32& n_seqs,
                   const double& len_mean,
                   const double& len_sd = 0,
                   NumericVector equil_freqs = NumericVector(0),
                   const uint32& n_cores = 1) {

    XPtr<RefGenome> ref_xptr(new RefGenome(), true);
    RefGenome& ref(*ref_xptr);

    ref = create_sequences_<RefGenome, RefSequence>(
        n_seqs, len_mean, len_sd, equil_freqs, n_cores);

    for (uint32 i = 0; i < n_seqs; i++) {
        ref.total_size += ref[i].size();
        ref[i].name = "seq" + std::to_string(i);
    }


    return ref_xptr;
}




//' @describeIn create_genome create random sequences as a character vector.
//'
//'
//' @inheritParams create_genome
//'
//' @return Character vector of sequence strings.
//'
//'
//' @export
//'
//' @examples
//' randos <- rando_seqs(10, 1000, 10)
//'
//[[Rcpp::export]]
std::vector<std::string> rando_seqs(const uint32& n_seqs,
                                    const double& len_mean,
                                    const double& len_sd = 0,
                                    NumericVector equil_freqs = NumericVector(0),
                                    const uint32& n_cores = 1) {

    std::vector<std::string> ref = create_sequences_<std::vector<std::string>,
                std::string>(n_seqs, len_mean, len_sd, equil_freqs, n_cores);

    return ref;
}

