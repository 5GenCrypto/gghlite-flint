#ifndef _MIFE_H_
#define _MIFE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <gghlite/gghlite.h>
#include <gghlite/gghlite-defs.h>

int NUM_ENCODINGS_GENERATED;
int PRINT_ENCODING_PROGRESS;
int NUM_ENCODINGS_TOTAL;

typedef enum {
  //!< default behaviour
  MIFE_DEFAULT = 0x00,
  
  //!< do not multiply kilian randomizers into the encodings
  MIFE_NO_KILIAN    = 0x01, 
  
  //!< do not multiply the scalar randomizers into the encodings
  MIFE_NO_RANDOMIZERS    = 0x02,

  //!< pick a simple partitioning (x[0] is encoded at the universe, all others 
  //are encoded at the empty set.)
  MIFE_SIMPLE_PARTITIONS  = 0x04,
} mife_flag_t;

struct _gghlite_enc_mat_struct {
  int nrows; // number of rows in the matrix
  int ncols; // number of columns in the matrix
  gghlite_enc_t **m;
};

typedef struct _gghlite_enc_mat_struct gghlite_enc_mat_t[1];

struct _mife_mat_clr_struct {
  fmpz_mat_t **clr;
};

typedef struct _mife_mat_clr_struct mife_mat_clr_t[1];

struct _mife_ciphertext_struct {
  gghlite_enc_mat_t **enc;
};

typedef struct _mife_ciphertext_struct mife_ciphertext_t[1];

struct _mife_sk_struct {
  int numR;
  gghlite_sk_t self;
  fmpz_mat_t *R;
  fmpz_mat_t *R_inv;
};

typedef struct _mife_sk_struct mife_sk_t[1];

typedef struct {
  unsigned int num_rows, num_cols;
  bool **elems;
} f2_matrix;

/* returns true iff dest is successfully initialized to the contents of src */
bool f2_matrix_copy(f2_matrix *const dest, const f2_matrix src);
bool f2_matrix_zero(f2_matrix *const dest, const unsigned int num_rows, const unsigned int num_cols);
void f2_matrix_free(f2_matrix m);

struct _mife_pp_struct {
  int num_inputs; // the arity of the MBP (for comparisons, this is 2).
  int *n; // of length num_inputs
  int *gammas; // gamma for each input
  int L; // log # of plaintexts we can support
  int gamma; // should be sum of gammas[i] 
  int kappa; // the kappa for gghlite (degree of multilinearity)
  int numR; // number of kilian matrices. should be kappa-1
  mife_flag_t flags;
  fmpz_t p; // the prime, the order of the field
  gghlite_params_t *params_ref; // gghlite's public parameters, by reference

  // MBP function pointers
  void *mbp_params; // additional parameters one can pass into the MBP setup
  int (*paramfn)  (struct _mife_pp_struct *, int); // for determining number of matrices per input
  void (*kilianfn)(struct _mife_pp_struct *, int *); // set kilian dimensions
  void (*orderfn) (struct _mife_pp_struct *, int, int *, int *); // function pointer for MBP ordering
  void (*setfn)   (struct _mife_pp_struct *, mife_mat_clr_t, void *);
  int (*parsefn)  (struct _mife_pp_struct *, char **); // function pointer for parsing output 
};

typedef struct _mife_pp_struct mife_pp_t[1];


/* MIFE interface */
void mife_init_params(mife_pp_t pp, mife_flag_t flags);
void mife_mbp_set(
    void *mbp_params,
    mife_pp_t pp,
    int num_inputs,
    int (*paramfn)  (mife_pp_t, int),
    void (*kilianfn)(mife_pp_t, int *),
    void (*orderfn) (mife_pp_t, int, int *, int *),
    void (*setfn)   (mife_pp_t, mife_mat_clr_t, void *),
    int (*parsefn)  (mife_pp_t, char **)
    );
void mife_setup(mife_pp_t pp, mife_sk_t sk, int L, int lambda,
    gghlite_flag_t ggh_flags, aes_randstate_t randstate);
void mife_encrypt(mife_ciphertext_t ct, void *message, mife_pp_t pp,
    mife_sk_t sk, aes_randstate_t randstate);
int mife_evaluate(mife_pp_t pp, mife_ciphertext_t *cts);

/* memory-efficient MIFE interface */
void mife_encrypt_setup(mife_pp_t pp, fmpz_t uid, void *message,
    mife_mat_clr_t out_clr, int ****out_partitions);
void mife_encrypt_single(mife_pp_t pp, mife_sk_t sk, aes_randstate_t randstate,
    int global_index, mife_mat_clr_t clr, int ***partitions,
    gghlite_enc_mat_t out_ct);
void mife_encrypt_cleanup(mife_pp_t pp, mife_mat_clr_t clr, int ***out_partitions);

/* MIFE internal functions */
void reset_T();
float get_T();
void set_NUM_ENC(int val);
int get_NUM_ENC();
void mife_apply_randomizer(mife_pp_t pp, aes_randstate_t randstate, fmpz_mat_t m);
void mife_apply_randomizers(mife_mat_clr_t met, mife_pp_t pp, mife_sk_t sk,
    aes_randstate_t randstate);
int ***mife_partitions(mife_pp_t pp, fmpz_t index);
void mife_partitions_clear(mife_pp_t pp, int ***partitions);
void mife_apply_kilian(mife_pp_t pp, mife_sk_t sk, fmpz_mat_t m, int global_index);
void mife_set_encodings(mife_ciphertext_t ct, mife_mat_clr_t met, fmpz_t index,
    mife_pp_t pp, mife_sk_t sk, aes_randstate_t randstate);
void mife_clear_pp_read(mife_pp_t pp);
void mife_clear_pp(mife_pp_t pp);
void mife_clear_sk(mife_sk_t sk);
void mife_mat_clr_clear(mife_pp_t pp, mife_mat_clr_t met);
void mife_gen_partitioning(int *partitioning, fmpz_t index, int L, int nu);
void mife_mat_encode(mife_pp_t pp, mife_sk_t sk, gghlite_enc_mat_t enc,
    fmpz_mat_t m, int *group, aes_randstate_t randstate);
void gghlite_enc_mat_zeros_print(mife_pp_t pp, gghlite_enc_mat_t m);
void gghlite_enc_mat_mul(gghlite_params_t params, gghlite_enc_mat_t r,
    gghlite_enc_mat_t m1, gghlite_enc_mat_t m2);
void mife_ciphertext_clear(mife_pp_t pp, mife_ciphertext_t ct);
void message_to_dary(ulong *dary, int bitstring_len, fmpz_t message, int d);

/* functions dealing with file reading and writing for encodings */
#define gghlite_enc_fprint fmpz_mod_poly_fprint 
void fread_gghlite_params(FILE *fp, gghlite_params_t params);
int gghlite_enc_fread(FILE * f, gghlite_enc_t poly);
void gghlite_params_clear_read(gghlite_params_t self);
void fwrite_mife_pp(mife_pp_t pp, char *filepath);
void fread_mife_pp(mife_pp_t pp, char *filepath);
void fwrite_mife_sk(mife_sk_t sk, char *filepath);
void fread_mife_sk(mife_sk_t sk, char *filepath);
void fwrite_mife_ciphertext(mife_pp_t pp, mife_ciphertext_t ct, char *filepath);
void fwrite_gghlite_enc_mat(mife_pp_t pp, gghlite_enc_mat_t m, FILE *fp);
void fread_mife_ciphertext(mife_pp_t pp, mife_ciphertext_t ct, char *filepath);
void fread_gghlite_enc_mat(mife_pp_t pp, gghlite_enc_mat_t m, FILE *fp);

/* functions dealing with fmpz types and matrix multiplications mod fmpz_t */
void fmpz_modp_matrix_inverse(fmpz_mat_t inv, fmpz_mat_t a, int dim, fmpz_t p);
void fmpz_mat_modp(fmpz_mat_t m, int dim, fmpz_t p);
void fmpz_mat_scalar_mul_modp(fmpz_mat_t m, fmpz_t scalar, fmpz_t modp);
void fmpz_mat_mul_modp(fmpz_mat_t a, fmpz_mat_t b, fmpz_mat_t c, int n,
    fmpz_t p);
void gghlite_enc_mat_init(gghlite_params_t params, gghlite_enc_mat_t m,
    int nrows, int ncols);
void gghlite_enc_mat_clear(gghlite_enc_mat_t m);
void fmpz_init_exp(fmpz_t exp, int base, int n);

/* functions for computing matrix inverse mod fmpz_t */
void fmpz_mat_modp(fmpz_mat_t m, int dim, fmpz_t p);
void fmpz_mat_det_modp(fmpz_t det, fmpz_mat_t a, int n, fmpz_t p);
void fmpz_mat_cofactor_modp(fmpz_mat_t b, fmpz_mat_t a, int n, fmpz_t p);
void fmpz_modp_matrix_inverse(fmpz_mat_t inv, fmpz_mat_t a, int dim, fmpz_t p);

/* test functions */
int test_matrix_inv(int n, aes_randstate_t randstate, fmpz_t modp);
int int_arrays_equal(ulong *arr1, ulong *arr2, int length);
void test_dary_conversion();

#endif /* _MIFE_H_ */
