#ifndef __JACKALOPE_VAR_CLASSES_H
#define __JACKALOPE_VAR_CLASSES_H


/*
 ********************************************************

 Classes to store variant chromosome info.

 ********************************************************
 */


#include "jackalope_config.h" // controls debugging and diagnostics output

#include <RcppArmadillo.h>
#include <vector>  // vector class
#include <string>  // string class
#include <cstring> // for std::strcpy
#include <deque>  // deque class

#include "jackalope_types.h"  // integer types
#include "ref_classes.h"  // Ref* classes
#include "util.h"  // clear_memory

using namespace Rcpp;




/*
 ========================================================================================
 ========================================================================================

 One mutation (substitution, insertion, or deletion)

 ========================================================================================
 ========================================================================================
 */

// struct Mutation {
//
//     // How this mutation changes the overall chromosome size:
//     sint64 size_modifier;
//     // Position on the old (i.e., reference) chromosome:
//     uint64 old_pos;
//     // Position on the new, variant chromosome:
//     uint64 new_pos;
//     // Nucleotides associated with this mutation:
//     std::string nucleos;
//
//     // Constructors
//     Mutation(uint64 old_pos_, uint64 new_pos_, std::string nucleos_)
//         : size_modifier(nucleos_.size() - 1), old_pos(old_pos_),
//           new_pos(new_pos_), nucleos(nucleos_) {};
//     Mutation(uint64 old_pos_, uint64 new_pos_, char nucleo_)
//         : size_modifier(0), old_pos(old_pos_),
//           new_pos(new_pos_), nucleos(1, nucleo_) {};
//     Mutation(const Mutation& other)
//         : size_modifier(other.size_modifier), old_pos(other.old_pos),
//           new_pos(other.new_pos), nucleos(other.nucleos) {};
//     // For deletions:
//     Mutation(uint64 old_pos_, uint64 new_pos_, sint64 size_modifier_)
//         : size_modifier(size_modifier_), old_pos(old_pos_),
//           new_pos(new_pos_), nucleos("") {};
//
//     /*
//      These operators compare Mutation objects.
//      If there is any overlap between the two mutations, then neither > nor <
//      will return true.
//      They are used to determine how and whether to merge VarChrom objects.
//     */
//     bool operator<(const Mutation& other) const {
//         // Check for a deletion spanning the distance between the two:
//         if (size_modifier < 0) {
//             uint64 end_pos = old_pos + static_cast<uint64>(std::abs(size_modifier)) - 1;
//             return end_pos < other.old_pos;
//         }
//         return old_pos < other.old_pos;
//     }
//     bool operator>(const Mutation& other) const {
//         // Check for a deletion spanning the distance between the two:
//         if (other.size_modifier < 0) {
//             uint64 other_end_pos = other.old_pos +
//                 static_cast<uint64>(std::abs(other.size_modifier)) - 1;
//             return old_pos > other_end_pos;
//         }
//         return old_pos > other.old_pos;
//     }
//
//
//     // For easily outputting mutation chromosome
//     const char& operator[](const uint64& idx) const {
//         return nucleos[idx];
//     }
// };



struct AllMutations {

    std::deque<sint64> size_modifier;
    std::deque<uint64> old_pos;
    std::deque<uint64> new_pos;
    std::deque<char*> nucleos;

    AllMutations() : size_modifier(), old_pos(), new_pos(), nucleos() {}

    AllMutations(const AllMutations& other)
        : size_modifier(other.size_modifier),
          old_pos(other.old_pos),
          new_pos(other.new_pos),
          nucleos(other.nucleos.size(), nullptr) {
        for (uint64 i = 0; i < nucleos.size(); i++) {
            fill_one_nucleos__(other.nucleos[i], &nucleos[i]);
        }
    }
    AllMutations& operator=(const AllMutations& other) {

        size_modifier = other.size_modifier;
        old_pos = other.old_pos;
        new_pos = other.new_pos;
        for (uint64 i = 0; i < nucleos.size(); i++) delete [] nucleos[i];
        nucleos = std::deque<char*>(other.nucleos.size(), nullptr);
        for (uint64 i = 0; i < nucleos.size(); i++) {
            fill_one_nucleos__(other.nucleos[i], &nucleos[i]);
        }
        return *this;
    }

    ~AllMutations() {
        for (uint64 i = 0; i < nucleos.size(); i++) {
            delete [] nucleos[i];
        }
    }

    inline size_t size() const noexcept {
        return old_pos.size();
    }

    inline bool empty() const noexcept {
        return old_pos.empty();
    }

    inline void clear() {

        if (old_pos.size() == 0) return;

        size_modifier.clear();
        old_pos.clear();
        new_pos.clear();
        nucleos.clear();
        // // Clear memory:
        // clear_memory<std::deque<sint64>>(size_modifier);
        // clear_memory<std::deque<uint64>>(old_pos);
        // clear_memory<std::deque<uint64>>(new_pos);
        // clear_memory<std::deque<char*>>(nucleos);

        return;
    }
    // Add to front
    inline void push_front(const sint64& sm,
                           const uint64& op,
                           const uint64& np,
                           const char* nts) {
        size_modifier.push_front(sm);
        old_pos.push_front(op);
        new_pos.push_front(np);
        nucleos.push_front(nullptr);
        fill_one_nucleos__(nts, &nucleos.front());
        return;
    }
    inline void push_front(const sint64& sm,
                           const uint64& op,
                           const uint64& np,
                           const char& nt) {
        size_modifier.push_front(sm);
        old_pos.push_front(op);
        new_pos.push_front(np);
        nucleos.push_front(nullptr);
        fill_one_nucleos__(nt, &nucleos.front());
        return;
    }
    // Add to back
    inline void push_back(const sint64& sm,
                          const uint64& op,
                          const uint64& np,
                          const char* nts) {
        size_modifier.push_back(sm);
        old_pos.push_back(op);
        new_pos.push_back(np);
        nucleos.push_back(nullptr);
        fill_one_nucleos__(nts, &nucleos.back());
        return;
    }
    inline void push_back(const sint64& sm,
                          const uint64& op,
                          const uint64& np,
                          const char& nt) {
        size_modifier.push_back(sm);
        old_pos.push_back(op);
        new_pos.push_back(np);
        nucleos.push_back(nullptr);
        fill_one_nucleos__(nt, &nucleos.back());
        return;
    }
    // Add to middle
    inline void insert(const uint64& ind,
                       const sint64& sm,
                       const uint64& op,
                       const uint64& np,
                       const char* nts) {

        size_modifier.insert(size_modifier.begin() + ind, sm);
        old_pos.insert(old_pos.begin() + ind, op);
        new_pos.insert(new_pos.begin() + ind, np);
        nucleos.insert(nucleos.begin() + ind, nullptr);
        fill_one_nucleos__(nts, &nucleos[ind]);
        return;
    }
    inline void insert(const uint64& ind,
                       const sint64& sm,
                       const uint64& op,
                       const uint64& np,
                       const char& nt) {

        size_modifier.insert(size_modifier.begin() + ind, sm);
        old_pos.insert(old_pos.begin() + ind, op);
        new_pos.insert(new_pos.begin() + ind, np);
        nucleos.insert(nucleos.begin() + ind, nullptr);
        fill_one_nucleos__(nt, &nucleos[ind]);
        return;
    }
    // Remove from position
    inline void erase(const uint64& ind) {

        size_modifier.erase(size_modifier.begin() + ind);
        old_pos.erase(old_pos.begin() + ind);
        new_pos.erase(new_pos.begin() + ind);
        delete [] nucleos[ind];
        nucleos.erase(nucleos.begin() + ind);
        return;
    }

    // Remove between positions
    inline void erase(const uint64& ind1, const uint64& ind2) {

        size_modifier.erase(size_modifier.begin() + ind1, size_modifier.begin() + ind2);
        old_pos.erase(old_pos.begin() + ind1, old_pos.begin() + ind2);
        new_pos.erase(new_pos.begin() + ind1, new_pos.begin() + ind2);
        for (uint64 ind = ind1; ind < ind2; ind++) delete [] nucleos[ind];
        nucleos.erase(nucleos.begin() + ind1, nucleos.begin() + ind2);
        return;
    }


private:


    // Fill one `char *` in `nucleos` with values from a `const char *` object
    inline void fill_one_nucleos__(const char* nts, char** one_nucleos) {
        if (nts != nullptr) {
            size_t nts_size = std::strlen(nts);
            *one_nucleos = new char[nts_size + 1];
            for (size_t i = 0; i < nts_size; i++) {
                (*one_nucleos)[i] = nts[i];
            }
            (*one_nucleos)[nts_size] = '\0';
        }
        return;
    }
    // Same but for a reference to one character
    inline void fill_one_nucleos__(const char& nt, char** one_nucleos) {
        *one_nucleos = new char[2];
        (*one_nucleos)[0] = nt;
        (*one_nucleos)[1] = '\0';
        return;
    }

};








/*
 ========================================================================================
 ========================================================================================

 Variant genomes

 ========================================================================================
 ========================================================================================
 */

// (These classes will later need access to private members of VarChrom.)
class SubMutator;


/*
 =========================================
 One chromosome from one variant haploid genome
 =========================================
 */

class VarChrom {

    friend SubMutator;

public:

    const RefChrom* ref_chrom;  // pointer to const RefChrom
    AllMutations mutations;
    uint64 chrom_size;
    std::string name;

    // Constructors
    VarChrom() : ref_chrom(nullptr) {};
    VarChrom(const RefChrom& ref)
        : ref_chrom(&ref),
          mutations(),
          chrom_size(ref.size()),
          name(ref.name) {};

    /*
     Since all other classes have a size() method, I'm including this here:
     */
    uint64 size() const noexcept {
        return chrom_size;
    }

    // Add existing mutation information in another `VarChrom` to this one,
    // adding to the back of `mutations`, with a starting mutation index
    sint64 add_to_back(const VarChrom& other, const uint64& mut_i);


    /*
     ------------------
     Re-calculate new positions (and total chromosome size)
     ------------------
     */
    void calc_positions();
    void calc_positions(uint64 mut_i);
    void calc_positions(uint64 mut_i, const sint64& modifier);


    /*
     ------------------
     Retrieve all nucleotides (i.e., the full chromosome; std::string type) from
     the variant chromosome
     ------------------
     */
    std::string get_chrom_full() const;



    /*
     ------------------
     Set a string object to a chunk of a chromosome from the variant chromosome.
     ------------------
     */
    void set_chrom_chunk(std::string& chunk_str,
                       const uint64& start,
                       const uint64& chunk_size,
                       uint64& mut_i) const;

    /*
     ------------------
     Adding mutations somewhere in the deque
     ------------------
     */
    void add_deletion(const uint64& size_, const uint64& new_pos_);
    void add_insertion(const std::string& nucleos_, const uint64& new_pos_);
    void add_substitution(const char& nucleo, const uint64& new_pos_);


    /*
     ------------------
     For filling a read at a given starting position from a chromosome of a
     given starting position and size.
     Used only for sequencer.
     ------------------
     */
    void fill_read(std::string& read,
                   const uint64& read_start,
                   const uint64& chrom_start,
                   uint64 n_to_add) const;



private:

    /*
     -------------------
     Internal function to "blowup" mutation(s) due to a deletion.
     By "blowup", I mean it removes substitutions and insertions if they're covered
     entirely by the deletion, and it merges any deletions that are contiguous.
     -------------------
     */
    void deletion_blowup_(uint64& mut_i, uint64& deletion_start, uint64& deletion_end,
                          sint64& size_mod);




    /*
     -------------------
     Inner function to merge an insertion and deletion.
     -------------------
     */
    void merge_del_ins_(uint64& insert_i,
                        uint64& deletion_start,
                        uint64& deletion_end,
                        sint64& size_mod);




    /*
     -------------------
     Inner function to remove Mutation and keep iterator from being invalidated.
     -------------------
     */
    void remove_mutation_(uint64& mut_i);
    void remove_mutation_(uint64& mut_i1, uint64& mut_i2);


    /*
     ------------------
     Internal function for finding character of either mutation or reference
     given an index (in the "new", variant chromosome) and an iterator pointing to
     a single Mutation object.
     ------------------
     */
    char get_char_(const uint64& new_pos, const uint64& mut) const;

    /*
     ------------------
     Inner function to return an iterator to the Mutation object nearest to
     (without being past) an input position on the "new", variant chromosome.
     ------------------
     */
    uint64 get_mut_(const uint64& new_pos) const;

};



/*
 =========================================
 One variant haploid genome
 =========================================
 */

class VarGenome {
public:

    // Fields
    std::string name;
    std::deque<VarChrom> var_genome;

    // Constructors
    VarGenome() {};
    VarGenome(const RefGenome& ref) {
        name = "";
        for (uint64 i = 0; i < ref.size(); i++) {
            VarChrom var_chrom(ref[i]);
            var_genome.push_back(var_chrom);
        }
    };
    VarGenome(const std::string& name_, const RefGenome& ref) {
        name = name_;
        for (uint64 i = 0; i < ref.size(); i++) {
            VarChrom var_chrom(ref[i]);
            var_genome.push_back(var_chrom);
        }
    };

    // For easily outputting a reference to a VarChrom
    VarChrom& operator[](const uint64& idx) {
        VarChrom& var_chrom(var_genome[idx]);
        return var_chrom;
    }
    // const version
    const VarChrom& operator[](const uint64& idx) const {
        const VarChrom& var_chrom(var_genome[idx]);
        return var_chrom;
    }
    // To return the number of chromosomes
    uint64 size() const noexcept {
        return var_genome.size();
    }
    // To return all the chromosome sizes
    std::vector<uint64> chrom_sizes() const noexcept {
        std::vector<uint64> out(size());
        for (uint64 i = 0; i < out.size(); i++) out[i] = var_genome[i].size();
        return out;
    }

private:

};





/*
 =========================================
 Multiple variant haploid genomes (based on the same reference)
 =========================================
 */

class VarSet {
public:
    std::deque<VarGenome> variants;
    const RefGenome* reference;  // pointer to const RefGenome

    /*
     Constructors:
     */
    VarSet(const RefGenome& ref) : variants(), reference(&ref) {};
    VarSet(const RefGenome& ref, const uint64& n_vars)
        : variants(n_vars, VarGenome(ref)),
          reference(&ref) {
        for (uint64 i = 0; i < n_vars; i++) variants[i].name = "var" + std::to_string(i);
    };
    // If you already have the names:
    VarSet(const RefGenome& ref, const std::vector<std::string>& names_)
        : variants(names_.size(), VarGenome(ref)),
          reference(&ref) {
        for (uint64 i = 0; i < names_.size(); i++) variants[i].name = names_[i];
    };

    // For easily outputting a reference to a VarGenome
    VarGenome& operator[](const uint64& idx) {
#ifdef __JACKALOPE_DEBUG
        if (idx >= variants.size()) {
            stop("trying to access a VarGenome that doesn't exist");
        }
#endif
        VarGenome& vg(variants[idx]);
        return vg;
    }
    // const version of above
    const VarGenome& operator[](const uint64& idx) const {
#ifdef __JACKALOPE_DEBUG
        if (idx >= variants.size()) {
            stop("trying to access a VarGenome that doesn't exist");
        }
#endif
        const VarGenome& vg(variants[idx]);
        return vg;
    }
    // To return the number of variants
    uint64 size() const noexcept {
        return variants.size();
    }
    // To return the minimum size of a given chromosome
    uint64 min_size(const uint64& i) const {
        uint64 ms = variants[0][i].size();
        for (const VarGenome vg : variants) {
            if (vg[i].size() < ms) ms = vg[i].size();
        }
        return ms;
    }

    /*
     Fill VarGenome objects after the reference has been filled
     */
    void fill_vars(const uint64& n_vars) {
        VarGenome vg(*reference);
        for (uint64 i = 0; i < n_vars; i++) variants.push_back(vg);
        return;
    }
    // Overloaded for if you want to provide names
    void fill_vars(const std::vector<std::string>& names) {
        for (uint64 i = 0; i < names.size(); i++) {
            VarGenome vg(names[i], *reference);
            variants.push_back(vg);
        }
        return;
    }

    // For printing output
    void print() const noexcept;

};

#endif
